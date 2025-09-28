#!/usr/bin/env fish
# Temporary macOS tweaks for afl++ runs: unload CrashReporter, raise SysV SHM limits, run fuzz commands, and restore state.

# Abort early if not running on macOS (Darwin).
if test (uname -s) != "Darwin"
    echo "error: scripts/run_afl_session.fish is only supported on macOS (Darwin) for now." >&2
    exit 65
end

# Paths for CrashReporter launch agents/daemons.
set -g __afl_sl /System/Library
set -g __afl_pl com.apple.ReportCrash
set -g __afl_agent "$__afl_sl/LaunchAgents/$__afl_pl.plist"
set -g __afl_daemon "$__afl_sl/LaunchDaemons/$__afl_pl.Root.plist"

# Flags tracking which resources need to be restored on exit.
set -g __afl_reload_agent 0
set -g __afl_reload_daemon 0
set -g __afl_have_sudo 0

# SysV shared memory parameters required by afl++ (current + target values).
set -g __afl_sysctl_names kern.sysv.shmmax kern.sysv.shmmin kern.sysv.shmseg kern.sysv.shmall
set -g __afl_sysctl_target 524288000 1 48 131072000
set -g __afl_sysctl_prev
set -g __afl_sysctl_changed 0

# Guard to avoid running cleanup twice on INT+EXIT
set -g __afl_cleaned 0

# Helper: increment integers without invoking external expr.
function __afl_math_inc --argument-names value
    math "$value + 1"
end

# Helper: reload CrashReporter daemon, escalating if the cached sudo credential expired.
function __afl_reload_root --argument-names plist
    sudo -n launchctl load -w "$plist" >/dev/null 2>&1
    if test $status -eq 0
        return 0
    end

    sudo -n launchctl bootstrap system "$plist" >/dev/null 2>&1
    if test $status -eq 0
        return 0
    end

    echo "[afl session] sudo password needed to refresh CrashReporter" >&2
    sudo -v >/dev/null 2>&1
    if test $status -ne 0
        return 1
    end

    sudo launchctl load -w "$plist" >/dev/null 2>&1
    if test $status -eq 0
        return 0
    end

    sudo launchctl bootstrap system "$plist" >/dev/null 2>&1
    if test $status -eq 0
        return 0
    end

    return 1
end

# Cleanup handler: restore crash reporters, sysctl limits, and drop sudo ticket.
function __afl_restore
    if test $__afl_cleaned -eq 1
        return
    end
    set -g __afl_cleaned 1
    if test $__afl_reload_agent -eq 1
        launchctl load -w "$__afl_agent" >/dev/null 2>&1
    end

    if test $__afl_reload_daemon -eq 1
        if test $__afl_have_sudo -eq 1
            __afl_reload_root "$__afl_daemon"
            if test $status -ne 0
                echo "warning: failed to reload root crash reporter; run: sudo launchctl bootstrap system $__afl_daemon" >&2
            end
        else
            echo "warning: re-enable root crash reporter with:\n  sudo launchctl bootstrap system $__afl_daemon" >&2
        end
    end

    if test $__afl_have_sudo -eq 1
        if test $__afl_sysctl_changed -eq 1
            set idx 1
            for name in $__afl_sysctl_names
                set prev $__afl_sysctl_prev[$idx]
                if test -n "$prev"
                    sudo -n sysctl -w "$name=$prev" >/dev/null 2>&1
                    if test $status -ne 0
                        echo "[afl session] sudo password needed to restore sysctl $name" >&2
                        sudo -v >/dev/null 2>&1
                        if test $status -eq 0
                            sudo sysctl -w "$name=$prev" >/dev/null 2>&1
                        end
                    end
                end
                set idx (__afl_math_inc $idx)
            end
        end
        sudo -k >/dev/null 2>&1
    end
end

# Ensure cleanup runs on exit / ctrl+c / termination.
trap '__afl_restore' EXIT INT TERM

# Require a command to run.
if test (count $argv) -eq 0
    echo "Usage: "(status filename)" COMMAND [ARGS...]" >&2
    echo "Example: "(status filename)" env MallocNanoZone=0 AFL_SKIP_CPUFREQ=1 afl-fuzz ..." >&2
    exit 64
end

# Unload user-level CrashReporter if present.
if test -f "$__afl_agent"
    launchctl unload -w "$__afl_agent" >/dev/null 2>&1
    if test $status -eq 0
        set -g __afl_reload_agent 1
    end
else
    echo "warning: user crash reporter plist not found at $__afl_agent" >&2
end

# Elevate privileges to adjust system crash reporter and sysctl settings.
if test -f "$__afl_daemon"
    if type -q sudo
        echo "[afl session] sudo password required to adjust sysctl and disable CrashReporter" >&2
        sudo -v >/dev/null 2>&1
        if test $status -eq 0
            set -g __afl_have_sudo 1

            # Capture existing sysctl values so we can restore later.
            set -g __afl_sysctl_prev
            set idx 1
            for name in $__afl_sysctl_names
                set current (sysctl -n $name 2>/dev/null)
                set -g __afl_sysctl_prev $__afl_sysctl_prev $current
                set idx (__afl_math_inc $idx)
            end

            # Raise sysctl limits to afl++ recommended values.
            set idx 1
            for name in $__afl_sysctl_names
                set desired $__afl_sysctl_target[$idx]
                sudo sysctl -w "$name=$desired" >/dev/null 2>&1
                if test $status -ne 0
                    echo "warning: failed to set $name" >&2
                else
                    set -g __afl_sysctl_changed 1
                end
                set idx (__afl_math_inc $idx)
            end

            # Disable system CrashReporter while fuzzing.
            sudo launchctl unload -w "$__afl_daemon" >/dev/null 2>&1
            if test $status -eq 0
                set -g __afl_reload_daemon 1
            else
                echo "warning: failed to unload root crash reporter" >&2
            end
        else
            echo "warning: skipping root crash reporter; sudo not granted" >&2
        end
    else
        echo "warning: sudo not available; skipping root crash reporter" >&2
    end
else
    echo "warning: root crash reporter plist not found at $__afl_daemon" >&2
end

# Run the requested command (e.g., `make fuzz`).
$argv
set exit_code $status
exit $exit_code

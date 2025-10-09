#include <stdio.h>
#include "c_string.h"

int main(void) {
  const char* literal = "héł🧊";
  CStringResult s = string_from_char(literal, (int)strlen(literal));

  print_utf8_info(s.value);

  return 0;
}

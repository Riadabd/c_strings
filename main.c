/*
 * main.c
 * 
 * Copyright 2018 Riad Abdallah <riad.abdallah@hotmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>

#include "c_string.h"

int main(void)
{
	//printf("Start\n");
	//printf("------------------------------\n");
	c_string* s = malloc(sizeof(c_string));
	//create_string(s, "---&ABC---League");
	//create_string(s, "---&ABC-----0321849325---a-a-a-a-x--");
	create_string(s, "ABC");
	
	c_string** result = string_delim(s, "-");
	
	pretty_print(result);
	destroy_delim_string(result);

	/*c_string* data = read_from_file("file.txt");
	c_string** result = string_delim(data, " ");
	pretty_print(result);
	destroy_delim_string(result);*/

	c_string* stripped = trim_char(s, '-');
	print(stripped, "{} is stripped\n");

	destroy_string(stripped);
	//destroy_string(data);
	destroy_string(s);

	//printf("------------------------------\n");
	return 0;
}

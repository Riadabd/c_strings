#include <stdio.h>
#include "c_string.h"

int main(void) {
  c_string* s = malloc(sizeof(c_string));
  //create_string(s, "---&ABC---League-Hello");
  create_string(s, "---&ABC-----0321849325---a-a-a-a-x--");
  //create_string(s, "ABB--CAW");
  
  c_string** result = string_delim(s, "--");
  
  print_delim_strings(result);

  //pretty_print(result);
  //printf("%.*s\n", result[0]->length, result[0]->string);
  //printf("%.*s\n", result[1]->length, result[1]->string);
  //printf("%.*s\n", result[2]->length, result[2]->string);
  //printf("%.*s\n", result[3]->length, result[3]->string);
  //printf("%.*s\n", result[4]->length, result[4]->string);
  //printf("%.*s\n", result[5]->length, result[5]->string);
  //printf("%.*s\n", result[6]->length, result[6]->string);
  //printf("%.*s\n", result[7]->length, result[7]->string);

  destroy_delim_string(result);

  //c_string* stripped = trim_char(s, '-');

  //destroy_string(stripped);
  //destroy_string(data);
  destroy_string(s);

  return 0;
}

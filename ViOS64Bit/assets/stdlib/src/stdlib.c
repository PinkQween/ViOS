#include "stdlib.h"
#include "vios.h"

char *itoa(int i) {
  static char text[12];
  int loc = 11;
  text[11] = 0;
  char neg = 1;
  if (i >= 0) {
    neg = 0;
    i = -i;
  }

  while (i) {
    text[--loc] = '0' - (i % 10);
    i /= 10;
  }

  if (loc == 11)
    text[--loc] = '0';

  if (neg)
    text[--loc] = '-';

  return &text[loc];
}

int atoi(const char *str) {
  int result = 0;
  int sign = 1;
  
  // Skip whitespace
  while (*str == ' ' || *str == '\t' || *str == '\n') {
    str++;
  }
  
  // Handle sign
  if (*str == '-') {
    sign = -1;
    str++;
  } else if (*str == '+') {
    str++;
  }
  
  // Convert digits
  while (*str >= '0' && *str <= '9') {
    result = result * 10 + (*str - '0');
    str++;
  }
  
  return sign * result;
}

void *malloc(size_t size) { 
  return vios_malloc(size); 
}

void free(void *ptr) { 
  vios_free(ptr); 
}

void *realloc(void *ptr, size_t size) {
  return vios_realloc(ptr, size);
}
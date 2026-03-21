
#include <stddef.h>

struct CharArray {
    char *data;
    size_t length;
    size_t capacity;
};

void char_array_append(struct CharArray *arr, char value);
void char_array_pop(struct CharArray *arr);
void char_array_initialize(struct CharArray *arr);
void free_char_array(struct CharArray *arr);
void read_char_array(struct CharArray arr);

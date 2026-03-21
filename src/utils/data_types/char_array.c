#include <stdio.h>
#include <stdlib.h>
#include "char_array.h"

// Appends a character to the end of the CharArray, resizing if necessary
void char_array_append(struct CharArray *arr, char value) {
    if(arr->length == arr->capacity) {
        arr->capacity *= 2;
        arr->data = realloc(arr->data, arr->capacity * sizeof(char));
        if(arr->data == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    if(arr->capacity == 0) {
        arr->capacity = 4; // Initial capacity
        arr->data = malloc(arr->capacity * sizeof(char));
        if(arr->data == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    arr->data[arr->length] = value;
    arr->length++;
}

// Removes the last character from the CharArray if it is not empty
void char_array_pop(struct CharArray *arr) {
    if(arr->length > 0) {
        arr->length--;
    }
}

// Initializes the CharArray with an initial capacity and sets length to 0
void char_array_initialize(struct CharArray *arr) {
    arr->data = malloc(4 * sizeof(char)); // Initial capacity of 4
    if(arr->data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    arr->length = 0;
    arr->capacity = 4;
}

// Frees the memory allocated for the CharArray and resets its properties
void free_char_array(struct CharArray *arr) {
    free(arr->data);
    arr->data = NULL;
    arr->length = 0;
    arr->capacity = 0;
}

//Debugging function to print the contents of the CharArray
void read_char_array(struct CharArray arr) {
    printf("Current Array Length: %zu, Capacity: %zu\n", arr.length, arr.capacity);
    for(size_t i = 0; i < arr.length; i++) {
        printf("%c\n", arr.data[i]);
    }
}
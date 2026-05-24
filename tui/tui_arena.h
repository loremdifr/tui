//single-region malloc based arenas, always 64bit aligned
//https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/
#ifndef TUI_ARENA
#define TUI_ARENA

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

typedef struct {
    size_t used;
    size_t capacity;
    uint8_t *data;
} Arena;

Arena *arena_init(size_t capacity);
const void *arena_alloc(Arena *arena, size_t size);
const void *arena_realloc(Arena *arena, void *prev_ptr, size_t prev_size, size_t new_size);
void *arena_memcpy(void *dest, const void *source, size_t size);
void arena_reset(Arena *arena);
void arena_free(Arena *arena);

#ifdef TUI_ARENA_IMPL

Arena *arena_init(size_t capacity){
    // size_t size_bytes = sizeof(Arena) + sizeof(uintptr_t) * capacity;
    size_t size_bytes = sizeof(Arena) + capacity;
    Arena *arena = (Arena *)malloc(size_bytes);
    assert(arena);
    arena->capacity = capacity;
    arena->used = 0;
    arena->data = (uint8_t*)(arena + 1);  // Point to memory after struct

    // display_malloc_bytes(arena, size_bytes, "Arena");
    // arena_print(arena);

    return arena;
}

const void *arena_alloc(Arena *arena, size_t size){
    //TODO: if not initialized, init it?

    //always 64bit aligned
    uintptr_t next_alloc_offset = arena->used + ((64 - (arena->used % 64)) & 63);

    assert(next_alloc_offset + size <= arena->capacity);

    uint8_t *ptr = &arena->data[next_alloc_offset];
    arena->used  = next_alloc_offset + size;

    memset(ptr, 0, size); //zero new memory by default
    return ptr;
}

const void *arena_realloc(Arena *arena, void *prev_ptr, size_t prev_size, size_t new_size) {
    if (new_size <= prev_size) return prev_ptr;

    const void *new_ptr = arena_alloc(arena, new_size);
    char *new_ptr_char  = (char*)new_ptr;
    char *prev_ptr_char = (char*)prev_ptr;

    for (size_t i = 0; i < prev_size; i++){
        new_ptr_char[i] = prev_ptr_char[i];
    }

    return new_ptr;
}

void *arena_memcpy(void *dest, const void *source, size_t size)
{
    char *d = (char *)dest;
    const char *s = (char *)source;
    for (; size > 0; size--){
        *d++ = *s++;
    }
    return dest;
}

void arena_reset(Arena *arena){
    arena->used = 0;
}

void arena_free(Arena *arena){
    arena->used = 0;
    arena->capacity = 0;
    free(arena);
}

#endif // TUI_ARENA_IMPL
#endif // TUI_ARENA

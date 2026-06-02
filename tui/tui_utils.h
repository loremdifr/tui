#ifndef TUI_UTILS
#define TUI_UTILS

#include <bits/time.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#define private static
#define arr_size(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct {
    union{
        int x;
        int w;
        int col;
        int width;
    };
    union {
        int y;
        int h;
        int row;
        int height;
    };
} vec2i;

typedef struct {
    union{
        vec2i position;
        vec2i pos;
    };
    vec2i size;
} rect2i;

typedef void(*FunctionPointer)(void);

int sign(int val);
int min(int a, int b);
int max(int a, int b);
int clamp(int val, int min, int max);
int clamp_overflow(int val, int min, int max);
double get_curr_time(void);

//utf8 stuff
uint8_t        utf8_char_length(uint8_t byte);
int            utf8_str_length(const uint8_t *str);
const uint8_t *utf8_str_next_char(const uint8_t *curr_char);
uint8_t       *utf8_str_concat(uint8_t *dest, const uint8_t *src);
uint8_t       *utf8_str_concat_max(uint8_t *dest, const uint8_t *src, size_t limit);
uint32_t       utf8_pack(const uint8_t bytes[]);
void           utf8_unpack(const uint32_t packed_bytes, uint8_t *unpacked_bytes);

#ifdef TUI_UTILS_IMPL

//TODO: not portable
double get_curr_time(void){
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double) now.tv_sec + (double) now.tv_nsec / 1e9;
}

int sign(int val){
    if (val < 0) return -1;
    if (val > 0) return 1;
    return 0;
}

int min(int a, int b){
    return a < b ? a : b;
}
int max(int a, int b){
    return a > b ? a : b;
}

int clamp(int val, int min, int max){
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

int clamp_overflow(int val, int min, int max){
    if (val < min) return max;
    if (val >= max) return min;
    return val;
}


//utf 8 refs:
// - https://www.youtube.com/watch?v=vpSkBV5vydg
// - https://sethmlarson.dev/utf-8

uint8_t utf8_char_length(uint8_t byte){
    //utf8 is variable length, so it can have 1, 2, 3 or 4 bytes
    //the way to know how many it has is by inspecting the first one,
    //also known as the "leading byte"

    //starts with 0 -> 1 byte (ASCII was 7 bits)
    if (byte <= 0b01111111) return 1;
    //starts with 110 -> 2 bytes
    if (byte >= 0b11000000 && byte <= 0b11011111) return 2;
    //starts with 1110 -> 3 bytes
    if (byte >= 0b11100000 && byte <= 0b11101111) return 3;
    //starts with 11110 -> 4 bytes
    if (byte >= 0b11110000 && byte <= 0b11110111) return 4;

    //invalid utf8 leading byte, possibly a continuation byte
    return 0;
}

const uint8_t *utf8_str_next_char(const uint8_t *curr_char){
    uint8_t total_bytes = utf8_char_length(curr_char[0]);
    if(total_bytes == 0){
        //the whole byte is invalid as a leading byte,
        //so we skip it
        return curr_char + 1;
    }
    return curr_char + total_bytes;
}

int utf8_str_length(const uint8_t *str){
    if(str == NULL) return 0;
    assert(str != NULL);

    int length = 0;
    const uint8_t *curr_char = str;

    //walk string char by char
    while(*curr_char != '\0'){
        length++;
        curr_char = utf8_str_next_char(curr_char);
    }

    return length;
}

uint8_t *utf8_str_concat(uint8_t *dest, const uint8_t *src){
    if(dest == NULL) return NULL; //TODO: use nullptr instead?
    if(src == NULL) return dest;

    size_t dest_length = strlen((const char*)dest);
    size_t src_length  = strlen((const char*)src);
    for(size_t i = 0; i < src_length; i++){
        dest[dest_length + i] = src[i];
    }

    return dest;
}

// this method truncates the resulting string to ensure it always stays within the limit
uint8_t *utf8_str_concat_max(uint8_t *dest, const uint8_t *src, size_t limit){
    if(dest == NULL) return NULL; //TODO: use nullptr instead?
    if(src == NULL) return dest;

    size_t dest_length = strlen((const char*)dest);
    size_t src_length  = strlen((const char*)src);
    size_t i = 0;
    for(; i < src_length; i++){
        if(i + dest_length > limit - 1) break;
        dest[dest_length + i] = src[i];
    }
    dest[dest_length + i] = '\0'; //null terminator always!!

    return dest;
}

uint32_t utf8_pack(const uint8_t bytes[]){
    // inspect the first byte to know how many bytes there
    // are in total, and put them in order in a uint32
    auto total_bytes = utf8_char_length(bytes[0]);
    switch(total_bytes){
    case 1: return (uint32_t)bytes[0];
    case 2: return (uint32_t)bytes[0]
                 | (uint32_t)bytes[1] << 8;
    case 3: return (uint32_t)bytes[0]
                 | (uint32_t)bytes[1] << 8
                 | (uint32_t)bytes[2] << 8*2;
    case 4: return (uint32_t)bytes[0]
                 | (uint32_t)bytes[1] << 8
                 | (uint32_t)bytes[2] << 8*2
                 | (uint32_t)bytes[3] << 8*3;
    case 0: //utf8_leading_byte was NOT a leading byte
    default: //in theory we don't need default because
             //utf8_char_length should only return 0-4
        return 0;
    }
    //TODO: possible optimization, using out param since the caller
    //      will always need the total_bytes and we would be calling
    //      it twice in each loop
}

void utf8_unpack(const uint32_t packed_bytes, uint8_t *unpacked_bytes){
    constexpr uint8_t byte_mask = 0b11111111;
    unpacked_bytes[0] = (packed_bytes       ) & byte_mask;
    unpacked_bytes[1] = (packed_bytes >> 8  ) & byte_mask;
    unpacked_bytes[2] = (packed_bytes >> 8*2) & byte_mask;
    unpacked_bytes[3] = (packed_bytes >> 8*3) & byte_mask;
}

#endif //TUI_UTILS_IMPL
#endif //TUI_UTILS

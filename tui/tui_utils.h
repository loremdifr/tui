#ifndef TUI_UTILS
#define TUI_UTILS

#include <string.h>
#include <assert.h>
// #include <stdio.h>
// #include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#define private static

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
} vec2;

typedef struct {
    union{
        vec2 position;
        vec2 pos;
    };
    vec2 size;
} rect;

typedef void(*FunctionPointer)(void);

int clamp(int val, int min, int max);
int clamp_overflow(int val, int min, int max);
double get_curr_time(void);

//utf8 garbage
uint8_t utf8_char_length(uint8_t byte);
const uint8_t *utf8_str_next_char(const uint8_t *curr_char);
int utf8_str_length(const uint8_t *str);
uint8_t *utf8_str_concat(uint8_t *dest, const uint8_t *src);

#ifdef TUI_UTILS_IMPL

//TODO: not portable
double get_curr_time(void){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return (double) now.tv_sec + (double) now.tv_nsec / 1e9;
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

// private uint32_t utf8_char_to_uint32(const uint8_t *utf8_leading_byte){
//     // inspect the first byte to know how many bytes there
//     // are in total, and put them in order in a uint32
//     auto total_bytes = utf8_char_length(utf8_leading_byte[0]);
//     switch(total_bytes){
//     case 1: return (uint32_t)utf8_leading_byte[0];
//     case 2: return (uint32_t)utf8_leading_byte[0]
//                  | (uint32_t)utf8_leading_byte[1] << 8;
//     case 3: return (uint32_t)utf8_leading_byte[0]
//                  | (uint32_t)utf8_leading_byte[1] << 8
//                  | (uint32_t)utf8_leading_byte[2] << 8*2;
//     case 4: return (uint32_t)utf8_leading_byte[0]
//                  | (uint32_t)utf8_leading_byte[1] << 8
//                  | (uint32_t)utf8_leading_byte[2] << 8*2
//                  | (uint32_t)utf8_leading_byte[3] << 8*3;
//     case 0: //utf8_leading_byte was NOT a leading byte
//     default: //in theory we don't need default because
//              //utf8_char_length should only return 0-4
//         return 0;
//     }
//     //TODO: possible optimization, using out param since the caller
//     //      will always need the total_bytes and we would be calling
//     //      it twice in each loop
// }

// typedef struct {
//     uint8_t bytes[4];
// } UTF8Bytes;
// private UTF8Bytes utf8_uint32_to_char(uint32_t packed_utf8_char){
//     //ugh maybe we dont need any of this lets just save uint8[4] to the Cell instead
//     constexpr uint8_t byte_mask = 0b11111111;
//     UTF8Bytes utf8 = {};
//     utf8.bytes[0] = (packed_utf8_char       ) & byte_mask;
//     utf8.bytes[1] = (packed_utf8_char >> 8  ) & byte_mask;
//     utf8.bytes[2] = (packed_utf8_char >> 8*2) & byte_mask;
//     utf8.bytes[3] = (packed_utf8_char >> 8*3) & byte_mask;
//     return utf8;
// }


#endif //TUI_UTILS_IMPL
#endif //TUI_UTILS

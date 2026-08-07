#ifndef TUI_UTILS
#define TUI_UTILS

#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdarg.h>

#define arr_size(arr) (sizeof(arr) / sizeof(arr[0]))

#define COLOR_HEX(hex) ((vec3i){ \
    .red   = (hex >> 16) & 0xFF, \
    .green = (hex >> 8)  & 0xFF, \
    .blue  = (hex >> 0)  & 0xFF, \
})

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
        int x;
        int r;
        int red;
    };
    union {
        int y;
        int g;
        int green;
    };
    union {
        int z;
        int b;
        int blue;
    };
} vec3i;

typedef struct {
    union{
        vec2i position;
        vec2i pos;
    };
    vec2i size;
} rect2i;

typedef struct {
    union{
        float x;
        float w;
        float col;
        float width;
    };
    union {
        float y;
        float h;
        float row;
        float height;
    };
} vec2f;

typedef struct {
    union{
        vec2f position;
        vec2f pos;
    };
    vec2f size;
} rect2f;

typedef void(*FunctionPointer)(void);

int sign(int val);
int min(int a, int b);
int max(int a, int b);
int clamp(int val, int min, int max);
int clamp_overflow(int val, int min, int max);

float lerp(float min, float max, float weight);
float inverse_lerp(float min, float max, float value);
float remap(float val, float start_min, float start_max, float end_min, float end_max);

//utf8
//TODO: move to its own file..?
uint8_t        utf8_char_length(uint8_t byte);
uint8_t        utf8_char_display_width(const uint8_t bytes[static 4]);
uint32_t       utf8_pack(const uint8_t bytes[static 4]);
void           utf8_unpack(const uint32_t packed_bytes, uint8_t *unpacked_bytes);
void           utf8_codepoint_to_bytes(uint32_t codepoint, uint8_t bytes[static 4]);

//utf8_str
size_t         utf8_str_length(const uint8_t *str);
size_t         utf8_str_display_width(const uint8_t *str);
const uint8_t *utf8_str_next_char(const uint8_t *curr_char);
uint8_t       *utf8_str_concat(uint8_t *dest, const uint8_t *src);
uint8_t       *utf8_str_concat_max(uint8_t *dest, const uint8_t *src, size_t max_width);

#ifdef TUI_UTILS_IMPL

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

float lerp(float min, float max, float weight){
    //returns value from weight
    return min + (max - min) * weight;
}

float inverse_lerp(float min, float max, float value){
    //returns weight from value
    return (value - min) / (max - min);
}

float remap(float val, float start_min, float start_max, float end_min, float end_max){
    //remaps a value from one range to another
    return lerp(end_min, end_max, inverse_lerp(start_min, start_max, val));
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

size_t utf8_str_length(const uint8_t *str){
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

size_t utf8_str_display_width(const uint8_t *str){
    if(str == NULL) return 0;

    size_t display_width = 0;
    const uint8_t *char_pointer = str;

    while(*char_pointer != '\0'){
        //build the exact size the function expects
        uint8_t char_bytes[4] = {
            char_pointer[0],
            char_pointer[1],
            char_pointer[2],
            char_pointer[3]
        };
        display_width += utf8_char_display_width(char_bytes);
        char_pointer   = utf8_str_next_char(char_pointer);
    }

    return display_width;
}

uint8_t *utf8_str_concat(uint8_t *dest, const uint8_t *src){
    if(dest == NULL) return NULL; //TODO: use nullptr instead?
    if(src == NULL) return dest;

    size_t dest_length = strlen((const char*)dest);
    size_t src_length  = strlen((const char*)src);
    for(size_t i = 0; i < src_length; i++){
        dest[dest_length + i] = src[i];
    }

    dest[dest_length + src_length] = '\0'; //null terminator!!!!!!!

    return dest;
}

// truncates so total DISPLAY WIDTH stays within max_width
uint8_t *utf8_str_concat_max(uint8_t *dest, const uint8_t *src, size_t max_width){
    if(dest == NULL) return NULL;
    if(src == NULL)  return dest;

    size_t dest_width    = utf8_str_display_width(dest);
    size_t dest_bytes    = strlen((const char*)dest);
    size_t bytes_written = 0;

    while(*src != '\0'){
        //build the exact size the function expects
        uint8_t char_bytes[4] = {src[0], src[1], src[2], src[3]};
        uint8_t display_width = utf8_char_display_width(char_bytes);

        if (dest_width + display_width > max_width){
            //next char will go over limit, so we break
            break;
        }

        dest_width += display_width;

        //copy bytes until char
        //TODO: this should PROBABLY be its own function
        uint8_t char_length = utf8_char_length(src[0]);
        for(uint8_t i = 0; i < char_length; i++){
            dest[dest_bytes + bytes_written + i] = src[i];
        }
        bytes_written += char_length;

        //move pointer to next char
        src = utf8_str_next_char(src);
    }

    dest[dest_bytes + bytes_written] = '\0'; //null terminator!
    return dest;
}

uint32_t utf8_pack(const uint8_t bytes[static 4]){
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


uint32_t utf8_codepoint_from_bytes(const uint8_t bytes[static 4]){
    //to get the utf8 codepoint we have to remove the continuation bits from
    //each byte (first two), and then concat the other bits in order
    //NOTE: first 2 bits are the continuation byte marker, so we use multiple of 8-2=6

    uint8_t char_length = utf8_char_length(bytes[0]);
    constexpr uint8_t mask_cont  = 0b00111111;
    constexpr uint8_t mask_5bits = 0b00011111;
    constexpr uint8_t mask_4bits = 0b00001111;
    constexpr uint8_t mask_3bits = 0b00000111;

    switch(char_length){
        case 1: return (uint32_t)bytes[0];
        case 2: return ((uint32_t)(bytes[0] & mask_5bits) << 6)
                     |  (uint32_t)(bytes[1] & mask_cont);
        case 3: return ((uint32_t)(bytes[0] & mask_4bits) << 12)
                     | ((uint32_t)(bytes[1] & mask_cont)  << 6)
                     |  (uint32_t)(bytes[2] & mask_cont);
        case 4: return ((uint32_t)(bytes[0] & mask_3bits) << 18)
                     | ((uint32_t)(bytes[1] & mask_cont)  << 12)
                     | ((uint32_t)(bytes[2] & mask_cont)  << 6)
                     |  (uint32_t)(bytes[3] & mask_cont);
        default: return 0;
    }
}

void utf8_codepoint_to_bytes(uint32_t codepoint, uint8_t bytes[static 4]){
    //reverse operation

    //theres no leading byte here to use with utf8_char_length,
    //so we have check the range directly here to determine total bytes
    int total_bytes = 4;
    if(codepoint <= 0xFFFF) total_bytes = 3;
    if(codepoint <= 0x7FF)  total_bytes = 2;
    if(codepoint <= 0x7F)   total_bytes = 1;

    constexpr uint8_t mask_cont = 0b00111111;
    switch(total_bytes){
    case 1:
        bytes[0] = (uint8_t)codepoint;
        bytes[1] = 0;
        break;
    case 2:
        bytes[0] = (uint8_t)(0b11000000 |  (codepoint >> 6));
        bytes[1] = (uint8_t)(0b10000000 | (codepoint        & mask_cont));
        bytes[2] = 0;
        break;
    case 3:
        bytes[0] = (uint8_t)(0b11100000 |  (codepoint >> 12));
        bytes[1] = (uint8_t)(0b10000000 | ((codepoint >> 6) & mask_cont));
        bytes[2] = (uint8_t)(0b10000000 | (codepoint        & mask_cont));
        bytes[3] = 0;
        break;
    case 4:
        bytes[0] = (uint8_t)(0b11110000 |  (codepoint >> 18));
        bytes[1] = (uint8_t)(0b10000000 | ((codepoint >> 12) & mask_cont));
        bytes[2] = (uint8_t)(0b10000000 | ((codepoint >> 6)  & mask_cont));
        bytes[3] = (uint8_t)(0b10000000 | (codepoint         & mask_cont));
        break;
    default:
        assert(false); //???
    }
}

uint8_t utf8_char_display_width(const uint8_t bytes[static 4]){
    //unicode display width is kind of a mess and there's no easy way to know
    //the width of a character. so we add here whitelisted ranges as we need them.
    //we want to avoid corrupting the screen as much as possible, so it's better that
    //a character does not show than it breaking the rendering.

    uint32_t codepoint = utf8_codepoint_from_bytes(bytes);

    // narrow
    if(codepoint >= 0x20   && codepoint <= 0x7E)   return 1; //ascii
    if(codepoint >= 0xA0   && codepoint <= 0x17F)  return 1; //latin
    if(codepoint >= 0x400  && codepoint <= 0x52F)  return 1; //cyrillic
    if(codepoint >= 0x2000 && codepoint <= 0x206F) return 1; //punctuation and other smybols
    if(codepoint >= 0x2190 && codepoint <= 0x21FF) return 1; //arrows
    if(codepoint >= 0x2500 && codepoint <= 0x25FF) return 1; //box drawing chars and blocks
    if(codepoint >= 0x2800 && codepoint <= 0x28FF) return 1; //braille

    // w i d e
    if(codepoint >= 0x2600   && codepoint <= 0x27BF)  return 2; // misc siymbols
    if(codepoint >= 0x1F300  && codepoint <= 0x1FAFF) return 2; // emojis

    return 0;
}

#endif //TUI_UTILS_IMPL
#endif //TUI_UTILS

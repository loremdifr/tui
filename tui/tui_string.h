#ifndef TUI_STRING
#define TUI_STRING

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "tui_utils.h"

//this is pretty much a wrapper so that we dont have to deal with utf8 stuff
typedef struct {
    uint8_t *data;
    size_t   length;
    size_t   bytes;
    size_t   capacity;
} String;

// api
String string_from(uint8_t *storage, size_t capacity);
void   string_insert_at(String *str, size_t index, uint32_t new_char);
void   string_delete_at(String *str, size_t index);

// IMPLEMENTATION BELOW
#ifdef TUI_STRING_IMPL

//TODO: could this be utf8_pos_at() ?
private size_t string_byte_pos_from_index(String *str, size_t index){
    // in utf8 strings the amount of bytes does not correspond to the amount
    // of characters. characters in utf8 are variable length.
    if (index == 0) return 0;
    if (index >= str->length) return str->bytes;
    size_t curr_char = 0;
    size_t curr_byte = 0;
    while (curr_byte < str->bytes && curr_char < index) {
        curr_byte += utf8_char_length(str->data[curr_byte]);
        curr_char++;
    }
    return curr_byte;
}

String string_from(uint8_t *storage, size_t capacity) {
    String str = {
        .data     = storage,
        .capacity = capacity,
        .bytes = (storage != nullptr)
            ? strlen((const char*)storage)
            : 0,
        .length = (storage != nullptr)
            ? utf8_str_length(storage)
            : 0
    };
    return str;
}

void string_insert_at(String *str, size_t index, uint32_t new_char){
    assert(str != NULL);
    assert(str->data != NULL);
    auto byte_pos = string_byte_pos_from_index(str, index);

    //unpack the uin32 that could come from keyboard
    //TODO: maybe this function should receive this unpacked?
    uint8_t bytes[4];
    utf8_unpack(new_char, bytes);
    uint8_t char_length = utf8_char_length(bytes[0]);

    if(char_length == 0) return; // invalid character?
    if(str->bytes + char_length >= str->capacity){
        assert(false); // string is full!!!!!
        return;
    }

    // we split the string in two at the cursor,
    // move the right side 1 over, and then insert the char
    auto right_side        = str->data + byte_pos;
    auto right_side_length = strlen((const char *)right_side) + 1; //+1 for null
    memmove(right_side + char_length, right_side, right_side_length);

    // insert bytes of new char into the cleared position
    for (uint8_t i = 0; i < char_length; i++){
        str->data[byte_pos + i] = bytes[i];
    }

    str->bytes += char_length;
    str->length++;
}

void string_delete_at(String *str, size_t index) {
    assert(str != NULL);
    assert(str->data != NULL);
    auto byte_pos       = string_byte_pos_from_index(str, index);
    uint8_t char_length = utf8_char_length(str->data[byte_pos]);
    if(index >= str->length){
        //NOTE: we don't clamp it because you can be at the end of
        //     the string and press delete!
         return;
    }

    //TODO: how should we handle invalid bytes..?
    assert(char_length != 0);

    // we split the string in two at the cursor,
    // move the right side 1 to the left

    auto right_side        = str->data + byte_pos;
    auto right_side_length = strlen((const char *)(right_side + char_length)) + 1; //+1 for null
    memmove(right_side, right_side + char_length, right_side_length);

    str->bytes -= char_length;
    str->length--;
}

#endif // TUI_STRING_IMPL
#endif // TUI_STRING

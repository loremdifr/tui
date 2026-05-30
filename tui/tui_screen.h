#ifndef TUI_SCREEN
#define TUI_SCREEN

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "tui_utils.h"

typedef enum {
    NORMAL    = 0x00,
    BOLD      = 0x02,
    ITALIC    = 0x04,
    UNDERLINE = 0x08,
    BLINKING  = 0x10,
} TextFormat;

typedef enum {
    COLOR_DEFAULT,
    COLOR_WHITE,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_MAGENTA,
    COLOR_GRAY,
    COLOR_GRAY_DARK,
    COLOR_MAGENTA_DARK,
    //TODO: add more colors...
} Color;

typedef struct {
    uint8_t    bytes[4];    //utf8 is 4 bytes max per char
    uint8_t    bytes_used;
    TextFormat text_format;
    Color      fg_color;
    Color      bg_color;
    //not supported for now:
    // uint8_t display_width;
} Cell;

typedef struct{
	vec2i  size;
	Cell *cells;
} Screen;

Screen    screen_create(vec2i size);
void      screen_clear(Screen *screen);
void      screen_format(TextFormat text_format, Color fg_color, Color bg_color);
Cell     *screen_get(Screen *screen, int x, int y);
void      screen_set(Screen *screen, int x, int y, Cell cell);
void      screen_set_char(Screen *screen, int x, int y, char chr);
void      screen_fill_rect(vec2i pos, vec2i size, char c); //TODO: impl
void      screen_set_str(Screen *screen, int x, int y, const char *str);
void      screen_set_strf(Screen *screen, int x, int y, const char *format, ...);
void      screen_set_utf8(Screen *screen, int x, int y, const uint8_t *utf8);
void      screen_set_utf8_str(Screen *screen, int x, int y, const uint8_t *str);
void      screen_free(Screen *screen);

#ifdef TUI_SCREEN_IMPL

Screen screen_create(vec2i size){
	Screen screen = { .size = size };
	screen.cells = calloc(size.w * size.h, sizeof(Cell));
	return screen;
}

void screen_clear(Screen *screen){
     screen->cells = memset(
        screen->cells,
        0,
        screen->size.w * screen->size.h * sizeof(Cell)
    );
}

private struct {
    TextFormat text_format;
    Color      fg_color;
    Color      bg_color;
} FORMAT_CURR = {};

void screen_format(TextFormat text_format, Color fg_color, Color bg_color){
    FORMAT_CURR.text_format = text_format;
    FORMAT_CURR.fg_color = fg_color;
    FORMAT_CURR.bg_color = bg_color;
}

Cell *screen_get(Screen *screen, int x, int y){
	assert(screen != NULL);
	x = clamp(x, 0, screen->size.x - 1);
	y = clamp(y, 0, screen->size.y - 1);
    auto index = y * screen->size.x + x;
	return &screen->cells[index];
}

void screen_set(Screen *screen, int x, int y, Cell cell){
	assert(screen != NULL);

    cell.text_format = FORMAT_CURR.text_format;
    cell.fg_color    = FORMAT_CURR.fg_color;
    cell.bg_color    = FORMAT_CURR.bg_color;

	x = clamp(x, 0, screen->size.x - 1);
	y = clamp(y, 0, screen->size.y - 1);
    auto index = y * screen->size.x + x;
	screen->cells[index] = cell;
}

void screen_set_utf8(Screen *screen, int x, int y, const uint8_t *utf8){
    assert(screen != NULL);
    uint8_t total_bytes = utf8_char_length(utf8[0]);
    Cell cell = {
        .bytes = {},
        .bytes_used = total_bytes,
    };
    for(uint8_t i = 0; i < total_bytes; i++){
        cell.bytes[i] = utf8[i];
    }
    screen_set(screen, x, y, cell);
}

void screen_set_char(Screen *screen, int x, int y, char chr){
	assert(screen != NULL);
    Cell cell = {
        .bytes = {(uint8_t)chr, 0, 0, 0},
        .bytes_used = 1,
    };
    screen_set(screen, x, y, cell);
}

void screen_set_utf8_str(Screen *screen, int x, int y, const uint8_t *str){
    //TODO: probably need to assert that characters are actually 1 char wide somehow
	assert(screen != NULL);
    if(str == NULL) return;

    int curr_x = x;
    const uint8_t *curr_char = str;

    //walk string char by char
    while(*curr_char != '\0'){
    	if (curr_x >= screen->size.x) break; //TODO: word wrap

        auto char_bytes_used = utf8_char_length(curr_char[0]);
        Cell cell = {
            .bytes = {},
            .bytes_used = char_bytes_used,
        };
        //walk char byte by byte
        for (uint8_t i = 0; i < char_bytes_used; i++) {
            cell.bytes[i] = curr_char[i];
        }

        screen_set(screen, curr_x, y, cell);
        curr_x++;
        curr_char = utf8_str_next_char(curr_char);
    }

}

void screen_set_str(Screen *screen, int x, int y, const char *str){
    screen_set_utf8_str(screen, x, y, (const uint8_t *)str);
}

void screen_set_strf(Screen *grid, int x, int y, const char *format, ...){
	if (!grid || !format) return;

    char buffer[1024]; //TODO: maybe something else..?
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    assert(length < (int)sizeof(buffer)); //check never truncated?
    if (length <= 0) return;

    screen_set_str(grid, x, y, buffer);
}

void screen_free(Screen *screen){
	if(!screen) return;
	if(!screen->cells) return;
    free(screen->cells);
    screen->cells = NULL; // prevent double free
    					  // since we double buffer the screens, exit can
    					  // happen when both screens share cells
    // free(screen); // no need because our screens are
                     // part of APP_STATE and are globals
}

#endif //TUI_SCREEN_IMPL
#endif //TUI_SCREEN

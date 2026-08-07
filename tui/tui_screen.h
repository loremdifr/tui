#ifndef TUI_SCREEN
#define TUI_SCREEN

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tui_utils.h"
#include "tui_string.h"

typedef struct{
    vec3i fg;
    vec3i bg;
} ColorPair;

typedef enum{
    COLOR_TEXT,         // normal text
    COLOR_TEXT_FOCUS,   // highlighted or selected text, usually inverted
    COLOR_PANEL,        //
    COLOR_PANEL_FOCUS,  //
    COLOR_PRIMARY,      // for an elemented that is "active", like a hovered button
    COLOR_SECONDARY,
    COLOR_SUCCESS,
    COLOR_INFO,
    COLOR_WARNING,
    COLOR_DANGER,
    _THEME_COLOR_COUNT,
} ThemeColor;

typedef struct {
    char *name;
    ColorPair colors[_THEME_COLOR_COUNT];
} Theme;

typedef enum {
    NORMAL    = 0x00,
    BOLD      = 0x02,
    ITALIC    = 0x04,
    UNDERLINE = 0x08,
    BLINKING  = 0x10,
} TextFormat;

typedef struct {
    uint8_t    bytes[4];    //utf8 is 4 bytes max per char
    uint8_t    bytes_used;
    TextFormat text_format;
    ColorPair  colors;
    uint8_t    display_width;
} Cell;

typedef struct{
	vec2i  size;
	Cell  *cells;
    Theme  theme; //TODO: need to be able to set and get this!
} Screen;

Screen    screen_create(vec2i size);
void      screen_clear(Screen *screen);
void      screen_format(TextFormat text_format, ColorPair colors);
Cell     *screen_get(Screen *screen, int x, int y);
void      screen_set(Screen *screen, int x, int y, Cell cell);
void      screen_set_char(Screen *screen, int x, int y, char chr);
void      screen_fill_rect(vec2i pos, vec2i size, char c); //TODO: impl
void      screen_set_string(Screen *screen, int x, int y, String *str);
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

typedef struct {
    TextFormat text_format;
    ColorPair  colors;
} CurrentCellFormat;
static CurrentCellFormat FORMAT_CURR = {};

void screen_format(TextFormat text_format, ColorPair colors){
    FORMAT_CURR.text_format = text_format;
    FORMAT_CURR.colors = colors;
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
    cell.colors      = FORMAT_CURR.colors;

	x = clamp(x, 0, screen->size.x - 1);
	y = clamp(y, 0, screen->size.y - 1);
    auto index = y * screen->size.x + x;
	screen->cells[index] = cell;
}

void screen_set_utf8(Screen *screen, int x, int y, const uint8_t *utf8){
    assert(screen != NULL);
    uint8_t total_bytes   = utf8_char_length(utf8[0]);
    uint8_t display_width = utf8_char_display_width(utf8);
    Cell cell = {
        .bytes         = {},
        .bytes_used    = total_bytes,
        .display_width = display_width,
    };
    for(uint8_t i = 0; i < total_bytes; i++){
        cell.bytes[i] = utf8[i];
    }
    screen_set(screen, x, y, cell);
}

void screen_set_char(Screen *screen, int x, int y, char chr){
	assert(screen != NULL);
    Cell cell = {
        .bytes         = {(uint8_t)chr, 0, 0, 0},
        .bytes_used    = 1,
        .display_width = 1,
    };
    screen_set(screen, x, y, cell);
}

void screen_set_utf8_str(Screen *screen, int x, int y, const uint8_t *str){
    auto string = string_from((uint8_t *)str, strlen((char *)str));
    screen_set_string(screen, x, y, &string);
}

void screen_set_string(Screen *screen, int x, int y, String *str){
    assert(screen != NULL);
    if(str == NULL || str->data == NULL) return;

    int curr_x = x;
    const uint8_t *curr_char = str->data;
    const uint8_t *end_ptr = str->data + str->bytes;

    //walk string char by char
    while(curr_char < end_ptr){
        if(curr_x >= screen->size.x) break;

        auto char_bytes_used = utf8_char_length(curr_char[0]);
        if(char_bytes_used == 0){
            curr_char++;
            continue;
        }

        Cell cell = {
            .bytes = {},
            .bytes_used = char_bytes_used,
        };

        //walk char byte by byte
        for(uint8_t i = 0; i < char_bytes_used; i++){
            cell.bytes[i] = curr_char[i];
        }

        cell.display_width = utf8_char_display_width(curr_char);
        if(cell.display_width > 0){
            screen_set(screen, curr_x, y, cell);
        }
        curr_x += cell.display_width; //move x cursor according to char width

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


// write color + text format -----------------------------

constexpr uint8_t FORMAT_PARAMS_MAX = 16;
typedef struct{
    uint8_t params[FORMAT_PARAMS_MAX];
    char    str[80];
    uint8_t used;
} FormatParams;
static FormatParams FORMAT_PARAMS = {};
static inline void _tui_format_params_push(uint8_t param){
    FORMAT_PARAMS.params[FORMAT_PARAMS.used++] = param;
}
static inline void _tui_format_params_reset(){
    FORMAT_PARAMS.str[0] = '\0';
    FORMAT_PARAMS.used = 0;
}

static inline void _tui_write_color(
    TextFormat text_format,
    ColorPair colors
){
    static const char start[]     = "\033[";
    static const char separator[] = ";";
    static const char end[]       = "m";

    //formato
    if(text_format == NORMAL){
        _tui_format_params_push(0);
    }else{
        if(text_format & BOLD)      _tui_format_params_push(1);
        if(text_format & ITALIC)    _tui_format_params_push(3);
        if(text_format & UNDERLINE) _tui_format_params_push(4);
        if(text_format & BLINKING)  _tui_format_params_push(5);
    }

    //fg color
    _tui_format_params_push(38);
    _tui_format_params_push(2);
    _tui_format_params_push(colors.fg.red);
    _tui_format_params_push(colors.fg.green);
    _tui_format_params_push(colors.fg.blue);

    //bg color
    _tui_format_params_push(48);
    _tui_format_params_push(2);
    _tui_format_params_push(colors.bg.red);
    _tui_format_params_push(colors.bg.green);
    _tui_format_params_push(colors.bg.blue);

    //concatenar formato
    int terminator_pos = sprintf(FORMAT_PARAMS.str, "%s", start);
    char *next_str     = FORMAT_PARAMS.str + terminator_pos;
    for(int i = 0; i < FORMAT_PARAMS.used; i++){
        //ultimo param no usa el separator
        if(i == FORMAT_PARAMS.used - 1){
            next_str += sprintf(next_str, "%d%s", FORMAT_PARAMS.params[i], end);
        }else{
            next_str += sprintf(next_str, "%d%s", FORMAT_PARAMS.params[i], separator);
        }
    }

    tui_write(FORMAT_PARAMS.str);
    _tui_format_params_reset();
}


#endif //TUI_SCREEN_IMPL
#endif //TUI_SCREEN

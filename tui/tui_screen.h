#ifndef TUI_SCREEN
#define TUI_SCREEN

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "tui_utils.h"
#include "tui_string.h"

typedef enum {
    NORMAL    = 0x00,
    BOLD      = 0x02,
    ITALIC    = 0x04,
    UNDERLINE = 0x08,
    BLINKING  = 0x10,
} TextFormat;

typedef enum {
    COLOR_BLACK = 0,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
    COLOR_BRIGHT_BLACK,
    COLOR_BRIGHT_RED,
    COLOR_BRIGHT_GREEN,
    COLOR_BRIGHT_YELLOW,
    COLOR_BRIGHT_BLUE,
    COLOR_BRIGHT_MAGENTA,
    COLOR_BRIGHT_CYAN,
    COLOR_BRIGHT_WHITE,

    COLOR_DARK_BLACK,
    COLOR_DARK_RED,
    COLOR_DARK_GREEN,
    COLOR_DARK_YELLOW,
    COLOR_DARK_BLUE,
    COLOR_DARK_MAGENTA,
    COLOR_DARK_CYAN,
    COLOR_DARK_WHITE,

    // Aliases
    COLOR_GRAY         = 8,
    COLOR_GRAY_DARK    = 0,
    COLOR_MAGENTA_DARK = 5,

    COLOR_DEFAULT      = 256,
} Color;

typedef struct {
    uint8_t    bytes[4];    //utf8 is 4 bytes max per char
    uint8_t    bytes_used;
    TextFormat text_format;
    Color      fg_color;
    Color      bg_color;
    uint8_t    display_width;
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

constexpr uint8_t FORMAT_PARAMS_MAX = 8;
typedef struct{
    uint8_t params[FORMAT_PARAMS_MAX];
    char    str[80];
    uint8_t used;
} FormatParams;
private FormatParams FORMAT_PARAMS = {};
private inline void format_params_push(uint8_t param){
    FORMAT_PARAMS.params[FORMAT_PARAMS.used++] = param;
}
private inline void format_params_reset(){
    FORMAT_PARAMS.str[0] = '\0';
    FORMAT_PARAMS.used = 0;
}

private inline void tui_write_color(TextFormat text_format, Color fg_color, Color bg_color){
    static const char start[]     = "\033[";
    static const char separator[] = ";";
    static const char end[]       = "m";

    //formato
    if(text_format == NORMAL){
        format_params_push(0);
    }else{
        if(text_format & BOLD)      format_params_push(1);
        if(text_format & ITALIC)    format_params_push(3);
        if(text_format & UNDERLINE) format_params_push(4);
        if(text_format & BLINKING)  format_params_push(5);
    }

    //color frente
    switch (fg_color) {
        case COLOR_DEFAULT:        format_params_push(39); break;
        case COLOR_BLACK:          format_params_push(30); break;
        case COLOR_RED:            format_params_push(31); break;
        case COLOR_GREEN:          format_params_push(32); break;
        case COLOR_YELLOW:         format_params_push(33); break;
        case COLOR_BLUE:           format_params_push(34); break;
        case COLOR_MAGENTA:        format_params_push(35); break;
        case COLOR_CYAN:           format_params_push(36); break;
        case COLOR_WHITE:          format_params_push(37); break;
        case COLOR_BRIGHT_BLACK:   format_params_push(90); break;
        case COLOR_BRIGHT_RED:     format_params_push(91); break;
        case COLOR_BRIGHT_GREEN:   format_params_push(92); break;
        case COLOR_BRIGHT_YELLOW:  format_params_push(93); break;
        case COLOR_BRIGHT_BLUE:    format_params_push(94); break;
        case COLOR_BRIGHT_MAGENTA: format_params_push(95); break;
        case COLOR_BRIGHT_CYAN:    format_params_push(96); break;
        case COLOR_BRIGHT_WHITE:   format_params_push(97); break;

        case COLOR_DARK_BLACK:     format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(232);
                                   break;
        case COLOR_DARK_RED:       format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(88);
                                    break;
        case COLOR_DARK_GREEN:     format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(22);
                                    break;
        case COLOR_DARK_YELLOW:    format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(58);
                                    break;
        case COLOR_DARK_BLUE:      format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(18);
                                    break;
        case COLOR_DARK_MAGENTA:   format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(90);
                                    break;
        case COLOR_DARK_CYAN:      format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(23);
                                    break;
        case COLOR_DARK_WHITE:     format_params_push(38);
                                   format_params_push(5);
                                   format_params_push(244);
                                   break;

        default: assert(false);
    }

    //color fondo
    switch (bg_color) {
        case COLOR_DEFAULT:        format_params_push(49);  break;
        case COLOR_BLACK:          format_params_push(40);  break;
        case COLOR_RED:            format_params_push(41);  break;
        case COLOR_GREEN:          format_params_push(42);  break;
        case COLOR_YELLOW:         format_params_push(43);  break;
        case COLOR_BLUE:           format_params_push(44);  break;
        case COLOR_MAGENTA:        format_params_push(45);  break;
        case COLOR_CYAN:           format_params_push(46);  break;
        case COLOR_WHITE:          format_params_push(47);  break;
        case COLOR_BRIGHT_BLACK:   format_params_push(100); break;
        case COLOR_BRIGHT_RED:     format_params_push(101); break;
        case COLOR_BRIGHT_GREEN:   format_params_push(102); break;
        case COLOR_BRIGHT_YELLOW:  format_params_push(103); break;
        case COLOR_BRIGHT_BLUE:    format_params_push(104); break;
        case COLOR_BRIGHT_MAGENTA: format_params_push(105); break;
        case COLOR_BRIGHT_CYAN:    format_params_push(106); break;
        case COLOR_BRIGHT_WHITE:   format_params_push(107); break;

        case COLOR_DARK_BLACK:     format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(232);
                                   break;
        case COLOR_DARK_RED:       format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(88);
                                   break;
        case COLOR_DARK_GREEN:     format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(22);
                                   break;
        case COLOR_DARK_YELLOW:    format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(58);
                                   break;
        case COLOR_DARK_BLUE:      format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(18);
                                   break;
        case COLOR_DARK_MAGENTA:   format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(90);
                                   break;
        case COLOR_DARK_CYAN:      format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(23);
                                   break;
        case COLOR_DARK_WHITE:     format_params_push(48);
                                   format_params_push(5);
                                   format_params_push(244);
                                   break;

        default: assert(false);
    }

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
    format_params_reset();
}


#endif //TUI_SCREEN_IMPL
#endif //TUI_SCREEN

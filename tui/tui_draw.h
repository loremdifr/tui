#ifndef TUI_DRAW
#define TUI_DRAW

#include <stdint.h>
#include <string.h>
// #include "tui_platform.h"
#include "tui_utils.h"
#include "tui_screen.h"

void tui_draw_box(Screen *screen, rect rect);
void tui_draw_box_connected(Screen *screen, rect rect);
void tui_draw_box_connected_cell(Screen *screen, int x, int y);
void tui_draw_line(Screen *screen, uint8_t *utf8_char, vec2 from, vec2 to);
//TODO: we need lines and squares and all that too

#ifdef TUI_DRAW_IMPL

constexpr uint8_t BOX[]    = u8"▢";
constexpr uint8_t BOX_H[]  = u8"─";
constexpr uint8_t BOX_V[]  = u8"│";
constexpr uint8_t BOX_TL[] = u8"╭";//u8"┌";
constexpr uint8_t BOX_TR[] = u8"╮";//u8"┐";
constexpr uint8_t BOX_BL[] = u8"╰";//u8"└";
constexpr uint8_t BOX_BR[] = u8"╯";//u8"┘";
constexpr uint8_t BOX_VR[] = u8"├";
constexpr uint8_t BOX_VL[] = u8"┤";
constexpr uint8_t BOX_HB[] = u8"┬";
constexpr uint8_t BOX_HT[] = u8"┴";
constexpr uint8_t BOX_HV[] = u8"┼";

void tui_draw_box(Screen *screen, rect rect){
    //this function only draws the border given, without looking at any neighbors
    //therefore, it doesnt "connect it"

    if(rect.size.w == 1 || rect.size.h == 1){
        screen_set_utf8(screen, rect.pos.x, rect.pos.y, BOX);
        return;
    }
    if(rect.size.w < 2 || rect.size.h < 2){
        //invalid, draw nothing
        return;
    }

    //borders
    for(int x = 0; x < rect.size.width; x++){
        screen_set_utf8(screen, x + rect.pos.x, rect.pos.y,                 BOX_H);
        screen_set_utf8(screen, x + rect.pos.x, rect.pos.y + rect.size.h-1, BOX_H);
    }
    for(int y = 0; y < rect.size.height; y++){
        screen_set_utf8(screen, rect.pos.x,                 y + rect.pos.y, BOX_V);
        screen_set_utf8(screen, rect.pos.x + rect.size.w-1, y + rect.pos.y, BOX_V);
    }

    //corners
    screen_set_utf8(screen, rect.pos.x,                 rect.pos.y, BOX_TL);
    screen_set_utf8(screen, rect.pos.x + rect.size.w-1, rect.pos.y, BOX_TR);
    screen_set_utf8(screen, rect.pos.x,                 rect.pos.y + rect.size.h-1, BOX_BL);
    screen_set_utf8(screen, rect.pos.x + rect.size.w-1, rect.pos.y + rect.size.h-1, BOX_BR);
}

void tui_draw_box_connected(Screen *screen, rect rect){
    //this function evaluates each neighbor cell before placing the new one,
    //therefore, ensuring it connects with whats already in the grid
    for(int x = 0; x < rect.size.width; x++){
        tui_draw_box_connected_cell(screen, x + rect.pos.x, rect.pos.y);
        tui_draw_box_connected_cell(screen, x + rect.pos.x, rect.pos.y + rect.size.h-1);
    }
    for(int y = 0; y < rect.size.height; y++){
        tui_draw_box_connected_cell(screen, rect.pos.x,                 y + rect.pos.y);
        tui_draw_box_connected_cell(screen, rect.pos.x + rect.size.w-1, y + rect.pos.y);
    }
}

private bool is_box_character(Screen *screen, int x, int y){
    Cell *cell = screen_get(screen, x, y);
    uint8_t *bytes = cell->bytes;
    static const uint8_t *box_chars[] = {
        BOX,
        BOX_H,
        BOX_V,
        BOX_TL,
        BOX_TR,
        BOX_BL,
        BOX_BR,
        BOX_VR,
        BOX_VL,
        BOX_HB,
        BOX_HT,
        BOX_HV,
    };

    for(int i = 0; i < 12; i++){
        if(memcmp(bytes, box_chars[i], 4) == 0){
            return true;
        }
    }
    return false;
}

void tui_draw_box_connected_cell(Screen *screen, int x, int y){
    //possible connections
    constexpr uint8_t UP    = 0b0001;
    constexpr uint8_t DOWN  = 0b0010;
    constexpr uint8_t LEFT  = 0b0100;
    constexpr uint8_t RIGHT = 0b1000;

    bool has_up    = y > 0                && is_box_character(screen,x, y-1);
    bool has_down  = y < screen->size.h-1 && is_box_character(screen,x, y+1);
    bool has_left  = x > 0                && is_box_character(screen,x-1, y);
    bool has_right = x < screen->size.w-1 && is_box_character(screen,x+1, y);

    uint8_t neighbors = 0b0000;
    if(has_up)    neighbors |= UP;
    if(has_down)  neighbors |= DOWN;
    if(has_left)  neighbors |= LEFT;
    if(has_right) neighbors |= RIGHT;

    switch(neighbors){
    case UP:
    case DOWN:
    case UP | DOWN:                screen_set_utf8(screen, x, y, BOX_V);  break;
    case LEFT:
    case RIGHT:
    case LEFT | RIGHT:             screen_set_utf8(screen, x, y, BOX_H);  break;
    case UP | LEFT:                screen_set_utf8(screen, x, y, BOX_BR); break;
    case UP | RIGHT:               screen_set_utf8(screen, x, y, BOX_BL); break;
    case DOWN | LEFT:              screen_set_utf8(screen, x, y, BOX_TR); break;
    case DOWN | RIGHT:             screen_set_utf8(screen, x, y, BOX_TL); break;
    case UP | DOWN | LEFT:         screen_set_utf8(screen, x, y, BOX_VR); break;
    case UP | DOWN | RIGHT:        screen_set_utf8(screen, x, y, BOX_VL); break;
    case LEFT | RIGHT | UP:        screen_set_utf8(screen, x, y, BOX_HB); break;
    case LEFT | RIGHT | DOWN:      screen_set_utf8(screen, x, y, BOX_HT); break;
    case UP | DOWN | LEFT | RIGHT: screen_set_utf8(screen, x, y, BOX_HV); break;
    default:                       screen_set_utf8(screen, x, y, BOX);    break;
    }
}

void tui_draw_line(Screen *screen, uint8_t *utf8_char, vec2 from, vec2 to){
    if(screen == NULL || utf8_char == NULL) return;

    from.x = clamp(from.x, 0, screen->size.x - 1);
    from.y = clamp(from.y, 0, screen->size.y - 1);

    if(from.x == to.x){
        int step = from.y <= to.y ? 1 : -1;
        for(int y = from.y; ; y += step){
            if(y < 0 || y >= screen->size.y) return;
            screen_set_utf8(screen, from.x, y, utf8_char);
            if(y == to.y) return;
        }
    }

    if(from.y == to.y){
        int step = from.x <= to.x ? 1 : -1;
        for(int x = from.x; ; x += step){
            if(x < 0 || x >= screen->size.x) return;
            screen_set_utf8(screen, x, from.y, utf8_char);
            if(x == to.x) return;
        }
    }

    int x = from.x;
    int y = from.y;
    int dx = abs(to.x - from.x);
    int dy = abs(to.y - from.y);
    int sx = from.x < to.x ? 1 : -1;
    int sy = from.y < to.y ? 1 : -1;
    int err = dx - dy;

    while(true){
        if(x < 0 || x >= screen->size.x || y < 0 || y >= screen->size.y) return;

        screen_set_utf8(screen, x, y, utf8_char);
        if(x == to.x && y == to.y) return;

        int twice_err = err * 2;
        if(twice_err > -dy){
            err -= dy;
            x += sx;
        }
        if(twice_err < dx){
            err += dx;
            y += sy;
        }
    }
}

#endif //TUI_DRAW_IMPL
#endif //TUI_DRAW

#ifndef TUI_DRAW
#define TUI_DRAW

#include <stdint.h>
#include <string.h>
#include <math.h>
// #include "tui_platform.h"
#include "tui_utils.h"
#include "tui_screen.h"
#include "tui_string.h"

//TODO: not sure about this type name
typedef enum {
    BOX_TITLE_TOP_LEFT,
    BOX_TITLE_TOP_RIGHT,
    BOX_TITLE_BOTTOM_LEFT,
    BOX_TITLE_BOTTOM_RIGHT,
} BoxTitleAnchor;

void tui_draw_box(Screen *screen, rect2i rect);
void tui_draw_box_connected(Screen *screen, rect2i rect);
void tui_draw_box_connected_cell(Screen *screen, int x, int y);
void tui_draw_box_title(Screen *screen, rect2i box, String *title, BoxTitleAnchor anchor);
void tui_draw_line(Screen *screen, uint8_t *utf8_char, vec2i from, vec2i to);
void tui_draw_line_bresenham(Screen *screen, uint8_t *utf8_char, vec2i from, vec2i to);
void tui_draw_rect(Screen *screen, uint8_t *utf8_char, rect2i rect); //TODO:
void tui_draw_circ(Screen *screen, uint8_t *utf8_char, rect2i rect); //TODO:
void tui_draw_line_braille(Screen *screen, vec2i from, vec2i to);
void tui_draw_scrollbar_vertical(Screen *screen, vec2i from, vec2i to, int total_size, int shown_from, int shown_to);
void tui_draw_scrollbar_horizontal(Screen *screen, vec2i from, vec2i to, int total_size, int shown_from, int shown_to);

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

void tui_draw_box(Screen *screen, rect2i rect){
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

void tui_draw_box_connected(Screen *screen, rect2i rect){
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

void tui_draw_box_title(Screen *screen, rect2i box, String *title, BoxTitleAnchor anchor){
    size_t title_width = utf8_str_display_width(title->data);

    size_t x = 0;
    size_t y = 0;

    size_t left   = box.pos.x + 4;
    size_t right  = box.pos.x + box.size.w - (title_width) - 8;
    size_t top    = box.pos.y;
    size_t bottom = box.pos.y + box.size.h - 1;

    switch(anchor){
        case BOX_TITLE_TOP_LEFT:     x = left;  y = top;    break;
        case BOX_TITLE_TOP_RIGHT:    x = right; y = top;    break;
        case BOX_TITLE_BOTTOM_LEFT:  x = left;  y = bottom; break;
        case BOX_TITLE_BOTTOM_RIGHT: x = right; y = bottom; break;
    }

    screen_set_utf8(  screen, x,                 y, u8"╸");
    screen_set_char(  screen, x+1,               y, ' ');
    screen_set_string(screen, x+2,               y, title);
    screen_set_char(  screen, x+2+title_width,   y, ' ');
    screen_set_utf8(  screen, x+2+title_width+1, y, u8"╺");
}

void tui_draw_line(Screen *screen, uint8_t *utf8_char, vec2i from, vec2i to){
    if(screen == NULL || utf8_char == NULL) return;

    from.x = clamp(from.x, 0, screen->size.x - 1);
    from.y = clamp(from.y, 0, screen->size.y - 1);

    bool is_horizontal = (from.x == to.x);
    bool is_vertical   = (from.y == to.y);

    if(is_horizontal){
        int direction = from.y < to.y ? 1 : -1;
        while(true){
            if(from.y < 0) break;
            if(from.y >= screen->size.y) break;
            screen_set_utf8(screen, from.x, from.y, utf8_char);
            if(from.y == to.y) break;
            from.y += direction;
        }
        return;
    }

    if(is_vertical){
        int direction = from.x < to.x ? 1 : -1;
        while(true){
            if(from.x < 0) break;
            if(from.x >= screen->size.x) break;
            screen_set_utf8(screen, from.x, from.y, utf8_char);
            if(from.x == to.x) break;
            from.x += direction;
        }
        return;
    }

    //bresenham
    tui_draw_line_bresenham(screen, utf8_char, from, to);
}


//TODO: actually test this!
//source: https://github.com/godotengine/godot/blob/fa09dd17a68a5741cb2361f1d07af271c7a40c4f/core/math/geometry_2d.h#L464
void tui_draw_line_bresenham(Screen *screen, uint8_t *utf8_char, vec2i from, vec2i to){
    //give me operator overloading  PLEASE
    vec2i diff     = {.x = to.x - from.x,  .y = to.y - from.y};
    vec2i diff_abs = {.x = abs(diff.x),    .y = abs(diff.y)};
    vec2i delta    = {.x = diff_abs.x * 2, .y = diff_abs.y * 2};
    vec2i step     = {.x = sign(diff.x),   .y = sign(diff.y)};
    vec2i current  = from;

    if(delta.x > delta.y){
        int err = delta.x / 2;
        for(; current.x != to.x; current.x += step.x){
            screen_set_utf8(screen, current.x, current.y, utf8_char);
            err -= delta.y;
            if(err < 0){
                current.y += step.y;
                err += delta.x;
            }
        }
    }else{
        int err = delta.y / 2;
        for(; current.y != to.y; current.y += step.y){
            screen_set_utf8(screen, current.x, current.y, utf8_char);
            err -= delta.x;
            if(err < 0){
                current.x += step.x;
                err += delta.y;
            }
        }
    }
    screen_set_utf8(screen, current.x, current.y, utf8_char);
}

void tui_draw_rect(Screen *screen, uint8_t *utf8_char, rect2i rect){
    for(int x = rect.pos.x; x < rect.pos.x + rect.size.w; x++){
        for(int y = rect.pos.y; y < rect.pos.y + rect.size.h; y++){
            screen_set_utf8(screen, x, y, utf8_char);
        }
    }
}

void tui_draw_circ(Screen *screen, uint8_t *utf8_char, rect2i rect){
    int center_x   = rect.pos.x + rect.size.w / 2;
    int center_y   = rect.pos.y + rect.size.h / 2;
    int radius_x   = rect.size.w / 2;
    int radius_y   = rect.size.h / 2;

    for(int y = -radius_y; y <= radius_y; y++){
        for(int x = -radius_x; x <= radius_x; x++){
            float normal_x = (float)x / radius_x;
            float normal_y = (float)y / radius_y;
            float squared_distance = normal_x * normal_x + normal_y * normal_y;
            bool inside_ellipse = squared_distance<= 1.0f;
            if(inside_ellipse){
                screen_set_utf8(screen, center_x + x, center_y + y, utf8_char);
            }
        }
    }
}

void tui_draw_scrollbar_vertical(Screen *screen, vec2i from, vec2i to, int total_size, int shown_from, int shown_to){
    //TODO: move these to top level?
    static uint8_t *SCROLL_CAP_TOP    = u8"╵";
    static uint8_t *SCROLL_CAP_BOTTOM = u8"╷";
    static uint8_t *SCROLL_BG         = u8"▋";
    static uint8_t *SCROLL_KNOB       = u8"▋";

    //draw end caps
    screen_set_utf8(screen, from.x, from.y, SCROLL_CAP_TOP);
    screen_set_utf8(screen, to.x, to.y, SCROLL_CAP_BOTTOM);

    from.y += 1;
    to.y -= 1;

    //draw scroll background
    screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
    tui_draw_line(screen, SCROLL_BG, from, to);

    //size of the knob
    auto  scrollbar_size = to.y - from.y;
    auto  shown_size     = shown_to - shown_from;

    float shown_prt = (float)shown_size / (float)total_size;
    int   knob_size = (int)(scrollbar_size * shown_prt);
    knob_size       = clamp(knob_size, 1, scrollbar_size);

    //pos of the knob
    float scrolled_prt = (float)shown_from / total_size;
    auto  knob_from = (vec2i){
        .x = from.x,
        .y = from.y + ceil(scrolled_prt * scrollbar_size)
    };

    auto knob_to = (vec2i){.x = from.x, .y = knob_from.y + knob_size};

    //draw the knob
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    tui_draw_line(screen, SCROLL_KNOB, knob_from, knob_to);
}

void tui_draw_scrollbar_horizontal(Screen *screen, vec2i from, vec2i to, int total_size, int shown_from, int shown_to){
    //TODO: move these to top level?
    //TODO: surely this can be refactored somehow...
    static uint8_t *SCROLL_CAP_LEFT  = u8"╴";
    static uint8_t *SCROLL_CAP_RIGHT = u8"╶";
    static uint8_t *SCROLL_BG        = u8"▂";
    static uint8_t *SCROLL_KNOB      = u8"▂";

    //draw end caps
    screen_set_utf8(screen, from.x, from.y, SCROLL_CAP_LEFT);
    screen_set_utf8(screen, to.x, to.y, SCROLL_CAP_RIGHT);

    from.x += 1;
    to.x   -= 1;

    //draw scroll background
    screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
    tui_draw_line(screen, SCROLL_BG, from, to);

    //size of the knob
    auto  scrollbar_size  = to.x - from.x;
    auto  shown_size      = shown_to - shown_from;

    float shown_prt = (float)shown_size / (float)total_size;
    int   knob_size = (int)(scrollbar_size * shown_prt);
    knob_size       = clamp(knob_size, 1, scrollbar_size);

    //pos of the knob
    float scrolled_prt = (float)shown_from / total_size;
    auto  knob_from = (vec2i){
        .x = from.x + (int)(scrolled_prt * scrollbar_size),
        .y = from.y
    };

    auto knob_to = (vec2i){.x = knob_from.x + knob_size, .y = from.y};

    //draw the knob
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    tui_draw_line(screen, SCROLL_KNOB, knob_from, knob_to);
}


//BRAILLE STUFF ---------------------------------------------------------------
//source https://github.com/asciimoo/drawille
constexpr int     BRAILLE_W = 2;
constexpr int     BRAILLE_H = 4;

private void braille_plot_dot(Screen *screen, int dot_x, int dot_y){
    static const uint8_t braille_bit_map[2][4] = { //transpuesta?
        {0, 1, 2, 6},
        {3, 4, 5, 7},
    };

    //convert coords
    int cell_x = dot_x / BRAILLE_W;
    int cell_y = dot_y / BRAILLE_H;

    //out of bounds checks
    if(cell_x < 0) return;
    if(cell_y < 0) return;
    if(cell_x >= screen->size.x) return;
    if(cell_y >= screen->size.y) return;

    //map the bit to the dot
    uint8_t dot_bit = (uint8_t)(1 << braille_bit_map[dot_x & 1][dot_y & 3]);
    Cell *cell      = screen_get(screen, cell_x, cell_y);
    uint8_t mask    = dot_bit;

    //merge with existing braille dots already in cell
    uint32_t codepoint = utf8_codepoint_from_bytes(cell->bytes);
    if(codepoint >= 0x2800 && codepoint <= 0x28FF){
        mask |= (codepoint & 0xFF);
    }

    uint8_t bytes[4];
    utf8_codepoint_to_bytes(0x2800 | mask, bytes);
    screen_set_utf8(screen, cell_x, cell_y, bytes);
}

private void braille_set_center(Screen *screen, int cell_x, int cell_y){
    constexpr uint8_t BRAILLE_CENTER_DOTS = 0x36; //bits 1,2,4,5 = 2+4+16+32 = 54 = 0x36

    uint8_t mask = BRAILLE_CENTER_DOTS;
    Cell *cell   = screen_get(screen, cell_x, cell_y);

    //merge with existing braille dots already in cell
    uint32_t codepoint = utf8_codepoint_from_bytes(cell->bytes);
    if(codepoint >= 0x2800 && codepoint <= 0x28FF){
        mask |= (codepoint & 0xFF);
    }

    uint8_t bytes[4];
    utf8_codepoint_to_bytes(0x2800 | mask, bytes);
    screen_set_utf8(screen, cell_x, cell_y, bytes);
}

void tui_draw_line_braille(Screen *screen, vec2i from, vec2i to){
    from.x = clamp(from.x, 0, screen->size.x - 1);
    from.y = clamp(from.y, 0, screen->size.y - 1);
    to.x   = clamp(to.x,   0, screen->size.x - 1);
    to.y   = clamp(to.y,   0, screen->size.y - 1);

    //cell coordinates to  dot coordinates (center of cell)
    vec2i from_dot = {.x = from.x * BRAILLE_W + BRAILLE_W / 2,
                      .y = from.y * BRAILLE_H + BRAILLE_H / 2};
    vec2i to_dot   = {.x = to.x * BRAILLE_W + BRAILLE_W / 2,
                      .y = to.y * BRAILLE_H + BRAILLE_H / 2};

    //now we do bresenham but at dots instead of cells

    vec2i diff     = {.x = to_dot.x - from_dot.x, .y = to_dot.y - from_dot.y};
    vec2i diff_abs = {.x = abs(diff.x),           .y = abs(diff.y)};
    vec2i delta    = {.x = diff_abs.x * 2,        .y = diff_abs.y * 2};
    vec2i step     = {.x = sign(diff.x),          .y = sign(diff.y)};
    vec2i current  = from_dot;

    bool steep = delta.x > delta.y;
    if(steep){
        int err = delta.x / 2;
        for(; current.x != to_dot.x; current.x += step.x){
            braille_plot_dot(screen, current.x, current.y);
            err -= delta.y;
            if(err < 0){
                current.y += step.y;
                err += delta.x;
            }
        }
    }else{
        int err = delta.y / 2;
        for(; current.y != to_dot.y; current.y += step.y){
            braille_plot_dot(screen, current.x, current.y);
            err -= delta.x;
            if(err < 0){
                current.x += step.x;
                err += delta.y;
            }
        }
    }
    braille_plot_dot(screen, current.x, current.y);

    //ensure four central dots at starting and ending cell
    braille_set_center(screen, from.x, from.y);
    braille_set_center(screen, to.x,   to.y);
}

#endif //TUI_DRAW_IMPL
#endif //TUI_DRAW

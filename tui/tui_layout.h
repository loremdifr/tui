#ifndef TUI_LAYOUT
#define TUI_LAYOUT

#include <assert.h>
#include <string.h>

#include "tui_utils.h"
#include "tui_screen.h"
#include "tui_platform.h"
#include "tui_arena.h"

#ifndef TUI_DRAW_IMPL
#define TUI_DRAW_IMPL
#endif //TUI_DRAW_IMPL
#include "tui_draw.h"

typedef enum {
    LAYOUT_SINGLE_PANEL,
    LAYOUT_SIDEBAR_LEFT,
    LAYOUT_SIDEBAR_RIGHT,
    LAYOUT_SPLIT_VERTICAL,
    LAYOUT_WITH_HEADER,
    LAYOUT_WITH_FOOTER,
    LAYOUT_WITH_HEADER_AND_FOOTER,
    LAYOUT_SPLIT_VERTICAL_WITH_HEADER,
    //TODO: add more..?
} PageLayout;

typedef enum {
    SLOT_MAIN,
    SLOT_SIDEBAR,
    SLOT_TOP,
    SLOT_BOTTOM,
    SLOT_OVERLAY,
    SLOT_WIDGETS_OVERLAY_DO_NOT_USE,
} PanelSlot;

typedef struct Widget Widget;
typedef bool (*WidgetInputFunction  )(Widget *widget, InputEvent input_event);
typedef void (*WidgetRenderFunction )(Widget *widget, Screen *screen, vec2i position);
typedef void (*WidgetOverlayFunction)(Widget *widget);

struct Widget {
    const char           *id;
    vec2i                 size;
    void                 *data;
    void                 *state; //data that survives frames
    bool                  focusable;
    bool                  focused;
    bool                  is_inline;
    WidgetInputFunction   input;
    WidgetRenderFunction  render;
    WidgetOverlayFunction build_overlay_panel;
};

private constexpr int TUI_WIDGETS_IN_PANEL_MAX = 32;
typedef struct {
    PanelSlot slot;
    bool      focused;
    Widget    widgets[TUI_WIDGETS_IN_PANEL_MAX];
    uint8_t   widget_count;
    rect2i    outer_rect;    //cached
    rect2i    inner_rect;    //cached
    rect2i    widgets_rect;  //total accumulated rect around the widgets
    vec2i     curr_row_size;
} Panel;

typedef struct {
    const uint8_t *text;
    Color fg_color;
    Color bg_color;
} AnimationFrame;

//API TO DEFIN THE PANELS AND WIDGETS ON THE PAGE
void tui_panel_begin(PanelSlot slot);
void tui_panel_end(void);

//widgets
char *tui_create_widget_id();
void  tui_widget_push(Widget widget);
void *tui_widget_state(const char *widget_id, size_t data_size);

//used in the actual rendering process by tui.h
void tui_layout_prepare(Screen *screen, PageLayout layout);
void tui_layout_prepare_widget_overlay(void);
bool tui_widget_focused_input(InputEvent input_event);
void tui_layout_render(void); //actually rendering to the screen

//focus/navigation
void tui_cursor_next_widget(void);
void tui_cursor_prev_widget(void);
void tui_cursor_next_panel(void);
void tui_cursor_prev_panel(void);
void tui_panel_scroll(int offset);
void tui_panel_scroll_to(int widget_index); //TODO: widget index or widget pointer?
void tui_panel_scroll_up(void);
void tui_panel_scroll_down(void);

#ifdef TUI_LAYOUT_IMPL

private constexpr int PADDING = 1;
private constexpr int BORDER = 1;
private constexpr int TUI_PANELS_MAX = 12;
private constexpr int TUI_WIDGET_STATES_MAX = TUI_PANELS_MAX * TUI_WIDGETS_IN_PANEL_MAX;

typedef struct {
    const char *widget_id;
    void       *state_data;
    size_t      state_size;
} WidgetState;

typedef struct {
    WidgetState states[TUI_WIDGET_STATES_MAX]; //survives through frames!
    size_t      states_count;
    Arena       *arena;
} WidgetStateRegistry;

private WidgetStateRegistry WIDGET_REGISTRY = {.arena  = nullptr};

typedef struct {
    vec2i        base_size;
    Panel        panels[TUI_PANELS_MAX];
    uint8_t      panel_count;
    int8_t       panel_curr; // -1 = no panel selected
    uint8_t      panel_focused; //at least one panel always focused
    int          panel_focused_prev;
    //TODO: could we refactor this whole thing into being a panel like all the others..??
    int          widget_overlay_focused_prev;
    int          widget_overlay_owner_panel;
    const char  *widget_overlay_owner_widget;
    //--------------------
    uint8_t      widget_auto_id;
    const char  *widget_focused[TUI_PANELS_MAX]; //one id per panel
    int          panel_scroll_offset[TUI_PANELS_MAX]; //one per panel
    PageLayout   layout;
    Screen      *screen;
    Arena       *arena_frame;
} LayoutState;

private LayoutState LAYOUT_STATE = {
    .panel_curr         = -1,
    .panel_focused_prev = -1,
    .widget_overlay_focused_prev = -1,
    .widget_overlay_owner_panel = -1,
    .widget_overlay_owner_widget = nullptr,
    .arena_frame        = nullptr,
};

private void tui_widget_row_begin(Panel *panel);
private void tui_widget_row_push(Panel *panel, Widget *widget);
private void tui_widget_row_end(Panel *panel);

private inline int center_in_container(int base, int length, int container_length){
    base += container_length / 2 - length / 2;
    //cant do -1 on h above because it's an int and it doesnt accumulate the 0.5 after division
    //we do it twice separatedly to avoid reaching negativess
    if(base > 1) base--;
    if(base > 1) base--;
    return base;
}

private void tui_render_widget(Widget *widget, vec2i position){
    widget->render(widget, LAYOUT_STATE.screen, position);
    //always reset color after a widget!
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
}

private void tui_render_panel(Panel *panel, int scroll_offset){
    const int BASE_X = panel->outer_rect.pos.x + BORDER + PADDING + 1;
    const int BASE_Y = panel->outer_rect.pos.y + BORDER + PADDING;
    vec2i cursor_pos = {.x = BASE_X, .y = BASE_Y};

    //in case there was still an open inline row from the definition pass
    tui_widget_row_end(panel);

    //panels always render their content centered vertically
    //widget heights are precomputed


    cursor_pos.y = panel->inner_rect.pos.y;
    if (panel->widgets_rect.size.h <= panel->inner_rect.size.h) {
        //content fits inside the panel, center it vertically
        cursor_pos.y += (panel->inner_rect.size.h - panel->widgets_rect.size.h) / 2;
    } else {
        //content overflows, apply scroll offset from the top
        cursor_pos.y -= scroll_offset;
    }

    bool inline_row = false;
    int inline_row_width = 0;
    // int inline_row_total = 0;
    // int inline_row_index = 0;

    for (int i = 0; i < panel->widget_count; i++){
        Widget *widget = &panel->widgets[i];

        //if we have encountered a new inline widget,
        //we collect the width of all subsequent inline widgets
        if(widget->is_inline && !inline_row){
            inline_row = true;
            // inline_row_index = 0;
            for(int j = i; j < panel->widget_count; j++){
                Widget *next_widget = &panel->widgets[j];
                if(!next_widget->is_inline) break;
                // inline_row_total++;
                inline_row_width += next_widget->size.x;
            }

            //centrar row horizontally
            auto centered_row = center_in_container(
                cursor_pos.x,
                inline_row_width,
                panel->inner_rect.size.w
            );
            cursor_pos.x = centered_row;
        }

        //not an inline widget, reset the row
        if(!widget->is_inline){
            inline_row = false;
            inline_row_width = 0;
            // inline_row_total = 0;
            // inline_row_index = 0;
            //immediately move cursor down

            // center next widget horizontally
            cursor_pos.x = center_in_container(
                cursor_pos.x,
                widget->size.w,
                panel->inner_rect.size.w
            );
        }

        //render current widget INSIDE panel boundaires
        if(cursor_pos.y >= panel->inner_rect.pos.y
        && cursor_pos.y < panel->inner_rect.pos.y + panel->inner_rect.size.h){
            tui_render_widget(widget, cursor_pos);
        }

        if (i >= panel->widget_count - 1) break; //no more panels, break early

        //hay siguiente?
        Widget *next_widget = (i < panel->widget_count - 2)
            ? &panel->widgets[i + 1]
            : nullptr;

        //determine cursor movement
        if(widget->is_inline
        && next_widget != nullptr
        && next_widget->is_inline){
            cursor_pos.x += widget->size.x;
        }else{
            //move cursror below the widget we just rendered
            cursor_pos.x = BASE_X;
            cursor_pos.y += widget->size.y;
        }

    }

    //draw panel border
    if(panel->focused){
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
    }
    tui_draw_box(LAYOUT_STATE.screen, panel->outer_rect);

    //panel scroll
    if(!panel->focused) return;
    constexpr int scrollbar_padding = 2;
    auto from = (vec2i){
        .x = panel->outer_rect.pos.x + panel->outer_rect.size.x -1,
        .y = panel->outer_rect.pos.y + scrollbar_padding
    };
    auto to = (vec2i){
        .x = from.x,
        .y = panel->outer_rect.pos.y + panel->outer_rect.size.y - 1 - scrollbar_padding
    };
    int total_size = panel->widgets_rect.size.h;
    int shown_from = scroll_offset;
    int shown_to   = scroll_offset + panel->inner_rect.size.h;
    if(shown_to - shown_from < total_size){ //dont show scrollbar if can't scroll
        tui_draw_scrollbar_vertical(LAYOUT_STATE.screen, from, to, total_size, shown_from, shown_to);
    }
}

private Panel *tui_get_panel_focused(){
    return &(LAYOUT_STATE.panels[LAYOUT_STATE.panel_focused]);
}

private Widget *tui_get_widget_focused(){
    auto widget_focused_id = LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused];
    if(widget_focused_id == nullptr){
        return nullptr;
    }
    auto panel = tui_get_panel_focused();
    for(int index = 0; index < panel->widget_count; index++){
        auto widget = panel->widgets[index];
        if(strcmp(widget_focused_id, widget.id) == 0){
            return &panel->widgets[index];
        }
    }
    //ERROR: LAYOUT_STATE.widget_focused mismatch with actual widgets
    assert(false); //que le hicieron a mi muchacho ?
    return nullptr;
}

private rect2i tui_panel_rect(PanelSlot slot){
    //panel size is based on the slot it occupies in the type of layout

    if(slot == SLOT_OVERLAY || slot == SLOT_WIDGETS_OVERLAY_DO_NOT_USE){
        //overlay slot is the same for all layouts, starts at max size,
        //and will shrink before rendering up to the widgets boundaries
        return (rect2i){ .size = LAYOUT_STATE.base_size };
    }

    int base_w    = LAYOUT_STATE.base_size.w;
    int base_h    = LAYOUT_STATE.base_size.h;
    int sidebar_w = max(30, 0.4 * base_w);
    int header_h  = 6;
    int footer_h  = 6;

    switch(LAYOUT_STATE.layout){
    case LAYOUT_SINGLE_PANEL: return (rect2i){.size = {w, h}};
    case LAYOUT_SIDEBAR_LEFT:
        switch slot:
        case SLOT_SIDEBAR: return (rect2i){.size = {sidebar_w, h}};
        case SLOT_MAIN:    return (rect2i){.pos  = {sidebar_w, 0}, .size = {w - sidebar_w, h}};
        default: assert(false); //layout doesnt support this slot
    case LAYOUT_SIDEBAR_RIGHT:
        switch slot:
        case SLOT_MAIN:    return (rect2i){.size = {w - sidebar_w, h}};
        case SLOT_SIDEBAR: return (rect2i){.pos  = {w - sidebar_w, 0}, .size = {sidebar_w, h}};
        default: assert(false); //layout doesnt support this slot
    case LAYOUT_SPLIT_VERTICAL:
        switch slot:
        case SLOT_LEFT:    return (rect2i){.size = {w / 2, h}};
        case SLOT_RIGHT:   return (rect2i){.pos = {w / 2, 0}, .size = {w - w / 2, h}};
        default: assert(false); //layout doesnt support this slot
    case LAYOUT_WITH_HEADER:
        switch slot:
        case SLOT_TOP:    return (rect2i){.size = {w, header_h}};
        case SLOT_MAIN:   return (rect2i){.pos = {0, header_h}, .size = {w, h - header_h}};
        default: assert(false); //layout doesnt support this slot
    case LAYOUT_WITH_FOOTER:
        switch slot:
        case SLOT_MAIN:   return (rect2i){.size = {w, h - footer_h}};
        case SLOT_BOTTOM: return (rect2i){.pos = {0, h - footer_h}, .size = {w, footer_h}};
        default: assert(false); //layout doesnt support this slot
    case LAYOUT_WITH_HEADER_AND_FOOTER:
        switch slot:
        case SLOT_TOP:    return (rect2i){.size = {w, header_h}};
        case SLOT_MAIN:   return (rect2i){.pos = {0, header_h}, .size = {w, h - header_h - footer_h}};
        case SLOT_BOTTOM: return (rect2i){.pos = {0, h - footer_h}, .size = {w, footer_h}};
        default: assert(false); //layout doesnt support this slot
    case LAYOUT_SPLIT_VERTICAL_WITH_HEADER:
        switch slot:
        case SLOT_TOP:    return (rect2i){.size = {w, header_h}};
        case SLOT_LEFT:   return (rect2i){.pos = {0, header_h}, .size = {w / 2, h - header_h}};
        case SLOT_RIGHT:  return (rect2i){.pos = {w / 2, header_h}, .size = {w - w / 2, h - header_h}};
        default: assert(false); //layout doesnt support this slot
    }

    assert(false); //ERROR: layout not implemented!
    return r;
}

private void tui_panel_shrink_to_widgets(Panel *panel){
    //achicar panel al tamaño de los widgets
    panel->inner_rect = panel->widgets_rect;
    panel->outer_rect = panel->widgets_rect;

    //agrandar
    panel->outer_rect.size.w += PADDING * 2 + BORDER * 2;
    panel->outer_rect.size.h += PADDING * 2 + BORDER * 2;

    panel->outer_rect.size.w = max(32, panel->outer_rect.size.w);
    panel->outer_rect.size.h = max(8,  panel->outer_rect.size.h);

    //centrar
    panel->outer_rect.pos.x = center_in_container(
        panel->outer_rect.pos.x,
        panel->outer_rect.size.w,
        LAYOUT_STATE.base_size.w
    );
    panel->outer_rect.pos.y = center_in_container(
        panel->outer_rect.pos.y,
        panel->outer_rect.size.h,
        LAYOUT_STATE.base_size.h
    );

    //compensar
    panel->inner_rect.pos.x  = panel->outer_rect.pos.x  + BORDER + PADDING;
    panel->inner_rect.pos.y  = panel->outer_rect.pos.y  + BORDER + PADDING;
    panel->inner_rect.size.w = panel->outer_rect.size.w - BORDER * 2 - PADDING * 2;
    panel->inner_rect.size.h = panel->outer_rect.size.h - BORDER * 2 - PADDING * 2;
}

private void tui_render_overlay_panel(Panel *overlay_panel, int scroll_offset){
    //in case there was still an open inline row from the definition pass
    tui_widget_row_end(overlay_panel);

    //shrink and render
    tui_panel_shrink_to_widgets(overlay_panel);
    tui_draw_rect(LAYOUT_STATE.screen, u8" ", overlay_panel->outer_rect);
    tui_render_panel(overlay_panel, scroll_offset);
}

private Panel *tui_widget_overlay_panel_create(void){
    Widget *widget = tui_get_widget_focused();
    if(widget == nullptr) return nullptr;
    if(widget->build_overlay_panel == nullptr) return nullptr;

    int panel_focused_prev = LAYOUT_STATE.panel_focused;
    const char *widget_overlay_owner_widget = widget->id;

    tui_panel_begin(SLOT_WIDGETS_OVERLAY_DO_NOT_USE);
    widget->build_overlay_panel(widget);
    tui_panel_end();

    Panel *overlay_panel = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_count - 1];
    if(overlay_panel->widget_count == 0){
        LAYOUT_STATE.panel_count--;
        return nullptr;
    }

    LAYOUT_STATE.widget_overlay_focused_prev = panel_focused_prev;
    LAYOUT_STATE.widget_overlay_owner_panel = panel_focused_prev;
    LAYOUT_STATE.widget_overlay_owner_widget = widget_overlay_owner_widget;
    LAYOUT_STATE.panel_focused = LAYOUT_STATE.panel_count - 1;

    return overlay_panel;
}

void tui_panel_begin(PanelSlot slot){
    assert(LAYOUT_STATE.panel_curr == -1); //close the prev panel first!
    assert(LAYOUT_STATE.panel_count < TUI_PANELS_MAX);

    auto panel_rect = tui_panel_rect(slot);
    panel_rect.pos.y += 1: //leave space for the app title
    Panel new_panel = {
        .slot = slot,
        .outer_rect = panel_rect,
        .inner_rect = (rect2i){
            .pos  = (vec2i){
                .x = panel_rect.pos.x + PADDING + BORDER,
                .y = panel_rect.pos.y + PADDING + BORDER,
            },
            .size = (vec2i){
                .w = panel_rect.size.w - PADDING * 2 - BORDER * 2,
                .h = panel_rect.size.h - PADDING * 2 - BORDER * 2,
            },
        },
    };
    LAYOUT_STATE.panel_curr = LAYOUT_STATE.panel_count;
    LAYOUT_STATE.panels[LAYOUT_STATE.panel_count++] = new_panel;
}

void tui_panel_end(void){
    assert(LAYOUT_STATE.panel_curr != -1); //no panel to close
    LAYOUT_STATE.panel_curr = -1;
}

void tui_panel_scroll(int offset){
    Panel *panel      = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_focused];
    int scroll_offset = LAYOUT_STATE.panel_scroll_offset[LAYOUT_STATE.panel_focused];

    LAYOUT_STATE.panel_scroll_offset[LAYOUT_STATE.panel_focused] = clamp(
        scroll_offset + offset,
        0,
        max(0, panel->widgets_rect.size.h - panel->inner_rect.size.h)
    );
}

void tui_panel_scroll_to(int widget_index); //TODO: widget index or widget pointer?

void tui_panel_scroll_up(void){
    tui_panel_scroll(-1);
}

void tui_panel_scroll_down(void){
    tui_panel_scroll(+1);
}

char *tui_create_widget_id(){
    char *new_id = (char *)arena_alloc(LAYOUT_STATE.arena_frame, sizeof(char) * 16);
    sprintf(new_id, "auto_id_%d", LAYOUT_STATE.widget_auto_id++);
    return new_id;
}

private inline Widget *get_latest_widget(){
    Panel *panel = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_curr];
    if(panel->widget_count == 0) return nullptr;
    return &panel->widgets[panel->widget_count - 1];
}

private inline Widget *get_new_widget(char const *widget_id){
    assert(LAYOUT_STATE.panel_curr != -1); //must be used inside a tui_panel_begin!
    Panel *panel = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_curr];
    assert(panel->widget_count < TUI_WIDGETS_IN_PANEL_MAX); //too many widgets!

    Widget *new_widget = &panel->widgets[panel->widget_count++];
    new_widget->id = widget_id;

    return new_widget;
}

void *tui_widget_state(const char *widget_id, size_t data_size){
    assert(widget_id != NULL);
    assert(data_size > 0);

    //find the state for the ID and return if found
    for(size_t i = 0; i < WIDGET_REGISTRY.states_count; i++){
        WidgetState *state = &WIDGET_REGISTRY.states[i];
        if(strcmp(widget_id, state->widget_id) == 0){
            //reallocate if bigger than expected
            if(data_size >= state->state_size){
                state->state_data = (void *)arena_realloc(
                    WIDGET_REGISTRY.arena,
                    state->state_data,
                    state->state_size,
                    data_size
                );
                state->state_size = data_size;
            }
            return state->state_data;
        }
    }

    //create new state for the id
    assert(WIDGET_REGISTRY.states_count < TUI_WIDGET_STATES_MAX);
    WidgetState *state = &WIDGET_REGISTRY.states[WIDGET_REGISTRY.states_count++];
    state->widget_id  = widget_id;
    state->state_size = data_size;

    state->state_data = (void *)arena_alloc(WIDGET_REGISTRY.arena, data_size);
    return state->state_data;
}

private void tui_widget_row_begin(Panel *panel){
    //reset row
    panel->curr_row_size = (vec2i){.w = 0, .h = 0};
}

private void tui_widget_row_push(Panel *panel, Widget *widget){
    //pushes widget to row
    //should NOT modify the widgets_rect !

    //HEIGHT: biggest widget height remains
    panel->curr_row_size.h += widget->size.h;

    //WIDTH:  widget width is always added to row width
    if(widget->size.w > panel->curr_row_size.w){
        panel->curr_row_size.w = widget->size.w;
    }
}

private void tui_widget_row_end(Panel *panel){
    //push row if exists to widget rect
    if(panel->curr_row_size.w == 0 && panel->curr_row_size.h == 0){
        //nothing to commit
        return;
    }
    //HEIGHT: row height is always added to panel height
    panel->widgets_rect.size.h += panel->curr_row_size.h;
    //WIDTH:  biggest row width remains
    if(panel->curr_row_size.w > panel->widgets_rect.size.w){
        panel->widgets_rect.size.w = panel->curr_row_size.w;
    }
    //reset row size
    panel->curr_row_size = (vec2i){.w = 0, .h = 0};
}

void tui_widget_push(Widget widget){
    assert(LAYOUT_STATE.panel_curr != -1); //must be used inside a tui_panel_begin!
    Widget *last_widget = get_latest_widget();
    Widget *new_widget  = get_new_widget(widget.id);
    memcpy(new_widget, &widget, sizeof(Widget));

    //widget focus
    auto panel_focused_widget = LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_curr];
    new_widget->focused = panel_focused_widget != NULL && strcmp(new_widget->id, panel_focused_widget) == 0;

    //panel row, increases widget rect in panel
    Panel *panel = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_curr];
    bool inside_row = (last_widget == nullptr || last_widget->is_inline);
    if(new_widget->is_inline && !inside_row){
        //comienza nueva row inline
        tui_widget_row_begin(panel);
        tui_widget_row_push(panel, new_widget);
    }else if(new_widget->is_inline && inside_row){
        //ya estaba en una row inline y la continuo
        tui_widget_row_push(panel, new_widget);
    }else{ //widget not inline
        tui_widget_row_end(panel); //ten case de que haya habido una row abierta
        //widgets not inline have their own row
        tui_widget_row_begin(panel);
        tui_widget_row_push(panel, new_widget);
        tui_widget_row_end(panel);
    }
}

void tui_layout_prepare(Screen *screen, PageLayout layout){

    if(LAYOUT_STATE.arena_frame == nullptr){
        LAYOUT_STATE.arena_frame = arena_init(1024 * 1024 * 5); //5mb
    }

    if(WIDGET_REGISTRY.arena == nullptr){
        WIDGET_REGISTRY.arena = arena_init(1024 * 1024 * 5); //5mb
    }

    //if layout changes we reset the widget registry
    if(layout != LAYOUT_STATE.layout){
        LAYOUT_STATE.layout = layout;
        arena_reset(WIDGET_REGISTRY.arena);
        WIDGET_REGISTRY.states_count = 0;
    }

    //clear panels and widgets
    if(LAYOUT_STATE.widget_overlay_focused_prev != -1){
        LAYOUT_STATE.panel_focused = LAYOUT_STATE.widget_overlay_focused_prev;
        LAYOUT_STATE.widget_overlay_focused_prev = -1;
        LAYOUT_STATE.widget_overlay_owner_panel = -1;
        LAYOUT_STATE.widget_overlay_owner_widget = nullptr;
    }

    LAYOUT_STATE.panel_curr     = -1;
    LAYOUT_STATE.panel_count    = 0;
    LAYOUT_STATE.widget_auto_id = 0;

    //NOTE: base size has h-2 to leave room for header at top
    //      and key hints at bottom
    LAYOUT_STATE.base_size.w = screen->size.w;
    LAYOUT_STATE.base_size.h = screen->size.h - 2;

    assert(screen != NULL); //apagaste el monitor capo??
    LAYOUT_STATE.screen = screen;
}

void tui_layout_prepare_widget_overlay(void){
    tui_widget_overlay_panel_create();
}

private bool tui_widget_overlay_close_input(InputEvent input_event){
    if(input_event.input_type != INPUT_KEY) return false;
    if(input_event.key_event.key != KEY_ESCAPE) return false;
    if(LAYOUT_STATE.panel_focused >= LAYOUT_STATE.panel_count) return false;

    Panel *panel_focused = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_focused];
    if(panel_focused->slot != SLOT_WIDGETS_OVERLAY_DO_NOT_USE) return false;
    if(LAYOUT_STATE.widget_overlay_owner_panel < 0) return true;
    if(LAYOUT_STATE.widget_overlay_owner_panel >= LAYOUT_STATE.panel_count) return true;
    if(LAYOUT_STATE.widget_overlay_owner_widget == nullptr) return true;

    int owner_panel_index = LAYOUT_STATE.widget_overlay_owner_panel;
    Panel *owner_panel = &LAYOUT_STATE.panels[owner_panel_index];
    for(int i = 0; i < owner_panel->widget_count; i++){
        Widget *owner_widget = &owner_panel->widgets[i];
        if(strcmp(owner_widget->id, LAYOUT_STATE.widget_overlay_owner_widget) != 0) continue;
        if(owner_widget->input != nullptr){
            owner_widget->input(owner_widget, input_event);
        }
        break;
    }

    if(LAYOUT_STATE.panel_focused == LAYOUT_STATE.panel_count - 1){
        LAYOUT_STATE.panel_count--;
    }
    LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused] = 0;
    LAYOUT_STATE.panel_focused = owner_panel_index;
    LAYOUT_STATE.widget_overlay_focused_prev = -1;
    LAYOUT_STATE.widget_overlay_owner_panel = -1;
    LAYOUT_STATE.widget_overlay_owner_widget = nullptr;
    return true;
}

bool tui_widget_focused_input(InputEvent input_event){
    if(tui_widget_overlay_close_input(input_event)) return true;

    Widget *widget = tui_get_widget_focused();
    if(widget == nullptr) return false;
    if(widget->input == nullptr) return false;
    return widget->input(widget, input_event);
}

void tui_layout_render(){
    //if we find the overlay panel defined, we store it for later
    Panel *overlay_panel = nullptr;
    int overlay_panel_index = 0;

    for (int i = 0; i < LAYOUT_STATE.panel_count; i++){
        Panel *panel = &LAYOUT_STATE.panels[i];
        if(panel->slot == SLOT_OVERLAY){
            overlay_panel_index = i;
            overlay_panel = panel;
            break;
        }
    }

    if(overlay_panel){
        //si hay overlay, siempre es el focused
        if(LAYOUT_STATE.panel_focused != overlay_panel_index){
            //guardamos el anterior focused anterior para poder volver atras
            LAYOUT_STATE.panel_focused_prev = LAYOUT_STATE.panel_focused;
            LAYOUT_STATE.panel_focused      = overlay_panel_index;
        }
    }else if(LAYOUT_STATE.panel_focused_prev != -1){
        //if overlay wont show but it was previously focused,
        //we restore the focused panel to the one before showing the overlay
        LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused] = 0;
        LAYOUT_STATE.panel_focused = LAYOUT_STATE.panel_focused_prev;
        LAYOUT_STATE.panel_focused_prev = -1;
    }

    Panel *widget_overlay_panel = nullptr;
    int widget_overlay_panel_index = 0;

    for (int i = 0; i < LAYOUT_STATE.panel_count; i++){
        Panel *panel = &LAYOUT_STATE.panels[i];
        if(panel->slot == SLOT_WIDGETS_OVERLAY_DO_NOT_USE){
            widget_overlay_panel_index = i;
            widget_overlay_panel = panel;
            break;
        }
    }

    if(widget_overlay_panel == nullptr){
        widget_overlay_panel = tui_widget_overlay_panel_create();
        widget_overlay_panel_index = LAYOUT_STATE.panel_count - 1;
    }

    //render panels
    for (int i = 0; i < LAYOUT_STATE.panel_count; i++){
        Panel *panel = &LAYOUT_STATE.panels[i];
        panel->focused = (LAYOUT_STATE.panel_focused == i);

        if(panel->slot == SLOT_OVERLAY
        || panel->slot == SLOT_WIDGETS_OVERLAY_DO_NOT_USE){
            continue;
        }
        int scroll_offset = LAYOUT_STATE.panel_scroll_offset[i];
        tui_render_panel(panel, scroll_offset);
    }

    //render OVERLAY PANEL
    if(overlay_panel && overlay_panel->widget_count > 0){
        overlay_panel->focused = true;
        tui_render_overlay_panel(overlay_panel, LAYOUT_STATE.panel_scroll_offset[overlay_panel_index]);
    }

    if(widget_overlay_panel != nullptr){
        widget_overlay_panel->focused = true;
        tui_render_overlay_panel(widget_overlay_panel, LAYOUT_STATE.panel_scroll_offset[widget_overlay_panel_index]);
    }

    //reset arena
    arena_reset(LAYOUT_STATE.arena_frame);
    //reset the layout state
    LAYOUT_STATE.panel_curr = -1;
    LAYOUT_STATE.panel_count = 0;
}

//focus navigation

void tui_cursor_next_widget(void){
    Panel *panel = tui_get_panel_focused();
    if(panel->widget_count == 0) return;

    int i = -1; //default focused widget

    // find currently focused widget index
    if(LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused] != NULL){
        for(i = 0; i < panel->widget_count; i++){
            auto widget = panel->widgets[i];
            if(strcmp(
                LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused],
                widget.id) == 0
            ){
                break;
            }
        }
    }

    //find the next focusable widget, wrapping around
    for(int j = 1; j <= panel->widget_count; j++){
        auto widget = panel->widgets[(i + j) % panel->widget_count];
        if(widget.focusable){
            LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused] = widget.id;
            break;
        }
    }
}

void tui_cursor_prev_widget(void){
    Panel *panel = tui_get_panel_focused();
    if(panel->widget_count == 0) return;

    int i = panel->widget_count; //default focused widget

    // find currently focused widget index
    if(LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused] != NULL){
        for(i = 0; i < panel->widget_count; i++){
            auto widget = panel->widgets[i];
            if(strcmp(
                LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused],
                widget.id) == 0
            ){
                break;
            }
        }
    }

    //find the next focusable widget, wrapping around
    for(int j = 0; j < panel->widget_count; j++){
        int widget_index = (i - j + panel->widget_count - 1) % panel->widget_count;
        auto widget = panel->widgets[widget_index];
        if(widget.focusable){
            LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_focused] = widget.id;
            break;
        }
    }
}

void tui_cursor_next_panel(void){
    if(LAYOUT_STATE.panel_count <= 1) return;

    int start = LAYOUT_STATE.panel_focused;
    int next  = start;

    //because there are two special types of panels we have to do this to avoid them
    for(int i = 0; i < LAYOUT_STATE.panel_count; i++){
        next = (next + 1) % LAYOUT_STATE.panel_count;
        if(LAYOUT_STATE.panels[next].slot != SLOT_OVERLAY &&
           LAYOUT_STATE.panels[next].slot != SLOT_WIDGETS_OVERLAY_DO_NOT_USE) {
            LAYOUT_STATE.panel_focused = (uint8_t)next;
            return;
        }
    }
}

void tui_cursor_prev_panel(void){
    if(LAYOUT_STATE.panel_count <= 1) return;

    int start = LAYOUT_STATE.panel_focused;
    int prev  = start;

    //because there are two special types of panels we have to do this to avoid them
    for(int i = 0; i < LAYOUT_STATE.panel_count; i++){
        prev = (prev - 1 + LAYOUT_STATE.panel_count) % LAYOUT_STATE.panel_count;
        if(LAYOUT_STATE.panels[prev].slot != SLOT_OVERLAY &&
           LAYOUT_STATE.panels[prev].slot != SLOT_WIDGETS_OVERLAY_DO_NOT_USE) {
            LAYOUT_STATE.panel_focused = (uint8_t)prev;
            return;
        }
    }
}

#endif //TUI_LAYOUT_IMPL
#endif //TUI_LAYOUT

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
    // LAYOUT_SIDEBAR_LEFT_NARROW,
    // LAYOUT_SIDEBAR_LEFT_WIDE,
    LAYOUT_SIDEBAR_RIGHT,
    LAYOUT_SPLIT_VERTICAL,
    LAYOUT_SIDEBAR_LEFT_SPLIT_RIGHT,
    //TODO: add more..?
} PageLayout;

typedef enum {
    SLOT_MAIN,
    SLOT_SIDEBAR,
    SLOT_TOP,
    SLOT_BOTTOM,
} PanelSlot;

typedef struct Widget Widget;
typedef bool (*WidgetInputFunction  )(Widget *widget, InputEvent input_event);
typedef void (*WidgetRenderFunction )(Widget *widget, Screen *screen, vec2i position);

struct Widget {
    const char           *id;
    vec2i                  size;
    void                 *data;
    void                 *state; //data that survives frames
    bool                  focusable;
    bool                  focused;
    bool                  is_inline;
    WidgetInputFunction   input;
    WidgetRenderFunction  render;
};

private constexpr int TUI_WIDGETS_IN_PANEL_MAX = 32;
typedef struct {
    PanelSlot slot;
    bool      focused;
    Widget    widgets[TUI_WIDGETS_IN_PANEL_MAX];
    uint8_t   widget_count;
    rect2i    outer_rect;    //cached
    rect2i    widgets_rect;  //total accumulated rect around the widgets
} Panel;

//api to defin the panels and widgets on the page
void tui_panel_begin(PanelSlot slot);
void tui_panel_end(void);

//widgets
char *tui_create_widget_id();
void  tui_widget_push(Widget widget);
void *tui_widget_state(const char *widget_id, size_t data_size);

//used in the actual rendering process by tui.h
void tui_layout_prepare(Screen *screen, PageLayout layout);
bool tui_widget_focused_input(InputEvent input_event);
void tui_layout_render(void); //actually rendering to the screen

//focus navigation
void tui_cursor_next_widget(void);
void tui_cursor_prev_widget(void);
void tui_cursor_next_panel(void);
void tui_cursor_prev_panel(void);

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
    vec2i                 base_size;
    Panel                panels[TUI_PANELS_MAX];
    uint8_t              panel_count;
    int8_t               panel_curr; // -1 = no panel selected
    uint8_t              panel_focused; //at least one panel always focused
    uint8_t              widget_auto_id;
    const char          *widget_focused[TUI_PANELS_MAX]; //one id per panel
    PageLayout           layout;
    Screen              *screen;
    Arena               *arena_frame;
} LayoutState;

private LayoutState LAYOUT_STATE = {
    .panel_curr  = -1,
    .arena_frame = nullptr,
};

private inline int center_in_container(int base, int length, int container_length){
    base += container_length / 2 - length / 2;
    //cant do -1 on h above because it's an int and it doesnt accumulate the 0.5 after division
    //we do it twice separatedly to avoid reaching negativess
    if(base > 1) base--;
    if(base > 1) base--;
    return base;
}

private void tui_render_widget(Panel *panel, Widget *widget, vec2i position){
    //center widget horizontally
    position.x = center_in_container(
        position.x,
        widget->size.w,
        panel->outer_rect.size.w - PADDING - BORDER
    );
    widget->render(widget, LAYOUT_STATE.screen, position);
    //always reset color after a widget!
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
}

private void tui_render_panel(Panel *panel){
    vec2i cursor_pos = {
        .x = panel->outer_rect.pos.x + BORDER + PADDING,
        .y = panel->outer_rect.pos.y + BORDER + PADDING
    };

    //panels always render their content centered vertically
    //widget heights are precomputed
    cursor_pos.y = center_in_container(
        cursor_pos.y,
        panel->widgets_rect.size.h,
        panel->outer_rect.size.h
    );

    for (int i = 0; i < panel->widget_count; i++){
        //NOTE: we dont use clamp on the cursor because we might have scrollable content here
        if(cursor_pos.y < 0
        || cursor_pos.y > panel->outer_rect.pos.y + panel->outer_rect.size.h - 1){
            break;
        }
        Widget *widget = &panel->widgets[i];
        tui_render_widget(panel, widget, cursor_pos);

        //move below the prev widget
        cursor_pos.y += widget->size.y + PADDING;
    }

    //draw panel border, connected to other panels
    //TODO: set focused
    //LAYOUT_STATE.panel_focused == ??
    //TODO: possible small optimization, same border to the app border if only single panel
    tui_draw_box_connected(LAYOUT_STATE.screen, panel->outer_rect);
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
    //panel size is based on the slot it occuppies in the type of layout

    switch(LAYOUT_STATE.layout){
    case LAYOUT_SINGLE_PANEL:
        switch(slot){
        case SLOT_MAIN: return (rect2i){ .size = LAYOUT_STATE.base_size };
        default: assert(false); //using an invalid slot for this layout
        }
    case LAYOUT_SPLIT_VERTICAL:
    default: assert(false); //TODO: layout not implemented yet
    //TODO: add more panel layout definitions
    }
}

void tui_panel_begin(PanelSlot slot){
    assert(LAYOUT_STATE.panel_curr == -1); //close the prev panel first!
    assert(LAYOUT_STATE.panel_count < TUI_PANELS_MAX);

    auto panel_rect = tui_panel_rect(slot);
    Panel new_panel = {
        .slot = slot,
        .outer_rect = panel_rect,
    };
    LAYOUT_STATE.panel_curr = LAYOUT_STATE.panel_count;
    LAYOUT_STATE.panels[LAYOUT_STATE.panel_count++] = new_panel;
}

void tui_panel_end(void){
    assert(LAYOUT_STATE.panel_curr != -1); //no panel to close
    LAYOUT_STATE.panel_curr = -1;
}

char *tui_create_widget_id(){
    char *new_id = (char *)arena_alloc(LAYOUT_STATE.arena_frame, sizeof(char) * 16);
    sprintf(new_id, "auto_id_%d", LAYOUT_STATE.widget_auto_id++);
    return new_id;
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

void tui_widget_push(Widget widget){
    assert(LAYOUT_STATE.panel_curr != -1); //must be used inside a tui_panel_begin!
    Widget *new_widget = get_new_widget(widget.id);
    memcpy(new_widget, &widget, sizeof(Widget));

    //widget focus
    auto panel_focused_widget = LAYOUT_STATE.widget_focused[LAYOUT_STATE.panel_curr];
    new_widget->focused = panel_focused_widget != NULL && strcmp(new_widget->id, panel_focused_widget) == 0;

    //increase widget rect in panel
    Panel *panel = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_curr];
    panel->widgets_rect.size.h += new_widget->size.h;
    if(new_widget->size.w > panel->widgets_rect.size.w){
        panel->widgets_rect.size.w = new_widget->size.w;
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
    LAYOUT_STATE.panel_curr     = -1;
    LAYOUT_STATE.panel_count    = 0;
    LAYOUT_STATE.widget_auto_id = 0;

    //NOTE: base size has h-1 to leave room for the key hints
    LAYOUT_STATE.base_size.w = screen->size.w;
    LAYOUT_STATE.base_size.h = screen->size.h - 1;

    assert(screen != NULL); //apagaste el monitor capo
    LAYOUT_STATE.screen = screen;
}

bool tui_widget_focused_input(InputEvent input_event){
    Widget *widget = tui_get_widget_focused();
    if(widget == nullptr) return false;
    if(widget->input == nullptr) return false;
    return widget->input(widget, input_event);
}

void tui_layout_render(){
    //box surrounding app screen first
    tui_draw_box_connected(
        LAYOUT_STATE.screen,
        (rect2i){ .size = LAYOUT_STATE.base_size }
    );
    //render panels
    for (int i = 0; i < LAYOUT_STATE.panel_count; i++){
        tui_render_panel(&LAYOUT_STATE.panels[i]);
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
    //TODO: order of panels should be determined by
    //      the layout!
    // panel_focused move next...
    // TODO: need to ensure there never is a
    //       state where no panel is focused
}
void tui_cursor_prev_panel(void){

}

#endif //TUI_LAYOUT_IMPL
#endif //TUI_LAYOUT

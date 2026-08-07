#ifndef TUI_LAYOUT
#define TUI_LAYOUT

#include <assert.h>
#include <string.h>

#include "tui_utils.h"
#include "tui_screen.h"
#include "tui_platform.h"
#include "tui_arena.h"
#include "tui_draw.h"

typedef enum {
    LAYER_BASE = 0,
    LAYER_OVERLAY,
    LAYER_WIDGETS_OVERLAY_DO_NOT_USE,
    LAYER_COUNT,
} PageLayerKind;

typedef enum {
    LAYOUT_SINGLE_PANEL,
    LAYOUT_SIDEBAR_LEFT,
    LAYOUT_SIDEBAR_RIGHT,
    LAYOUT_SPLIT_VERTICAL,
    LAYOUT_WITH_HEADER,
    LAYOUT_WITH_FOOTER,
    LAYOUT_WITH_HEADER_AND_FOOTER,
    LAYOUT_SPLIT_VERTICAL_WITH_HEADER,
} PageLayout;

typedef enum {
    SLOT_MAIN,
    SLOT_SIDEBAR,
    SLOT_TOP,
    SLOT_BOTTOM,
    SLOT_LEFT,
    SLOT_RIGHT,
} PanelSlot;

//forward declares
typedef struct Widget Widget;
typedef struct PageLayer PageLayer;

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
    WidgetOverlayFunction overlay;
    const uint8_t        *overlay_title;
};

static constexpr int TUI_WIDGETS_IN_PANEL_MAX = 64;
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

//TODO: maybe this could go somewhere else
typedef struct {
    const uint8_t *text;
    ColorPair colors;
} AnimationFrame;

//API TO DEFINE THE PANELS AND WIDGETS ON THE PAGE
void tui_panel_begin(PanelSlot slot);
void tui_panel_end(void);

//NOTE: setting the layer is NOT required, framework assumes base layer by default.
void tui_layer_begin(PageLayerKind layer, PageLayout layout);
void tui_layer_end(void);

//widgets, public in case you define custom widgets
char *tui_create_widget_id();
void  tui_widget_push(Widget widget);
void *tui_widget_state(const char *widget_id, size_t data_size);

//used in the actual rendering process by tui.h
static void _tui_layout_prepare(Screen *screen, PageLayout layout);
static bool _tui_widget_focused_input(InputEvent input_event);
static void _tui_layout_build_widget_overlays(void);
static void _tui_layout_render(void); //actually rendering to the screen
static void _tui_layout_reset(void);

//focus/navigation
static void   _tui_cursor_next_widget(void);
static void   _tui_cursor_prev_widget(void);
static void   _tui_cursor_next_panel(void);
static void   _tui_cursor_prev_panel(void);
static Panel* _tui_get_panel_building(void); //used by widgets
static void   _tui_panel_scroll(int offset);
static void   _tui_panel_scroll_to(int widget_index); //TODO: widget index or widget pointer?
static void   _tui_panel_scroll_up(void);
static void   _tui_panel_scroll_down(void);

static PageLayer* _tui_get_layer_focused(void);
static bool       _tui_is_panel_scrollable(Panel *panel);
static Panel*     _tui_get_panel_focused(void);
static Panel*     _tui_get_panel_building(void);

//overlay
static void _tui_widget_overlay_open(void);
static void _tui_widget_overlay_close(void);
static bool _tui_widget_overlay_is_open(void);

#ifdef TUI_LAYOUT_IMPL

static constexpr int PADDING = 1;
static constexpr int BORDER = 1;
static constexpr int TUI_PANELS_MAX = 12;
static constexpr int TUI_WIDGET_STATES_MAX = TUI_PANELS_MAX * TUI_WIDGETS_IN_PANEL_MAX;

struct PageLayer {
    PageLayout   layout;
    bool         shrink;
    bool         focused;
    Panel        panels[TUI_PANELS_MAX];
    uint8_t      panel_count;
    int8_t       panel_building; // -1 = no panel selected
    uint8_t      panel_focused; //at least one panel always focused
    int          panel_scroll_offset[TUI_PANELS_MAX]; //one per panel
    const char  *widget_focused[TUI_PANELS_MAX]; //one id per panel
    uint8_t      widget_auto_id; //IDs are global to the layer,
                                 //in case the user wants to move widgest around
    rect2i       rect;          //bounding box of the layer, set after layout
};

typedef struct {
    const char *widget_id;
    void       *state_data;
    size_t      state_size;
} WidgetState;

typedef struct {
    WidgetState states[TUI_WIDGET_STATES_MAX]; //survives through frames!
    size_t      states_count;
    Arena       *arena;
} _WidgetStateRegistry;

static _WidgetStateRegistry WIDGET_REGISTRY = {.arena  = nullptr};

typedef struct {
    vec2i           base_size;
    PageLayer       layers[LAYER_COUNT];
    PageLayerKind   layer_building;
    PageLayerKind   layer_focused;
    Screen         *screen;
    Arena          *arena_frame;
    bool            widget_overlay_active; //because the widgets are encapsulated,
                                           // we have to manage their overlay state from here.
    const uint8_t  *widget_overlay_title; //temporary, set during overlay build for title rendering
} _LayoutState;

static _LayoutState LAYOUT_STATE = {
    .layers = {
        [LAYER_BASE]                       = {.panel_building = -1, .shrink = false},
        [LAYER_OVERLAY]                    = {.panel_building = -1, .shrink = true},
        [LAYER_WIDGETS_OVERLAY_DO_NOT_USE] = {.panel_building = -1, .shrink = true},
    },
    .arena_frame = nullptr,
    .widget_overlay_active = false,
};

static void _tui_widget_overlay_open(void){
    LAYOUT_STATE.widget_overlay_active = true;
}

static void _tui_widget_overlay_close(void){
    LAYOUT_STATE.widget_overlay_active = false;
}

static bool _tui_widget_overlay_is_open(void){
    return LAYOUT_STATE.widget_overlay_active;
}

static void _tui_widget_row_begin(Panel *panel);
static void _tui_widget_row_push(Panel *panel, Widget *widget);
static void _tui_widget_row_end(Panel *panel);

static inline int _tui_center_in_container(int base, int length, int container_length){
    base += (container_length - length + 1) / 2;
    return base;
}

static void _tui_render_widget(Widget *widget, vec2i position){
    widget->render(widget, LAYOUT_STATE.screen, position);
    //always reset color after a widget!
    screen_format(NORMAL, LAYOUT_STATE.screen->theme.colors[COLOR_TEXT]);
}

static void _tui_render_panel(Panel *panel, int scroll_offset){
    const int BASE_X = panel->outer_rect.pos.x + BORDER + PADDING;
    const int BASE_Y = panel->outer_rect.pos.y + BORDER;// + PADDING;
    vec2i cursor_pos = {.x = BASE_X, .y = BASE_Y};

    //in case there was still an open inline row from the definition pass
    _tui_widget_row_end(panel);

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
    int inline_row_height = 0;

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
                inline_row_width += next_widget->size.x;
                if(next_widget->size.h > inline_row_height)
                    inline_row_height = next_widget->size.h;
            }

            //centrar row horizontally
            auto centered_row = _tui_center_in_container(
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
            //immediately move the cursor down
            inline_row_height = 0;

            // center next widget horizontally
            cursor_pos.x = _tui_center_in_container(
                cursor_pos.x,
                widget->size.w,
                panel->inner_rect.size.w
            );
        }

        //center inline widgets vertically between them
        int render_y = widget->is_inline
            ? _tui_center_in_container(cursor_pos.y, widget->size.h, inline_row_height)
            : cursor_pos.y;
        //render current widget INSIDE panel boundaires
        int render_bottom = render_y + widget->size.h;
        if(render_bottom > panel->inner_rect.pos.y
        && render_y < panel->inner_rect.pos.y + panel->inner_rect.size.h){
            _tui_render_widget(widget, (vec2i){.x = cursor_pos.x, .y = render_y});
        }

        if (i >= panel->widget_count - 1) break; //no more panels, break early

        //hay siguiente?
        Widget *next_widget = (i < panel->widget_count - 1)
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
            cursor_pos.y += widget->is_inline ? inline_row_height : widget->size.y;
        }

    }

    //draw panel border
    if(panel->focused){
        screen_format(NORMAL, LAYOUT_STATE.screen->theme.colors[COLOR_PANEL_FOCUS]);
    }else{
        screen_format(NORMAL, LAYOUT_STATE.screen->theme.colors[COLOR_PANEL]);
    }
    tui_draw_box(LAYOUT_STATE.screen, panel->outer_rect);

    //panel scroll
    if(!panel->focused) return;
    if(!_tui_is_panel_scrollable(panel)) return;
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

static PageLayer* _tui_get_layer_focused(){
    PageLayer *layer_focused = &LAYOUT_STATE.layers[LAYOUT_STATE.layer_focused];
    return layer_focused;
}

static bool _tui_is_panel_scrollable(Panel *panel){
   return panel->widgets_rect.size.h > panel->inner_rect.size.h;
}

static Panel* _tui_get_panel_focused(){
    PageLayer *layer_focused = _tui_get_layer_focused();
    return &(layer_focused->panels[layer_focused->panel_focused]);
}

static Panel* _tui_get_panel_building(){
    PageLayer *layer_building = &(LAYOUT_STATE.layers[LAYOUT_STATE.layer_building]);
    return &(layer_building->panels[layer_building->panel_building]);
}

static Widget* _tui_get_widget_focused(){
    PageLayer *layer_focused = _tui_get_layer_focused();
    auto widget_focused_id = layer_focused->widget_focused[layer_focused->panel_focused];
    if(widget_focused_id == nullptr){
        return nullptr;
    }
    auto panel = _tui_get_panel_focused();
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

static rect2i _tui_panel_rect(PageLayout layout, PanelSlot slot, int base_w, int base_h){
    //panel size is based on the slot it occupies in the type of layout

    int sidebar_w = min(30, 0.4 * base_w);
    int header_h  = 8;
    int footer_h  = 8;

    switch(layout){
    case LAYOUT_SINGLE_PANEL: return (rect2i){.size = {base_w, base_h}};
    case LAYOUT_SIDEBAR_LEFT:
        switch(slot){
        case SLOT_SIDEBAR: return (rect2i){.size = {sidebar_w, base_h}};
        case SLOT_MAIN:    return (rect2i){.pos  = {sidebar_w, 0}, .size = {base_w - sidebar_w, base_h}};
        default: assert(false); //layout doesnt support this slot
        }
    case LAYOUT_SIDEBAR_RIGHT:
        switch(slot){
        case SLOT_MAIN:    return (rect2i){.size = {base_w - sidebar_w, base_h}};
        case SLOT_SIDEBAR: return (rect2i){.pos  = {base_w - sidebar_w, 0}, .size = {sidebar_w, base_h}};
        default: assert(false); //layout doesnt support this slot
        }
    case LAYOUT_SPLIT_VERTICAL:
        switch(slot){
        case SLOT_LEFT:    return (rect2i){.size = {base_w / 2, base_h}};
        case SLOT_RIGHT:   return (rect2i){.pos = {base_w / 2, 0}, .size = {base_w - base_w / 2, base_h}};
        default: assert(false); //layout doesnt support this slot
        }
    case LAYOUT_WITH_HEADER:
        switch(slot){
        case SLOT_TOP:    return (rect2i){.size = {base_w, header_h}};
        case SLOT_MAIN:   return (rect2i){.pos = {0, header_h}, .size = {base_w, base_h - header_h}};
        default: assert(false); //layout doesnt support this slot
        }
    case LAYOUT_WITH_FOOTER:
        switch(slot){
        case SLOT_MAIN:   return (rect2i){.size = {base_w, base_h - footer_h}};
        case SLOT_BOTTOM: return (rect2i){.pos = {0, base_h - footer_h}, .size = {base_w, footer_h}};
        default: assert(false); //layout doesnt support this slot
        }
    case LAYOUT_WITH_HEADER_AND_FOOTER:
        switch(slot){
        case SLOT_TOP:    return (rect2i){.size = {base_w, header_h}};
        case SLOT_MAIN:   return (rect2i){.pos = {0, header_h}, .size = {base_w, base_h - header_h - footer_h}};
        case SLOT_BOTTOM: return (rect2i){.pos = {0, base_h - footer_h}, .size = {base_w, footer_h}};
        default: assert(false); //layout doesnt support this slot
        }
    case LAYOUT_SPLIT_VERTICAL_WITH_HEADER:
        switch(slot){
        case SLOT_TOP:    return (rect2i){.size = {base_w, header_h}};
        case SLOT_LEFT:   return (rect2i){.pos = {0, header_h}, .size = {base_w / 2, base_h - header_h}};
        case SLOT_RIGHT:  return (rect2i){.pos = {base_w / 2, header_h}, .size = {base_w - base_w / 2, base_h - header_h}};
        default: assert(false); //layout doesnt support this slot
        }
    }

    assert(false); //ERROR: layout not implemented!
    return (rect2i){ .size = {base_w, base_h} };
}

static void _tui_panel_shrink_to_widgets(Panel *panel){
    //achicar panel al tamaño de los widgets
    panel->inner_rect.size = panel->widgets_rect.size;
    panel->outer_rect.size = panel->widgets_rect.size;

    //agrandar
    panel->outer_rect.size.w += PADDING * 2 + BORDER * 2;
    panel->outer_rect.size.h += PADDING * 2 + BORDER * 2;

    panel->outer_rect.size.w = max(32, panel->outer_rect.size.w);
    panel->outer_rect.size.h = max(8,  panel->outer_rect.size.h);

    panel->inner_rect.size.w = panel->outer_rect.size.w - PADDING * 2 - BORDER * 2;
    panel->inner_rect.size.h = panel->outer_rect.size.h - PADDING * 2 - BORDER * 2;
}

static void _tui_layer_shrink(PageLayer *layer) {
    // ffirst shrink all the panel to the widgets
    for(int i = 0; i < layer->panel_count; i++){
        _tui_panel_shrink_to_widgets(&layer->panels[i]);
    }

    //then fix panel gaps
    if(layer->panel_count > 1){
        for (int i = 1; i < layer->panel_count; i++) {
            Panel *prev = &layer->panels[i-1];
            Panel *curr = &layer->panels[i];

            //assume vertical stacking for simplicity.
            //TODO: this is probably terrible for other layouts but whatever
            curr->outer_rect.pos.y = prev->outer_rect.pos.y + prev->outer_rect.size.h;
            curr->outer_rect.pos.x = prev->outer_rect.pos.x;
            curr->inner_rect.pos.x = curr->outer_rect.pos.x + BORDER + PADDING;
            curr->inner_rect.pos.y = curr->outer_rect.pos.y + BORDER + PADDING;
        }
    }

    //now we expand the layer
    rect2i total_rect = {0};
    for (int i = 0; i < layer->panel_count; i++) {
        rect2i panel_rect = layer->panels[i].outer_rect;
        if(i == 0){
            total_rect = panel_rect;
            continue;
        }

        int right = max(
            total_rect.pos.x + total_rect.size.w,
            panel_rect.pos.x + panel_rect.size.w
        );
        int bottom = max(
            total_rect.pos.y + total_rect.size.h,
            panel_rect.pos.y + panel_rect.size.h
        );
        total_rect.pos.x  = min(total_rect.pos.x, panel_rect.pos.x);
        total_rect.pos.y  = min(total_rect.pos.y, panel_rect.pos.y);
        total_rect.size.w = right  - total_rect.pos.x;
        total_rect.size.h = bottom - total_rect.pos.y;
    }

    layer->rect = total_rect;

    //center
    int offset_x = (LAYOUT_STATE.base_size.w - total_rect.size.w) / 2 - total_rect.pos.x;
    int offset_y = (LAYOUT_STATE.base_size.h - total_rect.size.h) / 2 - total_rect.pos.y;

    for(int i = 0; i < layer->panel_count; i++){
        layer->panels[i].outer_rect.pos.x += offset_x;
        layer->panels[i].outer_rect.pos.y += offset_y;
        layer->panels[i].inner_rect.pos.x += offset_x;
        layer->panels[i].inner_rect.pos.y += offset_y;
    }

    layer->rect.pos.x += offset_x;
    layer->rect.pos.y += offset_y;
}

void tui_layer_begin(PageLayerKind layer, PageLayout layout){
    //switching to the requested layout and setting it's layout
    assert(layer != LAYER_COUNT);
    LAYOUT_STATE.layer_building       = layer;
    LAYOUT_STATE.layers[layer].layout = layout;
}

void tui_layer_end(void){
    //go back to the base layer, making no changes to it
    LAYOUT_STATE.layer_building = LAYER_BASE;
}

void tui_panel_begin(PanelSlot slot){
    assert(LAYOUT_STATE.layer_building < LAYER_COUNT);
    PageLayer *layer_building = &(LAYOUT_STATE.layers[LAYOUT_STATE.layer_building]);
    assert(layer_building->panel_building == -1); //close the prev panel first!
    assert(layer_building->panel_count < TUI_PANELS_MAX);

    auto panel_rect = _tui_panel_rect(
        layer_building->layout, slot,
        LAYOUT_STATE.base_size.w, LAYOUT_STATE.base_size.h
    );
    panel_rect.pos.y += 1; //leave space for the app title
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
    layer_building->panel_building = layer_building->panel_count;
    layer_building->panels[layer_building->panel_count++] = new_panel;
}

void tui_panel_end(void){
    PageLayer *layer_building = &(LAYOUT_STATE.layers[LAYOUT_STATE.layer_building]);
    assert(layer_building->panel_building != -1); //no panel to close
    layer_building->panel_building = -1;
}

void tui_panel_scroll(int offset){
    PageLayer *layer_focused = _tui_get_layer_focused();
    Panel     *panel         = _tui_get_panel_focused();
    auto       panel_idx     = layer_focused->panel_focused;
    int        scroll_offset = layer_focused->panel_scroll_offset[panel_idx];

    layer_focused->panel_scroll_offset[layer_focused->panel_focused] = clamp(
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

char* tui_create_widget_id(){
    PageLayer *layer_building = &LAYOUT_STATE.layers[LAYOUT_STATE.layer_building];
    char *new_id = (char *)arena_alloc(LAYOUT_STATE.arena_frame, sizeof(char) * 16);
    sprintf(new_id, "auto_id_%d", layer_building->widget_auto_id++);
    return new_id;
}

static inline Widget* _tui_get_latest_widget(){
    PageLayer *layer_building = &LAYOUT_STATE.layers[LAYOUT_STATE.layer_building];
    Panel *panel = &layer_building->panels[layer_building->panel_building];
    if(panel->widget_count == 0) return nullptr;
    return &panel->widgets[panel->widget_count - 1];
}

static inline Widget* _tui_get_new_widget(char const *widget_id){
    PageLayer *layer_building = &LAYOUT_STATE.layers[LAYOUT_STATE.layer_building];
    assert(layer_building->panel_building != -1); //must be used inside a tui_panel_begin!
    Panel *panel = &layer_building->panels[layer_building->panel_building];
    assert(panel->widget_count < TUI_WIDGETS_IN_PANEL_MAX); //too many widgets!

    Widget *new_widget = &panel->widgets[panel->widget_count++];
    new_widget->id = widget_id;

    return new_widget;
}

void* tui_widget_state(const char *widget_id, size_t data_size){
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

static void _tui_widget_row_begin(Panel *panel){
    //reset row
    panel->curr_row_size = (vec2i){.w = 0, .h = 0};
}

static void _tui_widget_row_push(Panel *panel, Widget *widget){
    //pushes widget to row
    //should NOT modify the widgets_rect !

    //HEIGHT: biggest widget height remains
    if(widget->size.h > panel->curr_row_size.h){
        panel->curr_row_size.h = widget->size.h;
    }

    //WIDTH:  widget width is always added to row width
    panel->curr_row_size.w += widget->size.w;
}

static void _tui_widget_row_end(Panel *panel){
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
    PageLayer *layer_building = &LAYOUT_STATE.layers[LAYOUT_STATE.layer_building];
    assert(layer_building->panel_building != -1); //must be used inside a tui_panel_begin!
    Widget *last_widget = _tui_get_latest_widget();
    Widget *new_widget  = _tui_get_new_widget(widget.id);
    memcpy(new_widget, &widget, sizeof(Widget));

    //widget focus
    auto panel_focused_widget = layer_building->widget_focused[layer_building->panel_building];
    new_widget->focused = panel_focused_widget != NULL && strcmp(new_widget->id, panel_focused_widget) == 0;

    //panel row, increases widget rect in panel
    Panel *panel = &layer_building->panels[layer_building->panel_building];
    bool inside_row = (last_widget == nullptr || last_widget->is_inline);
    if(new_widget->is_inline && !inside_row){
        //comienza nueva row inline
        _tui_widget_row_begin(panel);
        _tui_widget_row_push(panel, new_widget);
    }else if(new_widget->is_inline && inside_row){
        //ya estaba en una row inline y la continuo
        _tui_widget_row_push(panel, new_widget);
    }else{ //widget not inline
        _tui_widget_row_end(panel); //ten case de que haya habido una row abierta
        //widgets not inline have their own row
        _tui_widget_row_begin(panel);
        _tui_widget_row_push(panel, new_widget);
        _tui_widget_row_end(panel);
    }
}

static void _tui_layout_prepare(Screen *screen, PageLayout layout){
    PageLayer *layer_building = &LAYOUT_STATE.layers[LAYOUT_STATE.layer_building];

    if(LAYOUT_STATE.arena_frame == nullptr){
        LAYOUT_STATE.arena_frame = arena_init(1024 * 1024 * 5); //5mb
    }

    if(WIDGET_REGISTRY.arena == nullptr){
        WIDGET_REGISTRY.arena = arena_init(1024 * 1024 * 5); //5mb
    }

    layer_building->panel_building = -1;
    layer_building->panel_count    = 0;
    layer_building->widget_auto_id = 0;

    LAYOUT_STATE.layers[LAYER_BASE].layout = layout;

    //NOTE: base size has h-2 to leave room for header at top
    //      and key hints at bottom
    LAYOUT_STATE.base_size.w = screen->size.w;
    LAYOUT_STATE.base_size.h = screen->size.h - 2;

    assert(screen != NULL); //apagaste el monitor capo??
    LAYOUT_STATE.screen = screen;
}

static bool _tui_widget_focused_input(InputEvent input_event){
    Widget *widget = _tui_get_widget_focused();
    if(widget == nullptr) return false;
    if(widget->input == nullptr) return false;
    return widget->input(widget, input_event);
}

static void _tui_layout_evaluate_layer_focused(void){
    //decide focused layer first, from top to bottom
    // for(PageLayerKind layer_idx = LAYER_WIDGETS_OVERLAY_DO_NOT_USE; layer_idx >= 0; layer_idx--){
    for(int layer_idx = 2; layer_idx >= 0; layer_idx--){
        PageLayer *layer = &LAYOUT_STATE.layers[layer_idx];

        //if any of its panels has any widget at all, then it's the focused one
        for(int i = 0; i < layer->panel_count; i++){
            Panel *panel = &layer->panels[i];
            if(panel->widget_count == 0) continue;

            LAYOUT_STATE.layer_focused = layer_idx;
            return;
        }
    }
}

static void _tui_layout_build_widget_overlays(void){
    //first of all we build the widget overlays, very important!
    LAYOUT_STATE.widget_overlay_title = NULL;
     for(PageLayerKind layer_idx = LAYER_BASE; layer_idx < LAYER_COUNT; layer_idx++){
        PageLayer *layer = &LAYOUT_STATE.layers[layer_idx];
        for(int i = 0; i < layer->panel_count; i++){
            Panel *panel = &layer->panels[i];
            for(int j = 0; j < panel->widget_count; j++){
                Widget *widget = &panel->widgets[j];
                if(!widget->overlay) continue;
                if(!widget->focused) continue;
                if(!LAYOUT_STATE.widget_overlay_active) continue;
                LAYOUT_STATE.widget_overlay_title = widget->overlay_title;
                widget->overlay(widget);
            }
        }
    }

    //evaluate focused layer
    _tui_layout_evaluate_layer_focused();
}

static void _tui_layout_render(){
    //render layers from bottom to top
    for(PageLayerKind layer_idx = 0; layer_idx < LAYER_COUNT; layer_idx++){
        PageLayer *layer = &LAYOUT_STATE.layers[layer_idx];
        layer->focused   = (LAYOUT_STATE.layer_focused == layer_idx);

        //render panels of layer
        if(layer->shrink){
            _tui_layer_shrink(layer);
        }

        for(int i = 0; i < layer->panel_count; i++){
            Panel *panel = &layer->panels[i];
            panel->focused = layer->focused && (layer->panel_focused == i);

            //unfocus all widgets in non-focused panels so they
            //render normally while preserving focus history
            if(!panel->focused){
                for(int j = 0; j < panel->widget_count; j++){
                    panel->widgets[j].focused = false;
                }
            }

            //clear panel background
            tui_draw_rect(LAYOUT_STATE.screen, EMPTY_U8, panel->outer_rect);

            //render
            int scroll_offset = layer->panel_scroll_offset[i];
            _tui_render_panel(panel, scroll_offset);
        }

        //draw overlay title on the top-most border of the overlay
        if(layer_idx == LAYER_WIDGETS_OVERLAY_DO_NOT_USE
        && LAYOUT_STATE.widget_overlay_title != NULL
        && layer->panel_count > 0){
            String title_str = string_from(
                (uint8_t *)LAYOUT_STATE.widget_overlay_title,
                strlen((const char *)LAYOUT_STATE.widget_overlay_title)
            );
            tui_draw_box_title(
                LAYOUT_STATE.screen,
                layer->rect,
                &title_str,
                BOX_TITLE_TOP_LEFT
            );
        }

        //reset the layer state for next frame
        layer->panel_building = -1;
        layer->panel_count = 0;
    }

    //reset arena
    arena_reset(LAYOUT_STATE.arena_frame);
}

static void _tui_layout_reset(void){
    //this should be called right after the currently active page is changed,
    //to ensure no dirty data remains in the layout
    //This should not be called between each frame, because we destroy the
    //entire registry and all state!

    //reset arenas
    if(WIDGET_REGISTRY.arena != nullptr){
        arena_reset(WIDGET_REGISTRY.arena);
    }
    if(LAYOUT_STATE.arena_frame != nullptr){
        arena_reset(LAYOUT_STATE.arena_frame);
    }

    Arena *saved_arena_frame = LAYOUT_STATE.arena_frame;
    Arena *saved_widget_arena = WIDGET_REGISTRY.arena;

    //reset widgets registry
    WIDGET_REGISTRY = (_WidgetStateRegistry){ .arena = saved_widget_arena };

    //reset state
    LAYOUT_STATE = (_LayoutState){
        .layers = {
            [LAYER_BASE]                       = {.panel_building = -1, .shrink = false},
            [LAYER_OVERLAY]                    = {.panel_building = -1, .shrink = true},
            [LAYER_WIDGETS_OVERLAY_DO_NOT_USE] = {.panel_building = -1, .shrink = true},
        },
        .arena_frame = saved_arena_frame,
    };
}

//focus navigation

static void _tui_cursor_next_widget(void){
    PageLayer *layer_focused = _tui_get_layer_focused();
    Panel *panel = _tui_get_panel_focused();
    if(panel->widget_count == 0) return;

    auto panel_idx = layer_focused->panel_focused;
    auto widget_focused = layer_focused->widget_focused[panel_idx];
    int i = -1; //default focused widget

    // find currently focused widget index
    if(widget_focused != NULL){
        for(i = 0; i < panel->widget_count; i++){
            auto widget = panel->widgets[i];
            if(strcmp(widget_focused, widget.id) == 0){
                break;
            }
        }
    }

    //find the next focusable widget, wrapping around
    for(int j = 1; j <= panel->widget_count; j++){
        auto widget = panel->widgets[(i + j) % panel->widget_count];
        if(widget.focusable){
            layer_focused->widget_focused[layer_focused->panel_focused] = widget.id;
            break;
        }
    }
}

static void _tui_cursor_prev_widget(void){
    PageLayer *layer_focused = _tui_get_layer_focused();
    Panel *panel = _tui_get_panel_focused();
    if(panel->widget_count == 0) return;

    auto panel_idx = layer_focused->panel_focused;
    auto widget_focused = layer_focused->widget_focused[panel_idx];
    int i = panel->widget_count; //default focused widget

    // find currently focused widget index
    if(widget_focused != NULL){
        for(i = 0; i < panel->widget_count; i++){
            auto widget = panel->widgets[i];
            if(strcmp(widget_focused, widget.id) == 0 ){
                break;
            }
        }
    }

    //find the next focusable widget, wrapping around
    for(int j = 0; j < panel->widget_count; j++){
        int widget_index = (i - j + panel->widget_count - 1) % panel->widget_count;
        auto widget = panel->widgets[widget_index];
        if(widget.focusable){
            layer_focused->widget_focused[layer_focused->panel_focused] = widget.id;
            break;
        }
    }
}

static void _tui_cursor_next_panel(void){
    PageLayer *layer_focused = _tui_get_layer_focused();
    if(layer_focused->panel_count <= 1) return;
    int next_panel = (layer_focused->panel_focused + 1) % layer_focused->panel_count;
    layer_focused->panel_focused = (uint8_t)next_panel;
}

static void _tui_cursor_prev_panel(void){
    PageLayer *layer_focused = _tui_get_layer_focused();
    if(layer_focused->panel_count <= 1) return;
    //we cant do it the same way as the other because the % operator in C is UB with negative numbers
    int next_panel = (layer_focused->panel_focused + layer_focused->panel_count - 1) % layer_focused->panel_count;
    layer_focused->panel_focused = (uint8_t)next_panel;
}

#endif //TUI_LAYOUT_IMPL
#endif //TUI_LAYOUT

#ifndef TUI_WIDGET_VIRTUAL_LIST
#define TUI_WIDGET_VIRTUAL_LIST

#include "tui_layout.h"

typedef struct {
    const uint8_t *label;
} WidgetVirtualListItem;

typedef struct {
    WidgetVirtualListItem *items;
    size_t count;
} WidgetVirtualListItems;

typedef struct {
    bool                    is_inline;
    size_t                 *storage;
    WidgetVirtualListItems  items;
    FunctionPointer         on_select;
    FunctionPointer         on_click;
} WidgetVirtualListParams;

#define tui_widget_virtual_list(widget_id, ...) \
        tui_widget_virtual_list_((widget_id), &(WidgetVirtualListParams){__VA_ARGS__})

void tui_widget_virtual_list_(const char *widget_id, WidgetVirtualListParams *params);

#ifdef TUI_WIDGET_VIRTUAL_LIST_IMPL

typedef struct {
    size_t                *storage;
    WidgetVirtualListItems items;
    FunctionPointer        on_select;
    FunctionPointer        on_click;
    size_t                 max_label_width;
    Panel                 *panel;
} WidgetVirtualListData;

typedef struct {
    size_t selected_index;
    int    last_render_y;
    bool   has_rendered;
} WidgetVirtualListState;

private void tui_widget_virtual_list_auto_scroll(WidgetVirtualListState *state){
    if(!state->has_rendered) return;

    Panel *panel      = tui_get_panel_focused();
    rect2i visible    = panel->inner_rect;
    int first_visible = visible.pos.y - state->last_render_y;
    int last_visible  = first_visible + visible.size.h;

    if((int)state->selected_index < first_visible){
        tui_panel_scroll((int)state->selected_index - first_visible);
    }else if((int)state->selected_index >= last_visible){
        tui_panel_scroll((int)state->selected_index - last_visible + 1);
    }
}

private void tui_widget_virtual_list_render(Widget *widget, Screen *screen, vec2i position){
    WidgetVirtualListData  *data  = widget->data;
    WidgetVirtualListState *state = widget->state;

    state->last_render_y = position.y;
    state->has_rendered  = true;

    rect2i visible = data->panel->inner_rect;
    int first      = max(0, visible.pos.y - position.y);
    int last       = min(data->items.count, visible.pos.y + visible.size.h - position.y);

    for(int i = first; i < last; i++){
        bool selected = (i == (int)state->selected_index);
        int y = position.y + i;

        if(selected && widget->focused){
            screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
        }else if(selected){
            screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
        }else{
            screen_format(NORMAL, COLOR_DARK_WHITE, COLOR_BLACK);
        }

        auto selector_icon = selected ? u8"> " : u8"  ";
        screen_set_utf8_str(screen, position.x + PADDING, y, selector_icon);
        screen_set_utf8_str(screen, position.x + PADDING + 2, y, data->items.items[i].label);
    }
}

private bool tui_widget_virtual_list_input(Widget *widget, InputEvent input_event){
    WidgetVirtualListData  *data  = widget->data;
    WidgetVirtualListState *state = widget->state;

    switch(input_event.input_type){
    case INPUT_KEY:
        switch(input_event.key_event.key){
        case KEY_UP:
            if(state->selected_index == 0){
                state->selected_index = data->items.count - 1;
            }else{
                state->selected_index--;
            }
            memcpy(data->storage, &state->selected_index, sizeof(size_t));
            if(data->on_select != nullptr) data->on_select();
            tui_widget_virtual_list_auto_scroll(state);
            return true;
        case KEY_DOWN:
            state->selected_index = (state->selected_index + 1) % data->items.count;
            memcpy(data->storage, &state->selected_index, sizeof(size_t));
            if(data->on_select != nullptr) data->on_select();
            tui_widget_virtual_list_auto_scroll(state);
            return true;
        case KEY_ENTER:
            if(data->on_click != nullptr){
                data->on_click();
                return true;
            }
            return false;
        default:
            break;
        }
    default:
        break;
    }
    return false;
}

void tui_widget_virtual_list_(const char *widget_id, WidgetVirtualListParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->items.count > 0);

    WidgetVirtualListData *data = (WidgetVirtualListData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetVirtualListData)
    );
    data->storage   = params->storage;
    data->items     = params->items;
    data->on_select = params->on_select;
    data->on_click  = params->on_click;
    data->panel     = tui_get_panel_building();

    size_t max_label_width = 0;
    for(size_t i = 0; i < params->items.count; i++){
        size_t w = utf8_str_display_width(params->items.items[i].label);
        if(w > max_label_width) max_label_width = w;
    }
    data->max_label_width = max_label_width;

    WidgetVirtualListState *state = (WidgetVirtualListState *)tui_widget_state(
        widget_id, sizeof(WidgetVirtualListState)
    );
    state->selected_index = *params->storage;
    //NOTE: last_render_y and has_rendered persist from render calls, do NOT reset here!!

    Widget new_widget = {
        .id        = widget_id,
        .data      = data,
        .state     = state,
        .size.w    = (int)(max_label_width + 2 + PADDING * 2),
        .size.h    = (int)params->items.count,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &tui_widget_virtual_list_input,
        .render    = &tui_widget_virtual_list_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_VIRTUAL_LIST_IMPL
#endif //TUI_WIDGET_VIRTUAL_LIST

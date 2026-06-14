#ifndef TUI_WIDGET_SELECT_FILTER
#define TUI_WIDGET_SELECT_FILTER

#include "tui_layout.h"
#include "tui_platform.h"
#include "tui_screen.h"
#include "tui_widget_select.h"
#include <string.h>
// #include <stdlib.h>
#include "tui_utils.h"

//TODO: right now we're duplicating A LOT of code from the text input
//      might wanna see if we can refactor that out into helpers

typedef WidgetSelectOptions (*WidgetOptionsFunction)(const uint8_t *query);

typedef struct {
    bool                    is_inline;
    const uint8_t          *label;
    size_t                 *storage;
    WidgetOptionsFunction   options_function;
    const uint8_t          *overlay_title;
    const uint8_t          *placeholder;
    const uint8_t          *empty_options_label;
} _WidgetSelectFilterParams;

#define tui_widget_select_filter(widget_id, ...) \
        tui_widget_select_filter_((widget_id), &(_WidgetSelectFilterParams){__VA_ARGS__})

void tui_widget_select_filter_(const char *widget_id, _WidgetSelectFilterParams *params);

#ifdef TUI_WIDGET_SELECT_FILTER_IMPL

typedef struct {
    size_t                *storage;
    const uint8_t         *label;
    size_t                 label_width;
    size_t                 input_width;
    const uint8_t         *placeholder;
    const uint8_t         *empty_options_label;
    WidgetOptionsFunction  options_function;
} _WidgetSelectFilterData;

typedef struct {
    uint8_t             query_buffer[128];
    String              query;
    size_t              cursor; // char index
    size_t              scroll; // char index
    bool                caret_show;
    double              caret_interval;
    double              caret_last_shown;
    WidgetSelectOptions current_options;
    int                 selected_index;
} _WidgetSelectFilterState;

static void _tui_widget_select_filter_update_options(_WidgetSelectFilterData *data, _WidgetSelectFilterState *state){
    if(data->options_function == nullptr) return;
    state->current_options = data->options_function(state->query.data);
    if(state->selected_index >= (int)state->current_options.count){
        state->selected_index = (int)state->current_options.count - 1;
    }
    if(state->current_options.count == 0){
        state->selected_index = -1;
    }
    state->scroll = 0;
}

static void _tui_widget_select_filter_auto_scroll(_WidgetSelectFilterState *state, int visible_rows){
    if(state->selected_index < 0) return;
    if((int)state->scroll > state->selected_index){
        state->scroll = (size_t)state->selected_index;
    }
    if(state->selected_index >= (int)state->scroll + visible_rows){
        state->scroll = (size_t)(state->selected_index - visible_rows + 1);
    }
}

static void _tui_widget_select_filter_dropdown_render(Widget *widget, Screen *screen, vec2i position){
    _WidgetSelectFilterData  *data  = widget->data;
    _WidgetSelectFilterState *state = widget->state;

    //scroll based on cursor location
    if(state->cursor > data->input_width){
        state->scroll = state->cursor - data->input_width;
    }else{
        state->scroll = 0;
    }

    //caret logic
    tui_text_edit_tick_caret(&state->caret_last_shown, &state->caret_show, state->caret_interval);

    if(state->query.length == 0){
        //draw placeholder
        screen_format(NORMAL, COLOR_FG_SECONDARY, COLOR_BG_TEXT);
        auto placeholder_substr = string_from_substr(data->placeholder, 0, data->input_width);
        screen_set_string(screen, position.x + PADDING, position.y, &placeholder_substr);
    }else{
        //draw text
        screen_format(NORMAL, COLOR_FG_TEXT, COLOR_BG_TEXT);
        auto text_substr = string_substr(
            &state->query,
            state->scroll,
            min(state->query.length, state->scroll + data->input_width)
        );
        screen_set_string(screen, position.x + PADDING, position.y, &text_substr);
    }

    //caret
    if(state->caret_show){
        int caret_x = position.x + PADDING + (int)(state->cursor - state->scroll);
        if(state->cursor < state->query.length){
            screen_format(NORMAL, COLOR_BG_TEXT, COLOR_FG_PRIMARY);
            auto char_substr = string_substr(&state->query, state->cursor, state->cursor + 1);
            screen_set_string(screen, caret_x, position.y, &char_substr);
        }else{
            screen_format(NORMAL, COLOR_FG_PRIMARY, COLOR_BG_TEXT);
            screen_set_utf8(screen, caret_x, position.y, u8"█");
        }
    }

    //separator
    int sep_y = position.y + 1;
    screen_format(NORMAL, COLOR_FG_SECONDARY, COLOR_BG_TEXT);
    for(int x = position.x; x < position.x + widget->size.w; x++){
        screen_set_utf8(screen, x, sep_y, u8"─");
    }

    //options list
    int list_y = sep_y + 1;
    int base_x = position.x + PADDING;
    int visible = widget->size.h - (list_y - position.y + PADDING);

    if(state->current_options.count == 0){ //no options
        screen_format(NORMAL, COLOR_FG_SECONDARY, COLOR_BG_TEXT);
        screen_set_utf8_str(screen, base_x, list_y, data->empty_options_label);
        screen_format(NORMAL, COLOR_FG_TEXT, COLOR_BG_TEXT);
        return;
    }

    for(int i = 0; i < visible && (int)(state->scroll + i) < (int)state->current_options.count; i++){
        int idx = (int)(state->scroll + i);
        if(idx == state->selected_index){
            //selected
            screen_format(NORMAL, COLOR_BG_TEXT, COLOR_FG_PRIMARY);
            for(int x = position.x; x < position.x + widget->size.w; x++){
                screen_set_char(screen, x, list_y + i, EMPTY_CHAR);
            }
        }else{
            screen_format(NORMAL, COLOR_FG_TEXT, COLOR_BG_TEXT);
        }
        screen_set_utf8_str(screen, base_x, list_y + i, state->current_options.values[idx].label);
    }

    //reset format
    screen_format(NORMAL, COLOR_FG_TEXT, COLOR_BG_TEXT);
}

static bool _tui_widget_select_filter_dropdown_input(Widget *widget, InputEvent input_event){
    _WidgetSelectFilterData  *data  = widget->data;
    _WidgetSelectFilterState *state = widget->state;
    bool text_changed = false;

    switch (input_event.input_type) {
    case INPUT_KEY:
        auto key = input_event.key_event;
        switch (input_event.key_event.key) {
        case KEY_ENTER:
            if(state->selected_index >= 0
            && state->selected_index < (int)state->current_options.count){
                *data->storage = state->current_options.values[state->selected_index].value;
                _tui_widget_overlay_close();
            }
            return true;
        case KEY_ESCAPE:
            _tui_widget_overlay_close();
            return true;
        case KEY_UP:
            if(state->current_options.count > 0){
                if(state->selected_index <= 0){
                    state->selected_index = (int)state->current_options.count - 1;
                }else{
                    state->selected_index--;
                }
            }
            return true;
        case KEY_DOWN:
            if(state->current_options.count > 0){
                if(state->selected_index >= (int)state->current_options.count - 1){
                    state->selected_index = 0;
                }else{
                    state->selected_index++;
                }
            }
            return true;
        case KEY_F1:
        case KEY_HOME:
            tui_text_edit_reset_cursor(&state->caret_show, &state->caret_last_shown);
            state->cursor = 0;
            return true;
        case KEY_F2:
        case KEY_END:
            tui_text_edit_reset_cursor(&state->caret_show, &state->caret_last_shown);
            state->cursor = state->query.length;
            return true;
        case KEY_LEFT:
            tui_text_edit_reset_cursor(&state->caret_show, &state->caret_last_shown);
            tui_text_edit_move_cursor(&state->cursor, -1, state->query.length);
            return true;
        case KEY_RIGHT:
            tui_text_edit_reset_cursor(&state->caret_show, &state->caret_last_shown);
            tui_text_edit_move_cursor(&state->cursor, +1, state->query.length);
            return true;
        case KEY_BACKSPACE:
            tui_text_edit_reset_cursor(&state->caret_show, &state->caret_last_shown);
            tui_text_edit_backspace(&state->query, &state->cursor);
            text_changed = true;
            return true;
        case KEY_DELETE:
            tui_text_edit_reset_cursor(&state->caret_show, &state->caret_last_shown);
            tui_text_edit_delete(&state->query, state->cursor);
            text_changed = true;
            return true;
        case KEY_TAB:
        case KEY_BACKTAB:
            return false;
        case KEY_NONE:
        default:
            if(key.unicode == 0) break;
            if(state->query.bytes + 4 >= state->query.capacity) break;
            tui_text_edit_insert(&state->query, &state->cursor, key.unicode);
            text_changed = true;
            break;
        }
    case INPUT_NONE:
    default:
    }

    if(text_changed){
        state->query.data[state->query.bytes] = '\0';
        _tui_widget_select_filter_update_options(data, state);
    }

    return true;
}

static void _tui_widget_select_filter_overlay(Widget *widget){
    _WidgetSelectFilterData  *data  = widget->data;
    _WidgetSelectFilterState *state = widget->state;

    //QUERY AGAIN!!!
    _tui_widget_select_filter_update_options(data, state);

    //size of dropdown
    int max_opt_width = 0;
    for(size_t i = 0; i < state->current_options.count; i++){
        int width = (int)utf8_str_display_width(
            state->current_options.values[i].label
        );
        if(width > max_opt_width){
            max_opt_width = width;
        }
    }
    int content_w = max((int)data->input_width, max_opt_width);
    int width     = content_w + PADDING * 2;
    width         = max(width, 32);
    width         = min(width, LAYOUT_STATE.base_size.w - 4);

    int list_rows = state->current_options.count;
    int height    = min(list_rows, 64) + PADDING + 2;
    height        = min(height, LAYOUT_STATE.base_size.h - 4);
    height        = max(height, 2 + 1 + PADDING); //TODO: ya ni se que estoy sumando

    //scroll
    int visible_rows = height - (2 + PADDING);
    _tui_widget_select_filter_auto_scroll(state, visible_rows);

    tui_layer_begin(LAYER_WIDGETS_OVERLAY_DO_NOT_USE, LAYOUT_SINGLE_PANEL);
        tui_panel_begin(SLOT_MAIN);
            //a bit hacky..
            Widget dropdown = {
                .id        = widget->id,
                .data      = data,
                .state     = state,
                .size      = {.w = width, .h = height},
                .focusable = true,
                .is_inline = false,
                .input     = &_tui_widget_select_filter_dropdown_input,
                .render    = &_tui_widget_select_filter_dropdown_render,
            };
            tui_widget_push(dropdown);

            //ensure the dropdown starts focused!!
            PageLayer *layer = &LAYOUT_STATE.layers[LAYOUT_STATE.layer_building];
            layer->widget_focused[layer->panel_building] = dropdown.id;

        tui_panel_end();
    tui_layer_end();
}

static const uint8_t* _tui_widget_select_filter_label(_WidgetSelectFilterData *data, _WidgetSelectFilterState *state){
    if(state == nullptr) return u8"";
    if(state->current_options.count == 0) return u8"";

    size_t selected_index = 0;

    if(data->storage != nullptr){
        for(size_t i = 0; i < state->current_options.count; i++){
            if(state->current_options.values[i].value == *data->storage){
                selected_index = i;
                break;
            }
        }
    }

    return state->current_options.values[selected_index].label;
}

static void _tui_widget_select_filter_render(Widget *widget, Screen *screen, vec2i position){
    _WidgetSelectFilterData  *data  = widget->data;
    _WidgetSelectFilterState *state = widget->state;

    auto selected_label  = _tui_widget_select_filter_label(data, state);
    size_t selected_width = utf8_str_display_width(selected_label);

    if(widget->focused){
        screen_format(BOLD, COLOR_FG_PRIMARY, COLOR_BG_TEXT);
    }else{
        screen_format(NORMAL, COLOR_FG_TEXT, COLOR_BG_TEXT);
    }

    screen_set_utf8_str(screen, position.x + PADDING, position.y, data->label);
    screen_set_utf8_str(screen, position.x + PADDING + (int)data->label_width, position.y, selected_label);
    screen_set_utf8_str(
        screen,
        position.x + PADDING + (int)data->label_width + selected_width,
        position.y,
        u8" ▼"
    );
}

static bool _tui_widget_select_filter_input(Widget *widget, InputEvent input_event){
    _WidgetSelectFilterState *state = widget->state;

    switch(input_event.input_type){
    case INPUT_KEY:
        switch(input_event.key_event.key){
        case KEY_ENTER:
            if(_tui_widget_overlay_is_open()){
                _tui_widget_overlay_close();
            }else{
                //reset query
                state->query.data[0] = '\0';
                state->query.bytes = 0;
                state->query.length = 0;
                state->cursor = 0;
                _tui_widget_overlay_open();
            }
            return true;
        case KEY_ESCAPE:
            if(!_tui_widget_overlay_is_open()) break;
            _tui_widget_overlay_close();
            return true;
        case KEY_UP:
        case KEY_DOWN:
            if(!_tui_widget_overlay_is_open()) break;
            return false;
        case KEY_NONE:
        default:
            break;
        }
    case INPUT_NONE:
    default:
        break;
    }

    return _tui_widget_overlay_is_open();
}

void tui_widget_select_filter_(const char *widget_id, _WidgetSelectFilterParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->placeholder != nullptr);
    assert(params->empty_options_label != nullptr);

    int placeholder_len = (int)utf8_str_display_width(params->placeholder);

    _WidgetSelectFilterData *widget_data = (_WidgetSelectFilterData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(_WidgetSelectFilterData)
    );
    widget_data->storage             = params->storage;
    widget_data->label               = params->label;
    widget_data->label_width         = utf8_str_display_width(params->label);
    widget_data->options_function    = params->options_function;
    widget_data->placeholder         = params->placeholder;
    widget_data->empty_options_label = params->empty_options_label;
    widget_data->input_width         = max(16, placeholder_len + 1);

    auto widget_state = (_WidgetSelectFilterState *)tui_widget_state(
        widget_id,
        sizeof(_WidgetSelectFilterState)
    );
    if(widget_state->caret_interval == 0.0){
        widget_state->caret_interval = 0.5;
    }
    widget_state->query = string_from(widget_state->query_buffer, sizeof(widget_state->query_buffer));
    widget_state->cursor = clamp((int)widget_state->cursor, 0, (int)widget_state->query.length);
    if(params->options_function != nullptr){
        widget_state->current_options = params->options_function(widget_state->query.data);
    }

    auto selected_label   = _tui_widget_select_filter_label(widget_data, widget_state);
    size_t selected_width = utf8_str_display_width(selected_label);

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .state     = widget_state,
        .size.w    = widget_data->label_width + selected_width + 2 + PADDING * 2,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &_tui_widget_select_filter_input,
        .render    = &_tui_widget_select_filter_render,
        .overlay   = &_tui_widget_select_filter_overlay,
        .overlay_title = params->overlay_title ? params->overlay_title : params->label,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_SELECT_FILTER_IMPL
#endif //TUI_WIDGET_SELECT_FILTER

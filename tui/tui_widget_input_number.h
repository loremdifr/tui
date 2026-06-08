#ifndef TUI_WIDGET_INPUT_NUMBER
#define TUI_WIDGET_INPUT_NUMBER

#include "tui_layout.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    bool           is_inline;
    const uint8_t *label;
    int           *storage;
    int            min_value;
    int            max_value;
    int            step;
} WidgetInputNumberParams;

#define tui_widget_input_number(widget_id, ...) \
        tui_widget_input_number_((widget_id), &(WidgetInputNumberParams){__VA_ARGS__})

void tui_widget_input_number_(const char *widget_id, WidgetInputNumberParams *params);

#ifdef TUI_WIDGET_INPUT_NUMBER_IMPL

typedef struct {
    const uint8_t *label;
    int           *storage;
    int            min_value;
    int            max_value;
    int            step;
    size_t         label_width;
    size_t         value_width;
} WidgetInputNumberData;

typedef struct {
    size_t cursor;
    bool   editing;
    bool   caret_show;
    double caret_interval;
    double caret_last_shown;
    char   buffer[32];
    size_t buffer_len;
} WidgetInputNumberState;

private void tui_widget_input_number_sync_buffer(WidgetInputNumberState *state, int value){
    int written = snprintf(state->buffer, sizeof(state->buffer), "%d", value);
    assert(written > 0);
    assert((size_t)written < sizeof(state->buffer));
    state->buffer_len = (size_t)written;
    state->cursor = state->buffer_len;
}

private int tui_widget_input_number_parse_buffer(WidgetInputNumberState *state, bool *valid){
    char *end = nullptr;
    long parsed = strtol(state->buffer, &end, 10);
    if(end == state->buffer || *end != '\0'){
        if(valid) *valid = false;
        return 0;
    }
    if(valid) *valid = true;
    return (int)parsed;
}

private void tui_widget_input_number_apply_buffer(Widget *widget){
    WidgetInputNumberData  *data  = widget->data;
    WidgetInputNumberState *state = widget->state;
    bool valid = false;
    int value = tui_widget_input_number_parse_buffer(state, &valid);
    if(!valid) return;
    *data->storage = clamp(value, data->min_value, data->max_value);
    tui_widget_input_number_sync_buffer(state, *data->storage);
}

private void tui_widget_input_number_move_cursor(Widget *widget, int move){
    WidgetInputNumberState *state = widget->state;
    int next = (int)state->cursor + move;
    state->cursor = (size_t)clamp(next, 0, (int)state->buffer_len);
}

private void tui_widget_input_number_delete_before_cursor(Widget *widget){
    WidgetInputNumberState *state = widget->state;
    if(state->cursor == 0) return;
    memmove(
        &state->buffer[state->cursor - 1],
        &state->buffer[state->cursor],
        state->buffer_len - state->cursor + 1
    );
    state->cursor--;
    state->buffer_len--;
    tui_widget_input_number_apply_buffer(widget);
}

private void tui_widget_input_number_delete_at_cursor(Widget *widget){
    WidgetInputNumberState *state = widget->state;
    if(state->cursor >= state->buffer_len) return;
    memmove(
        &state->buffer[state->cursor],
        &state->buffer[state->cursor + 1],
        state->buffer_len - state->cursor
    );
    state->buffer_len--;
    tui_widget_input_number_apply_buffer(widget);
}

private void tui_widget_input_number_insert_char(Widget *widget, char chr){
    WidgetInputNumberData  *data  = widget->data;
    WidgetInputNumberState *state = widget->state;

    if(state->buffer_len + 1 >= sizeof(state->buffer)) return;
    if(chr == '-' && state->cursor != 0) return;
    if(chr == '-' && data->min_value >= 0) return;
    if(chr != '-' && (chr < '0' || chr > '9')) return;
    if(chr == '-' && state->buffer_len > 0 && state->buffer[0] == '-') return;

    memmove(
        &state->buffer[state->cursor + 1],
        &state->buffer[state->cursor],
        state->buffer_len - state->cursor + 1
    );
    state->buffer[state->cursor] = chr;
    state->cursor++;
    state->buffer_len++;
    tui_widget_input_number_apply_buffer(widget);
}

private void tui_widget_input_number_step(Widget *widget, int direction){
    WidgetInputNumberData  *data  = widget->data;
    WidgetInputNumberState *state = widget->state;
    int value = *data->storage;
    value += direction * data->step;
    value = clamp(value, data->min_value, data->max_value);
    *data->storage = value;
    tui_widget_input_number_sync_buffer(state, value);
}

private void tui_widget_input_number_begin_edit(Widget *widget){
    WidgetInputNumberData  *data  = widget->data;
    WidgetInputNumberState *state = widget->state;
    state->editing = true;
    tui_widget_input_number_sync_buffer(state, *data->storage);
    state->caret_show = true;
    state->caret_last_shown = get_curr_time();
}

private void tui_widget_input_number_end_edit(Widget *widget, bool commit){
    WidgetInputNumberData  *data  = widget->data;
    WidgetInputNumberState *state = widget->state;
    if(commit){
        tui_widget_input_number_apply_buffer(widget);
    }else{
        tui_widget_input_number_sync_buffer(state, *data->storage);
    }
    state->editing = false;
    state->caret_show = true;
}

private void tui_widget_input_number_render(Widget *widget, Screen *screen, vec2i position){
    WidgetInputNumberData  *data  = widget->data;
    WidgetInputNumberState *state = widget->state;

    if(state->editing){
        double now = get_curr_time();
        if(now - state->caret_last_shown > state->caret_interval){
            state->caret_show = !state->caret_show;
            state->caret_last_shown = now;
        }
    }

    const uint8_t *value_text = state->editing
        ? (const uint8_t *)state->buffer
        : (const uint8_t *)"";

    char value_buffer[32];
    if(!state->editing){
        snprintf(value_buffer, sizeof(value_buffer), "%d", *data->storage);
        value_text = (const uint8_t *)value_buffer;
    }

    size_t suffix_width = (widget->focused || state->editing) ? 3 : 0;
    size_t field_width = widget->size.w - data->label_width - suffix_width;

    screen_format(widget->focused ? BOLD : NORMAL, widget->focused ? COLOR_MAGENTA : COLOR_WHITE, COLOR_BLACK);
    screen_set_utf8_str(screen, position.x, position.y, data->label);

    size_t value_width = utf8_str_display_width(value_text);
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    screen_set_utf8_str(screen, position.x + data->label_width, position.y, value_text);
    for(size_t x = value_width; x < field_width; x++){
        screen_set_char(screen, position.x + (int)data->label_width + (int)x, position.y, ' ');
    }

    if(widget->focused || state->editing){
        screen_format(widget->focused ? BOLD : NORMAL, COLOR_MAGENTA, COLOR_BLACK);
        screen_set_utf8_str(
            screen,
            position.x + data->label_width + (int)field_width,
            position.y,
            u8" ▼▲"
        );
    }

    if(widget->focused && state->editing && state->caret_show){
        int caret_x = position.x + (int)data->label_width + (int)state->cursor;
        if(state->cursor < state->buffer_len){
            screen_format(NORMAL, COLOR_BLACK, COLOR_MAGENTA);
            char caret_chr[2] = {state->buffer[state->cursor], '\0'};
            screen_set_str(screen, caret_x, position.y, caret_chr);
        }else{
            screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
            screen_set_utf8(screen, caret_x, position.y, u8"█");
        }
    }

    screen_format(widget->focused ? BOLD : NORMAL, widget->focused ? COLOR_MAGENTA : COLOR_GRAY, COLOR_BLACK);
    tui_draw_line(
        screen,
        u8"‾",
        (vec2i){
            .x = position.x + (int)data->label_width,
            .y = position.y + 1,
        },
        (vec2i){
            .x = position.x + (int)data->label_width + (int)field_width - 1,
            .y = position.y + 1,
        }
    );
}

private bool tui_widget_input_number_input(Widget *widget, InputEvent input_event){
    WidgetInputNumberState *state = widget->state;

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_F1:
        case KEY_HOME:
            if(!state->editing) break;
            state->cursor = 0;
            break;
        case KEY_F2:
        case KEY_END:
            if(!state->editing) break;
            state->cursor = state->buffer_len;
            break;
        case KEY_LEFT:
            if(!state->editing) break;
            tui_widget_input_number_move_cursor(widget, -1);
            break;
        case KEY_RIGHT:
            if(!state->editing) break;
            tui_widget_input_number_move_cursor(widget, +1);
            break;
        case KEY_BACKSPACE:
            if(!state->editing) break;
            tui_widget_input_number_delete_before_cursor(widget);
            break;
        case KEY_DELETE:
            if(!state->editing) break;
            tui_widget_input_number_delete_at_cursor(widget);
            break;
        case KEY_UP:
            if(!state->editing) break;
            tui_widget_input_number_step(widget, +1);
            return true;
        case KEY_DOWN:
            if(!state->editing) break;
            tui_widget_input_number_step(widget, -1);
            return true;
        case KEY_ENTER:
            if(state->editing){
                tui_widget_input_number_end_edit(widget, true);
            }else{
                tui_widget_input_number_begin_edit(widget);
            }
            return true;
        case KEY_ESCAPE:
            if(!state->editing) break;
            tui_widget_input_number_end_edit(widget, false);
            return true;
        case KEY_NONE:
        default:
            if(!state->editing) break;
            if(input_event.key_event.unicode == 0) break;
            tui_widget_input_number_insert_char(widget, (char)input_event.key_event.unicode);
            return true;
        }
    case INPUT_NONE:
    default:
        break;
    }

    return state->editing;
}

void tui_widget_input_number_(const char *widget_id, WidgetInputNumberParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->step > 0);
    assert(params->min_value <= params->max_value);

    WidgetInputNumberData *widget_data = (WidgetInputNumberData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetInputNumberData)
    );
    widget_data->label        = params->label ? params->label : (const uint8_t*)"";
    widget_data->storage      = params->storage;
    widget_data->min_value    = params->min_value;
    widget_data->max_value    = params->max_value;
    widget_data->step         = params->step;
    widget_data->label_width  = params->label ? utf8_str_display_width(params->label) : 0;

    char min_buffer[32];
    char max_buffer[32];
    char curr_buffer[32];
    snprintf(min_buffer, sizeof(min_buffer), "%d", params->min_value);
    snprintf(max_buffer, sizeof(max_buffer), "%d", params->max_value);
    snprintf(curr_buffer, sizeof(curr_buffer), "%d", *params->storage);
    widget_data->value_width = max(
        max((int)strlen(min_buffer), (int)strlen(max_buffer)),
        (int)strlen(curr_buffer)
    );

    WidgetInputNumberState *widget_state = (WidgetInputNumberState *)tui_widget_state(
        widget_id,
        sizeof(WidgetInputNumberState)
    );
    if (!widget_state) return;
    if(widget_state->caret_interval == 0.0){
        widget_state->caret_interval = 0.5;
        widget_state->caret_last_shown = get_curr_time();
        widget_state->editing = false;
        widget_state->caret_show = true;
    }

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .state     = widget_state,
        .size.w    = widget_data->label_width + widget_data->value_width + 3,
        .size.h    = 2,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &tui_widget_input_number_input,
        .render    = &tui_widget_input_number_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_INPUT_NUMBER_IMPL
#endif //TUI_WIDGET_INPUT_NUMBER

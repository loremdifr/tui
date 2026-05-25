#ifndef TUI_WIDGET_INPUT_TEXT
#define TUI_WIDGET_INPUT_TEXT

#include "tui_layout.h"
#include "tui_screen.h"
#include <string.h>
// #include <stdlib.h>

void tui_widget_input_text_utf8(
    const char *widget_id,
    const uint8_t *label,
    const uint8_t *placeholder,
    uint8_t *storage,
    size_t capacity
);
void tui_widget_input_text(
    const char *widget_id,
    const char *label,
    const char *placeholder,
    uint8_t *storage,
    size_t capacity
);

#ifdef TUI_WIDGET_INPUT_TEXT_IMPL

typedef struct {
    const uint8_t *label;
    size_t         label_width;
	uint8_t       *storage;
    const uint8_t *placeholder;
    size_t         length;
    size_t         capacity;
} WidgetInputTextData;

typedef struct {
    size_t cursor;
    bool   editing;
} WidgetInputTextState;

private void tui_widget_input_text_render(Widget *widget, Screen *screen, vec2 position){
    WidgetInputTextData  *data     = widget->data;
    WidgetInputTextState *state    = widget->state;
    screen_set_utf8_str(
        screen,
        position.x,
        position.y,
        data->label
    );
    if(data->length == 0){
        //show placeholder
        screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
        screen_set_utf8_str(
            screen,
            position.x + data->label_width,
            position.y,
            data->placeholder
        );
    }else{
        //show text
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
        screen_set_utf8_str(
            screen,
            position.x + data->label_width,
            position.y,
            data->storage
        );
    }

    if(widget->focused){
        screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
        screen_set_utf8(
            screen,
            position.x + widget->size.w - 1,
            position.y,
            u8"󰏫"
        );
    }

    if(widget->focused && state->editing){
        screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
        screen_set_utf8(
            screen,
            position.x + data->label_width + state->cursor,
            position.y,
            u8"█"
        );
    }

    if(widget->focused){
        screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
    }

    tui_draw_line(screen, u8"‾",
        (vec2){
            .x = position.x + data->label_width,
            .y = position.y + 1
        },
        (vec2){
            .x = position.x + widget->size.w - 1,
            .y = position.y + 1
        }
    );
}

private inline void tui_widget_input_text_move_cursor(Widget *widget, int move){
    if(move == 0) return;
    WidgetInputTextData  *widget_data  = widget->data;
    WidgetInputTextState *widget_state = widget->state;
    widget_state->cursor = clamp(
        widget_state->cursor + move,
        0,
        widget_data->length
    );
}

private void tui_widget_input_text_insert(Widget *widget, uint32_t unicode){
    WidgetInputTextData  *widget_data  = widget->data;
    WidgetInputTextState *widget_state = widget->state;

    // we split the string in two at the cursor,
    // move the right side 1 over, and then insert the char
    auto right_side        = widget_data->storage + widget_state->cursor;
    auto right_side_length = strlen((const char *)right_side);
    memcpy(right_side + 1, right_side, right_side_length);
    widget_data->storage[widget_state->cursor] = unicode;

    //also move cursor by 1!
    tui_widget_input_text_move_cursor(widget, +1);
}

private bool tui_widget_input_text_input(Widget *widget, InputEvent input_event){
    WidgetInputTextData  *widget_data  = widget->data;
    WidgetInputTextState *widget_state = widget->state;
    switch (input_event.input_type) {
    case INPUT_KEY:
        auto key = input_event.key_event;
        switch (input_event.key_event.key) {
        case KEY_LEFT:
            tui_widget_input_text_move_cursor(widget, -1);
            break;
        case KEY_RIGHT:
            tui_widget_input_text_move_cursor(widget, +1);
            break;
        case KEY_SPACE:
        case KEY_ENTER:
            widget_state->editing = !widget_state->editing;
            return true; //important! otherwise it bubbles up!
            break;
        case KEY_ESCAPE:
            widget_state->editing = false;
            return true; //important! otherwise it bubbles up!
            break;
        //any way to differentiate key from special key?
        case KEY_NONE:
        default:
            if(!widget_state->editing) break;
            if(key.unicode == 0) break;
            //check if there's space to insert the new character
            if(widget_data->length >= widget_data->capacity){
                //TODO: maybe check length + key length?
                //TODO: might wanna do some sort of error flash or something here
                break;
            }
            //at this point, we're dealing with a printable key!
            tui_widget_input_text_insert(widget, key.unicode);
        break;
        }
    case INPUT_NONE:
    default:
    }

    //TODO: mouse

    //captures input only when editing
    return widget_state->editing;
}

//public
void tui_widget_input_text_utf8(
    const char *widget_id,
    const uint8_t *label,
    const uint8_t *placeholder,
    uint8_t *storage,
    size_t capacity
){

    //widget data
	WidgetInputTextData *widget_data = (WidgetInputTextData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetInputTextData)
	);
    widget_data->label       = label;
    widget_data->label_width = utf8_str_length(label);
    widget_data->storage     = storage;
    widget_data->capacity    = capacity;
    widget_data->placeholder = placeholder;
    widget_data->length      = utf8_str_length(storage);

    //widget state persist across frames
    auto widget_state = (WidgetInputTextState *)tui_widget_state(
        widget_id,
        sizeof(WidgetInputTextState)
    );

    Widget new_widget  = {
        .id        = widget_id,
        .data      = widget_data,
        .state     = widget_state,
        .size.w    = widget_data->label_width + 16,
        .size.h    = 2,
        .focusable = true,
        .input     = &tui_widget_input_text_input,
        .render    = &tui_widget_input_text_render,
    };
    tui_widget_push(new_widget);
}

void tui_widget_input_text(
    const char *widget_id,
    const char *label,
    const char *placeholder,
    uint8_t *storage,
    size_t capacity
){
    tui_widget_input_text_utf8(
        widget_id,
        (const uint8_t*)label,
        (const uint8_t*)placeholder,
        storage,
        capacity
    );
}

#endif //TUI_WIDGET_INPUT_TEXT_IMPL
#endif //TUI_WIDGET_INPUT_TEXT

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

private void tui_widget_input_text_input(Widget *widget, InputEvent input_event){
    // WidgetButtonState *widget_data = widget->data;
    WidgetInputTextState *widget_state = widget->state;
    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_SPACE:
        case KEY_ENTER:
            widget_state->editing = !widget_state->editing;
            break;
        case KEY_ESCAPE:
            //TODO: we need to somehow make the widget consume this input!
            widget_state->editing = false;
            break;
        case KEY_NONE:
        default:
        break;
        }
    case INPUT_NONE:
    default:
    }

    //TODO: mouse
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
    auto state = (WidgetInputTextState *)tui_widget_state(
        widget_id,
        sizeof(WidgetInputTextState)
    );

    Widget new_widget  = {
        .id        = widget_id,
        .data     = widget_data,
        .state = state,
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

#ifndef TUI_WIDGET_INPUT_TEXT
#define TUI_WIDGET_INPUT_TEXT

#include "tui_layout.h"
#include "tui_screen.h"
#include <string.h>
// #include <stdlib.h>

void tui_widget_input_text_utf8(const char *widget_id, const uint8_t *label, uint8_t *storage, size_t capacity);
void tui_widget_input_text(const char *widget_id, const char *label, uint8_t *storage, size_t capacity);

#ifdef TUI_WIDGET_INPUT_TEXT_IMPL

typedef struct {
    const uint8_t *label;
    size_t         label_width;
	uint8_t       *storage;
    const uint8_t *placeholder;
    size_t         length;
    size_t         capacity;
} WidgetInputTextState;

typedef struct {
    size_t cursor;
    bool   editing;
} WidgetInputTextStateI;

private void tui_widget_input_text_render(Widget *widget, Screen *screen, vec2 position){
    WidgetInputTextState  *state     = widget->state;
    WidgetInputTextStateI *internals = widget->internals;
    screen_set_utf8_str(
        screen,
        position.x,
        position.y,
        state->label
    );
    if(state->length == 0){
        //show placeholder
        screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
        screen_set_utf8_str(
            screen,
            position.x + state->label_width,
            position.y,
            state->placeholder
        );
    }else{
        //show text
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
        screen_set_utf8_str(
            screen,
            position.x + state->label_width,
            position.y,
            state->storage
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

    if(widget->focused && internals->editing){
        screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
        screen_set_utf8(
            screen,
            position.x + state->label_width + internals->cursor,
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
            .x = position.x + state->label_width,
            .y = position.y + 1
        },
        (vec2){
            .x = position.x + widget->size.w - 1,
            .y = position.y + 1
        }
    );
}

private void tui_widget_input_text_input(Widget *widget, InputEvent input_event){
    // WidgetButtonState *widget_state = widget->state;
    WidgetInputTextStateI *widget_internals = widget->internals;
    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_SPACE:
        case KEY_ENTER:
            widget_internals->editing = !widget_internals->editing;
            break;
        case KEY_ESCAPE:
            //TODO: we need to somehow make the widget consume this input!
            widget_internals->editing = false;
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
void tui_widget_input_text_utf8(const char *widget_id, const uint8_t *label, uint8_t *storage, size_t capacity){

    //widget state
	WidgetInputTextState *widget_state = (WidgetInputTextState *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetInputTextState)
	);
    widget_state->label       = label;
    widget_state->label_width = utf8_str_length(label);
    widget_state->storage     = storage;
    widget_state->capacity    = capacity;
    widget_state->placeholder = u8"qwe";
    widget_state->length      = utf8_str_length(storage);

    //widget internals persist across frames
    auto internals = (WidgetInputTextStateI *)tui_widget_internals_get(
        widget_id,
        sizeof(WidgetInputTextStateI)
    );

    Widget new_widget  = {
        .id        = widget_id,
        .state     = widget_state,
        .internals = internals,
        .size.w    = widget_state->label_width + 16,
        .size.h    = 2,
        .focusable = true,
        .input     = &tui_widget_input_text_input,
        .render    = &tui_widget_input_text_render,
    };
    tui_widget_push(new_widget);
}

void tui_widget_input_text(const char *widget_id, const char *label, uint8_t *storage, size_t capacity){
    tui_widget_input_text_utf8(widget_id, (const uint8_t*)label, storage, capacity);
}

#endif //TUI_WIDGET_INPUT_TEXT_IMPL
#endif //TUI_WIDGET_INPUT_TEXT

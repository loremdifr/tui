#ifndef TUI_WIDGET_INPUT_TEXT
#define TUI_WIDGET_INPUT_TEXT

#include "tui_layout.h"
#include "tui_platform.h"
#include "tui_screen.h"
#include <string.h>
// #include <stdlib.h>
#include "tui_utils.h"

typedef struct {
    //base widget params
    bool           is_inline;
    // ---
    const uint8_t *label;
    const uint8_t *placeholder;
    uint8_t       *storage;
    size_t         capacity;
} _WidgetInputTextParams;

#define tui_widget_input_text(widget_id, ...) \
        tui_widget_input_text_((widget_id), &(_WidgetInputTextParams){__VA_ARGS__})

void tui_widget_input_text_(const char *widget_id, _WidgetInputTextParams *params);

//text edit
void tui_text_edit_reset_cursor(bool *caret_show, double *caret_last_shown);
void tui_text_edit_move_cursor(size_t *cursor, int move, size_t length);
void tui_text_edit_tick_caret(double *caret_last_shown, bool *caret_show, double interval);
void tui_text_edit_insert(String *str, size_t *cursor, uint32_t unicode);
void tui_text_edit_delete(String *str, size_t cursor);
void tui_text_edit_backspace(String *str, size_t *cursor);


#ifdef TUI_WIDGET_INPUT_TEXT_IMPL

typedef struct {
    const uint8_t *label;
    size_t         label_width;
    size_t         input_width;
    const uint8_t *placeholder;
    String         string;
} _WidgetInputTextData;

typedef struct {
    size_t cursor; // char index
    size_t scroll; // char index
    bool   editing;
    bool   caret_show;
    double caret_interval;
    double caret_last_shown;
} _WidgetInputTextState;


static void _tui_widget_input_text_reset_cursor(Widget *widget){
    _WidgetInputTextState *s = widget->state;
    tui_text_edit_reset_cursor(&s->caret_show, &s->caret_last_shown);
}

static void _tui_widget_input_text_render(Widget *widget, Screen *screen, vec2i position){
    _WidgetInputTextData  *data  = widget->data;
    _WidgetInputTextState *state = widget->state;

    // assert(state->cursor >= 0);
    // assert(state->scroll >= 0);

    //scroll based on cursor location and cursor
    if(state->cursor > data->input_width){
        state->scroll = state->cursor - data->input_width;
    }else{
        //reset scroll!!!!
        state->scroll = 0;
    }

    //caret logic
    tui_text_edit_tick_caret(&state->caret_last_shown, &state->caret_show, state->caret_interval);

    screen_set_utf8_str(
        screen,
        position.x + PADDING,
        position.y,
        data->label
    );
    if(data->string.length == 0){
        //show placeholder
        screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
        auto placeholder_substr = string_from_substr(data->placeholder, 0, data->input_width);
        screen_set_string(
            screen,
            position.x + PADDING + data->label_width,
            position.y,
            &placeholder_substr
        );
    }else{
        //show text
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
        auto text_substr = string_substr(
            &data->string,
            state->scroll,
            clamp(data->string.length, 0, state->scroll + data->input_width) + state->scroll
        );
        screen_set_string(
            screen,
            position.x + PADDING + data->label_width,
            position.y,
            &text_substr
        );
    }

    if(widget->focused && !state->editing){
        const uint8_t *edit_icon = u8"🖉";
        screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
        screen_set_utf8(
            screen,
            position.x + widget->size.w - utf8_str_display_width(edit_icon),
            position.y,
            edit_icon
        );
    }

    //caret
    if(widget->focused && state->editing && state->caret_show){
        int caret_x = position.x + PADDING + data->label_width + (int)(state->cursor - state->scroll);
        if(state->cursor < data->string.length){
            //care is on a character, invert that character color
            screen_format(NORMAL, COLOR_BLACK, COLOR_MAGENTA);
            auto char_substr = string_substr(&data->string, state->cursor, state->cursor + 1);
            screen_set_string(screen, caret_x, position.y, &char_substr);
        }else{
            //simple solid block
            screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
            screen_set_utf8(screen, caret_x, position.y, u8"█");
        }
    }

    if(widget->focused){
        screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
    }

    //underline
    tui_draw_line(screen, u8"‾",
        (vec2i){
            .x = position.x + PADDING + data->label_width,
            .y = position.y + 1
        },
        (vec2i){
            .x = position.x + widget->size.w - 1,
            .y = position.y + 1
        }
    );
}

static inline void _tui_widget_input_text_move_cursor(Widget *widget, int move){
    _WidgetInputTextData  *d = widget->data;
    _WidgetInputTextState *s = widget->state;
    tui_text_edit_move_cursor(&s->cursor, move, d->string.length);
}

static void _tui_widget_input_text_move_word(Widget *widget, int direction){
    _WidgetInputTextData  *widget_data  = widget->data;
    _WidgetInputTextState *widget_state = widget->state;

    if(direction == 0) return;
    if(widget_data->string.length == 0) return;

    size_t curr = widget_state->cursor;

    if(direction < 0){ // Left
        if(curr == 0) return;
        curr--;
        // skip spaces
        while(curr > 0){
            auto curr_char = string_byte_pos_from_index(&widget_data->string, curr);
            if(widget_data->string.data[curr_char] != ' ') break;
            curr--;
        }

        // go to start of word = skip until space
        while(curr > 0){
            auto next_char = string_byte_pos_from_index(&widget_data->string, curr-1);
            if(widget_data->string.data[next_char] == ' ') break;
            curr--;
        }

    }else{ //right
        if(curr >= widget_data->string.length) return;

        // skip curr word = skip until space
        while(curr < widget_data->string.length){
            auto next_char = string_byte_pos_from_index(&widget_data->string, curr+1);
            if(widget_data->string.data[next_char] == ' ') break;
            curr++;
        }
    }

    widget_state->cursor = curr;
}

static void _tui_widget_input_text_insert(Widget *widget, uint32_t unicode){
    _WidgetInputTextData  *d = widget->data;
    _WidgetInputTextState *s = widget->state;
    tui_text_edit_insert(&d->string, &s->cursor, unicode);
}

static void _tui_widget_input_text_delete(Widget *widget){
    _WidgetInputTextData  *d = widget->data;
    _WidgetInputTextState *s = widget->state;
    tui_text_edit_delete(&d->string, s->cursor);
}

static void _tui_widget_input_text_backspace(Widget *widget){
    _WidgetInputTextData  *d = widget->data;
    _WidgetInputTextState *s = widget->state;
    tui_text_edit_backspace(&d->string, &s->cursor);
}

static bool _tui_widget_input_text_input(Widget *widget, InputEvent input_event){
    _WidgetInputTextData  *widget_data  = widget->data;
    _WidgetInputTextState *widget_state = widget->state;
    switch (input_event.input_type) {
    case INPUT_KEY:
        auto key = input_event.key_event;
        switch (input_event.key_event.key) {
        case KEY_F1:
        case KEY_HOME:
            if(!widget_state->editing) break;
            _tui_widget_input_text_reset_cursor(widget);
            widget_state->cursor = 0;
            break;
        case KEY_F2:
        case KEY_END:
            if(!widget_state->editing) break;
            _tui_widget_input_text_reset_cursor(widget);
            widget_state->cursor = widget_data->string.length;
            break;
        case KEY_LEFT:
            if(!widget_state->editing) break;
            _tui_widget_input_text_reset_cursor(widget);
            if(key.ctrl){
                _tui_widget_input_text_move_word(widget, -1);
            }else{
                _tui_widget_input_text_move_cursor(widget, -1);
            }
            break;
        case KEY_RIGHT:
            if(!widget_state->editing) break;
            _tui_widget_input_text_reset_cursor(widget);
            if(key.ctrl){
                _tui_widget_input_text_move_word(widget, +1);
            }else{
                _tui_widget_input_text_move_cursor(widget, +1);
            }
            break;
        case KEY_BACKSPACE:
            if(!widget_state->editing) break;
            _tui_widget_input_text_reset_cursor(widget);
            _tui_widget_input_text_backspace(widget);
            break;
        case KEY_DELETE:
            if(!widget_state->editing) break;
            _tui_widget_input_text_reset_cursor(widget);
            _tui_widget_input_text_delete(widget);
            break;
        case KEY_UP:
        case KEY_DOWN:
        case KEY_TAB:
        case KEY_BACKTAB:
            // don't consume, let bubble to hotkeys for widget/panel navigation
            return false;
        case KEY_ENTER:
            widget_state->editing = !widget_state->editing;
            if(widget_state->editing) _tui_widget_input_text_reset_cursor(widget);
            return true; //important! otherwise it bubbles up!
            break;
        case KEY_ESCAPE:
            if(!widget_state->editing) return false;
            widget_state->editing = false;
            return true;
            break;
        //any way to differentiate key from special key?
        case KEY_NONE:
        default:
            if(!widget_state->editing) break;
            if(key.unicode == 0) break;
            //check if there's space to insert the new character
            if(widget_data->string.bytes + utf8_char_length((uint8_t)key.unicode) >= widget_data->string.capacity){
                //TODO: might wanna do some sort of error flash or something here
                break;
            }
            //at this point, we're dealing with a printable key!
            _tui_widget_input_text_insert(widget, key.unicode);
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
void tui_widget_input_text_(const char *widget_id, _WidgetInputTextParams *params){
    Panel *panel = tui_get_panel_building();

    //widget data
	_WidgetInputTextData *widget_data = (_WidgetInputTextData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(_WidgetInputTextData)
	);
    widget_data->label       = params->label;
    widget_data->label_width = utf8_str_display_width(params->label);
    widget_data->placeholder = params->placeholder;
    widget_data->string      = string_from(params->storage, params->capacity);

    // change size based on placeholder
    size_t placeholder_len = utf8_str_display_width(params->placeholder);
    widget_data->input_width = max(16, (int)placeholder_len + 1);

    //widget state persist across frames
    auto widget_state = (_WidgetInputTextState *)tui_widget_state(
        widget_id,
        sizeof(_WidgetInputTextState)
    );
    if(widget_state->caret_interval == 0.0){
        widget_state->caret_interval = 0.5;
        widget_state->cursor = widget_data->string.length; // start at the end
    }
    // in case the data has changed from outside inbetween frames we need to make sure
    // that the cursor is not pointing "outside" the string
    widget_state->cursor = clamp(widget_state->cursor, 0, widget_data->string.length);

    int total_width = widget_data->label_width + widget_data->input_width + PADDING * 2;

    // ensure input fits inside the panel
    int panel_width = panel->inner_rect.size.w;
    if(total_width > panel_width) total_width = panel_width;
    widget_data->input_width = max(1, total_width - (int)widget_data->label_width);

    Widget new_widget  = {
        .id        = widget_id,
        .data      = widget_data,
        .state     = widget_state,
        .size.w    = total_width,
        .size.h    = 2,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &_tui_widget_input_text_input,
        .render    = &_tui_widget_input_text_render,
    };
    tui_widget_push(new_widget);
}

//text edit helpers
void tui_text_edit_reset_cursor(bool *caret_show, double *caret_last_shown){
    *caret_show = true;
    *caret_last_shown = get_curr_time();
}

void tui_text_edit_move_cursor(size_t *cursor, int move, size_t length){
    if(move == 0) return;
    *cursor = (size_t)clamp((int)*cursor + move, 0, (int)length);
}

void tui_text_edit_tick_caret(double *caret_last_shown, bool *caret_show, double interval){
    double now = get_curr_time();
    if(now - *caret_last_shown > interval){
        *caret_show = !*caret_show;
        *caret_last_shown = now;
    }
}

void tui_text_edit_insert(String *str, size_t *cursor, uint32_t unicode){
    string_insert_at(str, *cursor, unicode);
    tui_text_edit_move_cursor(cursor, +1, str->length);
}

void tui_text_edit_delete(String *str, size_t cursor){
    string_delete_at(str, cursor);
}

void tui_text_edit_backspace(String *str, size_t *cursor){
    if(*cursor == 0) return;
    tui_text_edit_move_cursor(cursor, -1, str->length);
    tui_text_edit_delete(str, *cursor);
}

#endif //TUI_WIDGET_INPUT_TEXT_IMPL
#endif //TUI_WIDGET_INPUT_TEXT

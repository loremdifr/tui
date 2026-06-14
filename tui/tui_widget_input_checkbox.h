#ifndef TUI_WIDGET_INPUT_CHECKBOX
#define TUI_WIDGET_INPUT_CHECKBOX

#include "tui_layout.h"

typedef struct {
    bool            is_inline;
    const uint8_t  *label;
    bool           *storage;
    FunctionPointer on_toggle;
} WidgetInputCheckboxParams;

#define tui_widget_input_checkbox(widget_id, ...) \
        tui_widget_input_checkbox_((widget_id), &(WidgetInputCheckboxParams){__VA_ARGS__})

void tui_widget_input_checkbox_(const char *widget_id, WidgetInputCheckboxParams *params);

#ifdef TUI_WIDGET_INPUT_CHECKBOX_IMPL

static size_t TUI_WIDGET_INPUT_CHECKBOX_MAX_WIDTH = 0; //this is used to line up all checkboxes

typedef struct {
    const uint8_t  *label;
    bool           *storage;
    FunctionPointer on_toggle;
} WidgetInputCheckboxData;

static void _tui_widget_input_checkbox_render(Widget *widget, Screen *screen, vec2i position){
    WidgetInputCheckboxData *widget_data = widget->data;

    static const uint8_t CHECKBOX_ON[4]  = u8"[x]";
    static const uint8_t CHECKBOX_OFF[4] = u8"[ ]";
    const uint8_t *box = *widget_data->storage ? CHECKBOX_ON : CHECKBOX_OFF;
    size_t box_width   = 3; //utf8_str_display_width(box);
    size_t label_width = utf8_str_display_width(widget_data->label);
    int rendered_width = (int)(box_width + 1 + label_width);

    if(widget->focused){
        screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }

    screen_set_utf8_str(screen, position.x + PADDING, position.y, box);
    screen_set_utf8_str(screen,
        position.x + PADDING + (int)box_width + 1,
        position.y,
        widget_data->label
    );
}

static bool _tui_widget_input_checkbox_input(Widget *widget, InputEvent input_event){
    WidgetInputCheckboxData *widget_data = widget->data;

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_SPACE:
        case KEY_ENTER:
            *widget_data->storage = !(*widget_data->storage);
            if(widget_data->on_toggle != nullptr){
                widget_data->on_toggle();
            }
            break;
        case KEY_NONE:
        default:
            break;
        }
    case INPUT_NONE:
    default:
        break;
    }

    return false;
}

void tui_widget_input_checkbox_(const char *widget_id, WidgetInputCheckboxParams *params){
    WidgetInputCheckboxData *widget_data = (WidgetInputCheckboxData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetInputCheckboxData)
    );
    widget_data->label     = params->label;
    widget_data->storage   = params->storage;
    widget_data->on_toggle = params->on_toggle;

    size_t label_width = utf8_str_display_width(params->label);
    size_t widget_width = 3 + 1 + label_width + PADDING * 2;

    //store the biggest checkbox so that we can pad it and they will all line up
    if(widget_width > TUI_WIDGET_INPUT_CHECKBOX_MAX_WIDTH){
        TUI_WIDGET_INPUT_CHECKBOX_MAX_WIDTH = widget_width;
    }

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .size.w    = TUI_WIDGET_INPUT_CHECKBOX_MAX_WIDTH,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &_tui_widget_input_checkbox_input,
        .render    = &_tui_widget_input_checkbox_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_INPUT_CHECKBOX_IMPL
#endif //TUI_WIDGET_INPUT_CHECKBOX

#ifndef TUI_WIDGET_INPUT_RADIO
#define TUI_WIDGET_INPUT_RADIO

#include "tui_layout.h"

typedef struct {
    bool           is_inline;
    const uint8_t *label;
    void          *storage;
    size_t         storage_size;
    const void    *value;
    FunctionPointer on_select;
} _WidgetInputRadioParams;

#define tui_widget_input_radio(widget_id, ...) \
        tui_widget_input_radio_((widget_id), &(_WidgetInputRadioParams){__VA_ARGS__})

void tui_widget_input_radio_(const char *widget_id, _WidgetInputRadioParams *params);

#ifdef TUI_WIDGET_INPUT_RADIO_IMPL

static size_t TUI_WIDGET_INPUT_RADIO_MAX_WIDTH = 0; //this is used to line up all checkboxes

typedef struct {
    const uint8_t  *label;
    void           *storage;
    size_t          storage_size;
    const void     *value;
    FunctionPointer on_select;
} _WidgetInputRadioData;

static bool _tui_widget_input_radio_is_selected(_WidgetInputRadioData *widget_data){
    assert(widget_data != nullptr);
    assert(widget_data->storage != nullptr);
    assert(widget_data->value != nullptr);
    assert(widget_data->storage_size > 0);
    //TODO: is this wise...?
    return memcmp(
        widget_data->storage,
        widget_data->value,
        widget_data->storage_size
    ) == 0;
}

static void _tui_widget_input_radio_render(Widget *widget, Screen *screen, vec2i position){
    _WidgetInputRadioData *widget_data = widget->data;
    bool is_selected = _tui_widget_input_radio_is_selected(widget_data);

    static const uint8_t RADIO_ON[4]  = u8"◉";
    static const uint8_t RADIO_OFF[4] = u8"○";
    const uint8_t *radio = is_selected ? RADIO_ON : RADIO_OFF;
    size_t radio_width   = 1; //utf8_str_display_width(radio);
    size_t label_width = utf8_str_display_width(widget_data->label);
    int rendered_width = (int)(radio_width + 1 + label_width);

    if(widget->focused){
        screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
        screen_set_utf8_str(screen, position.x + PADDING, position.y, radio);
    }else if (is_selected){
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_DARK_WHITE, COLOR_BLACK);
    }

    screen_set_utf8_str(screen, position.x + PADDING, position.y, radio);
    screen_set_utf8_str(screen,
        position.x + PADDING + (int)radio_width + 1,
        position.y,
        widget_data->label
    );
}

static bool _tui_widget_input_radio_input(Widget *widget, InputEvent input_event){
    _WidgetInputRadioData *widget_data = widget->data;

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_SPACE:
        case KEY_ENTER:
            memcpy(
                widget_data->storage,
                widget_data->value,
                widget_data->storage_size
            );
            if(widget_data->on_select != nullptr){
                widget_data->on_select();
            }
            return true;
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

void tui_widget_input_radio_(const char *widget_id, _WidgetInputRadioParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->value != nullptr);
    assert(params->storage_size > 0);

    _WidgetInputRadioData *widget_data = (_WidgetInputRadioData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(_WidgetInputRadioData)
    );
    widget_data->label        = params->label;
    widget_data->storage      = params->storage;
    widget_data->storage_size = params->storage_size;
    widget_data->value        = params->value;
    widget_data->on_select    = params->on_select;

    size_t label_width = utf8_str_display_width(params->label);
    size_t widget_width = PADDING * 2 + 2 + label_width;

    //store the widest radio btn so that we can pad it and they will all line up
    if(widget_width > TUI_WIDGET_INPUT_RADIO_MAX_WIDTH){
        TUI_WIDGET_INPUT_RADIO_MAX_WIDTH = widget_width;
    }

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .size.w    = TUI_WIDGET_INPUT_RADIO_MAX_WIDTH,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &_tui_widget_input_radio_input,
        .render    = &_tui_widget_input_radio_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_INPUT_RADIO_IMPL
#endif //TUI_WIDGET_INPUT_RADIO

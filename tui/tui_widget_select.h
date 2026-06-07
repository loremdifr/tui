#ifndef TUI_WIDGET_SELECT
#define TUI_WIDGET_SELECT

//TODO: finish implementing. this is a WIP!

#include "tui_layout.h"

typedef struct {
    bool           is_inline;
    const uint8_t *label;
    const uint8_t *value;
} WidgetSelectParams;

#define tui_widget_select(widget_id, ...) \
        tui_widget_select_((widget_id), &(WidgetSelectParams){__VA_ARGS__})

void tui_widget_select_(const char *widget_id, WidgetSelectParams *params);

#ifdef TUI_WIDGET_SELECT_IMPL

typedef struct {
    const uint8_t *label;
    const uint8_t *value;
    size_t         label_width;
    size_t         value_width;
} WidgetSelectData;

typedef struct {
    bool overlay_open;
} WidgetSelectState;

private void do_nothing(void){}

private void tui_widget_select_render(Widget *widget, Screen *screen, vec2i position){
    WidgetSelectData *data = widget->data;

    if(widget->focused){
        screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }

    screen_set_utf8_str(screen, position.x, position.y, data->label);
    screen_set_utf8_str(screen, position.x + data->label_width, position.y, data->value);
    screen_set_utf8(screen, position.x + widget->size.w - 1, position.y, u8"▼");
}

private void tui_widget_select_overlay(Widget */*widget*/){
    tui_widget_label(u8"Dummy select overlay");
    tui_widget_button("DUMMY_SELECT_OPTION_1",
        .label=u8"Option A",
        .on_click=&do_nothing
    );
    tui_widget_button("DUMMY_SELECT_OPTION_2",
        .label=u8"Option B",
        .on_click=&do_nothing
    );
    tui_widget_label(u8"Overlay size comes from these widgets.");
}

private bool tui_widget_select_input(Widget *widget, InputEvent input_event){
    WidgetSelectState *state = widget->state;

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_ENTER:
            state->overlay_open = !state->overlay_open;
            return true;
        case KEY_ESCAPE:
            if(!state->overlay_open) break;
            state->overlay_open = false;
            return true;
        case KEY_NONE:
        default:
            break;
        }
    case INPUT_NONE:
    default:
        break;
    }

    //TODO: mouse

    return state->overlay_open;
}

void tui_widget_select_(const char *widget_id, WidgetSelectParams *params){
    WidgetSelectData *widget_data = (WidgetSelectData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetSelectData)
    );
    widget_data->label       = params->label;
    widget_data->value       = params->value;
    widget_data->label_width = utf8_str_length(params->label);
    widget_data->value_width = utf8_str_length(params->value);

    WidgetSelectState *widget_state = (WidgetSelectState *)tui_widget_state(
        widget_id,
        sizeof(WidgetSelectState)
    );

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .state     = widget_state,
        .size.w    = widget_data->label_width + widget_data->value_width + PADDING + 1,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &tui_widget_select_input,
        .render    = &tui_widget_select_render,
        .build_overlay_panel  = widget_state->overlay_open
            ? &tui_widget_select_overlay
            : nullptr,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_SELECT_IMPL
#endif //TUI_WIDGET_SELECT

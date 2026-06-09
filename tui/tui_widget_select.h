#ifndef TUI_WIDGET_SELECT
#define TUI_WIDGET_SELECT

#include "tui_layout.h"

typedef struct {
    size_t         value;
    const uint8_t *label;
} WidgetSelectOption;

typedef struct {
    WidgetSelectOption *values;
    size_t count;
} WidgetSelectOptions;

typedef struct {
    bool                 is_inline;
    const uint8_t       *label;
    size_t              *storage;
    WidgetSelectOptions   options;
} WidgetSelectParams;

#define tui_widget_select(widget_id, ...) \
        tui_widget_select_((widget_id), &(WidgetSelectParams){__VA_ARGS__})

void tui_widget_select_(const char *widget_id, WidgetSelectParams *params);

#ifdef TUI_WIDGET_SELECT_IMPL

typedef struct {
    size_t              *storage;
    const uint8_t       *label;
    size_t               label_width;
    WidgetSelectOptions   options;
} WidgetSelectData;

private const uint8_t *tui_widget_select_label(WidgetSelectData *data){
    if(data == nullptr) return u8"";
    if(data->options.count == 0) return u8"";
    size_t selected_index = 0;
    if(data->storage != nullptr){
        for(size_t i = 0; i < data->options.count; i++){
            if(data->options.values[i].value == *data->storage){
                selected_index = i;
                break;
            }
        }
    }
    return data->options.values[selected_index].label;
}

private void tui_widget_select_render(Widget *widget, Screen *screen, vec2i position){
    WidgetSelectData  *data = widget->data;

    auto selected_label   = tui_widget_select_label(data);
    size_t selected_width = utf8_str_display_width(selected_label);

    if(widget->focused){
        screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }

    screen_set_utf8_str(screen, position.x, position.y, data->label);
    screen_set_utf8_str(screen, position.x + (int)data->label_width, position.y, selected_label);
    screen_set_utf8_str(
        screen,
        position.x + (int)data->label_width + selected_width,
        position.y,
        u8" ▼"
    );

}

private bool tui_widget_select_input(Widget *widget, InputEvent input_event){
    WidgetSelectData *data = widget->data;

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_ENTER:
            if(tui_widget_overlay_is_open()){
                tui_widget_overlay_close();
            }else{
                tui_widget_overlay_open();
            }
            return true; //important! we capture the event!
        case KEY_ESCAPE:
            if(!tui_widget_overlay_is_open()) break;
            tui_widget_overlay_close();
            return true; //important! we capture the event!
        case KEY_NONE:
        default:
            break;
        }
    case INPUT_NONE:
    default:
        break;
    }

    return tui_widget_overlay_is_open();
}

private void tui_widget_select_overlay(Widget *widget){
    WidgetSelectData *data = widget->data;

    tui_layer_begin(LAYER_WIDGETS_OVERLAY_DO_NOT_USE, LAYOUT_SINGLE_PANEL);
        tui_panel_begin(SLOT_MAIN);
            for(size_t i = 0; i < data->options.count; i++){
                char *option_id = tui_create_widget_id();
                tui_widget_input_radio(
                    option_id,
                    .label        = data->options.values[i].label,
                    .storage      = data->storage,
                    .storage_size = sizeof(size_t),
                    .value        = &data->options.values[i].value,
                    .on_select    = &tui_widget_overlay_close
                );
            }
        tui_panel_end();
    tui_layer_end();
}

void tui_widget_select_(const char *widget_id, WidgetSelectParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->options.count > 0);

    WidgetSelectData *widget_data = (WidgetSelectData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetSelectData)
    );
    widget_data->storage        = params->storage;
    widget_data->label          = params->label;
    widget_data->label_width    = utf8_str_display_width(params->label);
    widget_data->options        = params->options;

    auto selected_label   = tui_widget_select_label(widget_data);
    size_t selected_width = utf8_str_display_width(selected_label);

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .size.w    = widget_data->label_width + selected_width + 2,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &tui_widget_select_input,
        .render    = &tui_widget_select_render,
        .overlay   = &tui_widget_select_overlay,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_SELECT_IMPL
#endif //TUI_WIDGET_SELECT

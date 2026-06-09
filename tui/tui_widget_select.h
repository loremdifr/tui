#ifndef TUI_WIDGET_SELECT
#define TUI_WIDGET_SELECT

#include "tui_layout.h"

typedef struct {
    size_t         value;
    const uint8_t *label;
} WidgetSelectOption;

typedef struct {
    bool                 is_inline;
    const uint8_t       *label;
    size_t              *storage;
    WidgetSelectOption  *options;
    size_t               options_count;
} WidgetSelectParams;

#define tui_widget_select(widget_id, ...) \
        tui_widget_select_((widget_id), &(WidgetSelectParams){__VA_ARGS__})

void tui_widget_select_(const char *widget_id, WidgetSelectParams *params);

#ifdef TUI_WIDGET_SELECT_IMPL

typedef struct {
    const uint8_t       *label;
    size_t              *storage;
    WidgetSelectOption  *options;
    size_t               options_count;
    size_t               label_width;
    size_t               option_width;
} WidgetSelectData;

private size_t tui_widget_select_selected_index(WidgetSelectData *data){
    if(data->storage == nullptr) return 0;
    for(size_t i = 0; i < data->options_count; i++){
        if(data->options[i].value == *data->storage){
            return i;
        }
    }
    return 0;
}

private const uint8_t *tui_widget_select_option_label(WidgetSelectData *data, size_t index){
    if(data == nullptr) return u8"";
    if(index >= data->options_count) return u8"";
    return data->options[index].label;
}

private void tui_widget_select_render(Widget *widget, Screen *screen, vec2i position){
    WidgetSelectData  *data = widget->data;

    size_t selected_index         = tui_widget_select_selected_index(data);
    const uint8_t *selected_label = tui_widget_select_option_label(data, selected_index);
    size_t selected_width         = utf8_str_display_width(selected_label);

    if(widget->focused){
        screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }

    screen_set_utf8_str(screen, position.x, position.y, data->label);
    screen_set_utf8_str(screen, position.x + (int)data->label_width, position.y, selected_label);
    screen_set_utf8_str(
        screen,
        position.x + (int)data->label_width + (int)data->option_width,
        position.y,
        u8" ▼"
    );

}

private bool tui_widget_select_input(Widget *widget, InputEvent input_event){
    WidgetSelectData  *data = widget->data;

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
    WidgetSelectData  *data  = widget->data;

    tui_layer_begin(LAYER_WIDGETS_OVERLAY_DO_NOT_USE, LAYOUT_SINGLE_PANEL);
        tui_panel_begin(SLOT_MAIN);
            for(size_t i = 0; i < data->options_count; i++){
                char *option_id = tui_create_widget_id();
                tui_widget_input_radio(
                    option_id,
                    .label        = data->options[i].label,
                    .storage      = data->storage,
                    .storage_size = sizeof(size_t),
                    .value        = &data->options[i].value,
                    .on_select    = &tui_widget_overlay_close
                );
            }
        tui_panel_end();
    tui_layer_end();
}

void tui_widget_select_(const char *widget_id, WidgetSelectParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->options != nullptr);
    assert(params->options_count > 0);

    WidgetSelectData *widget_data = (WidgetSelectData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetSelectData)
    );
    widget_data->label         = params->label;
    widget_data->storage       = params->storage;
    widget_data->options       = params->options;
    widget_data->options_count = params->options_count;
    widget_data->label_width   = utf8_str_display_width(params->label);
    widget_data->option_width  = 0;

    for(size_t i = 0; i < params->options_count; i++){
        size_t option_width = utf8_str_display_width(params->options[i].label);
        if(option_width > widget_data->option_width){
            widget_data->option_width = option_width;
        }
    }

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .size.w    = widget_data->label_width + widget_data->option_width + 2,
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

#ifndef TUI_WIDGET_SELECT_FILTER
#define TUI_WIDGET_SELECT_FILTER

#include "tui_widget_input_text.h"
#include "tui_widget_input_radio.h"
#include "tui_widget_select.h"

typedef WidgetSelectOptions (*WidgetOptionsFunction)(const uint8_t *query);

typedef struct {
    bool                    is_inline;
    const uint8_t          *label;
    size_t                 *storage;
    WidgetOptionsFunction   options_function;
    const uint8_t          *overlay_title;
} WidgetSelectFilterParams;

#define tui_widget_select_filter(widget_id, ...) \
        tui_widget_select_filter_((widget_id), &(WidgetSelectFilterParams){__VA_ARGS__})

void tui_widget_select_filter_(const char *widget_id, WidgetSelectFilterParams *params);

#ifdef TUI_WIDGET_SELECT_FILTER_IMPL

typedef struct {
    size_t                *storage;
    const uint8_t         *label;
    size_t                 label_width;
    WidgetOptionsFunction  options_function;
} WidgetSelectFilterData;

typedef struct {
    uint8_t query[128];
    char    query_widget_id[64];
    //we have to store the options in the state as a form of cache
    //so that we don't compute them each frame, and only when the query changes!
    WidgetSelectOptions options;
} WidgetSelectFilterState;

private void tui_widget_select_filter_overlay(Widget *widget){
    WidgetSelectFilterState *state = widget->state;
    WidgetSelectFilterData  *data = widget->data;

    tui_layer_begin(LAYER_WIDGETS_OVERLAY_DO_NOT_USE, LAYOUT_WITH_HEADER);
        tui_panel_begin(SLOT_TOP);
            tui_widget_input_text(
                state->query_widget_id,
                .label=u8"Search: ",
                .placeholder=u8"Type to filter...",
                .storage=state->query,
                .capacity=sizeof(state->query),
            );
        tui_panel_end();
        // direct LAYOUT_STATE access: auto-focus text input and enter editing mode
        {
            PageLayer *overlay_layer = &LAYOUT_STATE.layers[LAYER_WIDGETS_OVERLAY_DO_NOT_USE];
            overlay_layer->widget_focused[0] = state->query_widget_id;
            overlay_layer->panel_focused = 0;
            Panel *top_panel = &overlay_layer->panels[0];
            WidgetInputTextState *text_state = top_panel->widgets[top_panel->widget_count - 1].state;
            text_state->editing = true;
            text_state->caret_show = true;
            text_state->caret_last_shown = get_curr_time();
        }
        tui_panel_begin(SLOT_MAIN);
            if(state->options.count > 0){
                for(size_t i = 0; i < state->options.count; i++){
                    char *option_id = tui_create_widget_id();
                    tui_widget_input_radio(
                        option_id,
                        .label        = state->options.values[i].label,
                        .storage      = data->storage,
                        .storage_size = sizeof(size_t),
                        .value        = &state->options.values[i].value,
                        .on_select    = &tui_widget_overlay_close
                    );
                }
            }else{
                tui_widget_label("No matches."); //TODO: move to i18n
            }
        tui_panel_end();
    tui_layer_end();

}


private const uint8_t *tui_widget_select_filter_label(WidgetSelectFilterData *data, WidgetSelectFilterState  *state){
    if(state == nullptr) return u8"";
    if(state->options.count == 0) return u8"";
    size_t selected_index = 0;
    if(data->storage != nullptr){
        for(size_t i = 0; i < state->options.count; i++){
            if(state->options.values[i].value == *data->storage){
                selected_index = i;
                break;
            }
        }
    }
    return state->options.values[selected_index].label;
}

private void tui_widget_select_filter_render(Widget *widget, Screen *screen, vec2i position){
    WidgetSelectFilterData  *data  = widget->data;
    WidgetSelectFilterState *state = widget->state;

    auto selected_label  = tui_widget_select_filter_label(data, state);
    size_t selected_width = utf8_str_display_width(selected_label);

    if(widget->focused){
        screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }

    screen_set_utf8_str(screen, position.x + PADDING, position.y, data->label);
    screen_set_utf8_str(screen, position.x + PADDING + (int)data->label_width, position.y, selected_label);
    screen_set_utf8_str(
        screen,
        position.x + PADDING + (int)data->label_width + selected_width,
        position.y,
        u8" ▼"
    );

}

private bool tui_widget_select_filter_input(Widget *widget, InputEvent input_event){
    WidgetSelectFilterState *state = widget->state;

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_ENTER:
            if(tui_widget_overlay_is_open()){
                tui_widget_overlay_close();
            }else{
                tui_widget_overlay_open();
                state->query[0] = '\0';
            }
            return true;
        case KEY_ESCAPE:
            if(!tui_widget_overlay_is_open()) break;
            tui_widget_overlay_close();
            return true;
        case KEY_UP:
        case KEY_DOWN:
            // let these bubble to hotkeys for cross-panel navigation while overlay is open
            if(!tui_widget_overlay_is_open()) break;
            return false;
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
void tui_widget_select_filter_(const char *widget_id, WidgetSelectFilterParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);

    WidgetSelectFilterData *widget_data = (WidgetSelectFilterData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetSelectFilterData)
    );
    widget_data->storage          = params->storage;
    widget_data->label            = params->label;
    widget_data->label_width      = utf8_str_display_width(params->label);
    widget_data->options_function = params->options_function;

    //widget state persist across frames
    auto widget_state = (WidgetSelectFilterState *)tui_widget_state(
        widget_id,
        sizeof(WidgetSelectFilterState)
    );

    auto query_widget_id = tui_create_widget_id();
    strncpy(widget_state->query_widget_id, query_widget_id, 64);
    if(params->options_function != nullptr){
        widget_state->options = params->options_function(widget_state->query);
    }

    auto selected_label   = tui_widget_select_filter_label(widget_data, widget_state);
    size_t selected_width = utf8_str_display_width(selected_label);

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .state     = widget_state,
        .size.w    = widget_data->label_width + selected_width + 2 + PADDING * 2,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &tui_widget_select_filter_input,
        .render    = &tui_widget_select_filter_render,
        .overlay   = &tui_widget_select_filter_overlay,
        .overlay_title = params->overlay_title ? params->overlay_title : params->label,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_SELECT_FILTER_IMPL
#endif //TUI_WIDGET_SELECT_FILTER

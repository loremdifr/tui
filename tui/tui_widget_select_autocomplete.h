#ifndef TUI_WIDGET_SELECT_AUTOCOMPLETE
#define TUI_WIDGET_SELECT_AUTOCOMPLETE

#include "tui_widget_input_text.h"
#include "tui_widget_input_radio.h"
#include "tui_widget_select.h"

typedef size_t (*WidgetSuggestionsFunction)(
    const uint8_t *query,
    WidgetSelectOption *options,
    size_t options_capacity
);

typedef struct {
    bool                       is_inline;
    const uint8_t             *label;
    size_t                    *storage;
    WidgetSuggestionsFunction  get_suggestions;
    size_t                     options_capacity;
} WidgetSelectAutocompleteParams;

#define tui_widget_select_autocomplete(widget_id, ...) \
        tui_widget_select_autocomplete_((widget_id), &(WidgetSelectAutocompleteParams){__VA_ARGS__})

void tui_widget_select_autocomplete_(const char *widget_id, WidgetSelectAutocompleteParams *params);

#ifdef TUI_WIDGET_SELECT_AUTOCOMPLETE_IMPL

#define TUI_WIDGET_SELECT_AUTOCOMPLETE_QUERY_MAX   128
#define TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX 8

typedef struct {
    const uint8_t             *label;
    size_t                    *storage;
    WidgetSuggestionsFunction  get_suggestions;
    size_t                     label_width;
    size_t                     option_width;
    size_t                     options_capacity;
    WidgetSelectOption         suggestions[TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX];
} WidgetSelectAutocompleteData;

typedef struct {
    bool   overlay_open;
    bool   focus_query_on_open;
    uint8_t query[TUI_WIDGET_SELECT_AUTOCOMPLETE_QUERY_MAX];
    char   query_widget_id[64];
} WidgetSelectAutocompleteState;

private const uint8_t *tui_widget_select_autocomplete_selected_label(WidgetSelectAutocompleteData *data){
    if(data == nullptr || data->storage == nullptr) return u8"";

    // WidgetSelectOption options[TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX] = {};
    // size_t options_capacity = data->options_capacity;
    // if(options_capacity > TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX){
    //     options_capacity = TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX;
    // }
    // size_t options_count = data->suggestions(u8"", options, options_capacity);
    // if(options_count > TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX){
    //     options_count = TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX;
    // }

    // for(size_t i = 0; i < options_count; i++){
    //     if(options[i].value == *data->storage){
    //         return options[i].label;
    //     }
    // }
    // return options_count > 0 ? options[0].label : u8"";
    return u8"";
}

private void tui_widget_select_autocomplete_overlay(Widget *widget){
    WidgetSelectAutocompleteData  *data  = widget->data;
    WidgetSelectAutocompleteState *state = widget->state;

    if(!state->overlay_open) return;

    tui_layer_begin(LAYER_WIDGETS_OVERLAY_DO_NOT_USE, LAYOUT_WITH_HEADER);
        tui_panel_begin(SLOT_TOP);
            tui_widget_input_text(
                state->query_widget_id,
                .label=u8"Search: ",
                .placeholder=u8"Type to filter...",
                .storage=state->query,
                .capacity=sizeof(state->query)
            );
        tui_panel_end();
        tui_panel_begin(SLOT_MAIN);
            // for(size_t i = 0; i < data->options_count; i++){
            //     char *option_id = tui_create_widget_id();
            //     tui_widget_input_radio(
            //         option_id,
            //         .label        = data->options[i].label,
            //         .storage      = data->storage,
            //         .storage_size = sizeof(size_t),
            //         .value        = &data->options[i].value,
            //         //TODO:
            //         // .on_select    = &tui_widget_select_close
            //     );
            // }
        tui_panel_end();
    tui_layer_end();

}

private void tui_widget_select_autocomplete_render(Widget *widget, Screen *screen, vec2i position){
    WidgetSelectAutocompleteData *data = widget->data;
    const uint8_t *selected_label = tui_widget_select_autocomplete_selected_label(data);
    size_t selected_width = utf8_str_display_width(selected_label);

    if(widget->focused){
        screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
    }else{
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }

    screen_set_utf8_str(screen, position.x, position.y, data->label);
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    screen_set_utf8_str(screen, position.x + (int)data->label_width, position.y, selected_label);

    for(size_t x = selected_width; x < data->option_width; x++){
        screen_set_char(screen, position.x + (int)data->label_width + (int)x, position.y, ' ');
    }

    screen_format(widget->focused ? BOLD : NORMAL, COLOR_MAGENTA, COLOR_BLACK);
    screen_set_utf8_str(
        screen,
        position.x + (int)data->label_width + (int)data->option_width,
        position.y,
        u8" ▼"
    );
}

private bool tui_widget_select_autocomplete_input(Widget *widget, InputEvent input_event){
    WidgetSelectAutocompleteState *state = widget->state;

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_ENTER:
            if(state->overlay_open){
                if(!state->focus_query_on_open){
                    state->overlay_open = false;
                    return true;
                }
            }else{
                state->overlay_open = true;
                state->focus_query_on_open = true;
                state->query[0] = '\0';
            }
            return true;
        case KEY_ESCAPE:
            if(!state->overlay_open) break;
            if(state->focus_query_on_open){
                state->focus_query_on_open = false;
                return true;
            }
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

    return state->overlay_open;
}

void tui_widget_select_autocomplete_(const char *widget_id, WidgetSelectAutocompleteParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->get_suggestions != nullptr);
    assert(params->options_capacity > 0);

    WidgetSelectAutocompleteData *widget_data = (WidgetSelectAutocompleteData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetSelectAutocompleteData)
    );
    widget_data->label            = params->label;
    widget_data->storage          = params->storage;
    // widget_data->suggestions      = params->suggestions;
    widget_data->label_width      = utf8_str_display_width(params->label);
    widget_data->option_width     = 0;
    widget_data->options_capacity = params->options_capacity;
    if(widget_data->options_capacity > TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX){
        widget_data->options_capacity = TUI_WIDGET_SELECT_AUTOCOMPLETE_OPTIONS_MAX;
    }

    WidgetSelectAutocompleteState *widget_state = (WidgetSelectAutocompleteState *)tui_widget_state(
        widget_id,
        sizeof(WidgetSelectAutocompleteState)
    );
    if(widget_state->query_widget_id[0] == '\0'){
        snprintf(widget_state->query_widget_id, sizeof(widget_state->query_widget_id), "%s_QUERY", widget_id);
    }

    const uint8_t *selected_label = tui_widget_select_autocomplete_selected_label(widget_data);
    widget_data->option_width = utf8_str_display_width(selected_label);

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .state     = widget_state,
        .size.w    = widget_data->label_width + widget_data->option_width + 2,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &tui_widget_select_autocomplete_input,
        .render    = &tui_widget_select_autocomplete_render,
        .overlay   = &tui_widget_select_autocomplete_overlay,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_SELECT_AUTOCOMPLETE_IMPL
#endif //TUI_WIDGET_SELECT_AUTOCOMPLETE

#ifndef TUI_WIDGET_TABS
#define TUI_WIDGET_TABS

#include "tui_layout.h"

//TODO: these are public so we should move them somewhere else and standardize this sort of stuff
typedef struct {
    const uint8_t *label;
    size_t         value;
} WidgetTabOption;

typedef struct {
    WidgetTabOption *values;
    size_t count;
} WidgetTabOptions;

typedef struct {
    bool             is_inline;
    size_t          *storage;
    WidgetTabOptions tabs;
} _WidgetTabsParams;

#define tui_widget_tabs(widget_id, ...) \
        tui_widget_tabs_((widget_id), &(_WidgetTabsParams){__VA_ARGS__})

void tui_widget_tabs_(const char *widget_id, _WidgetTabsParams *params);

#ifdef TUI_WIDGET_TABS_IMPL

typedef struct {
    size_t          *storage;
    WidgetTabOptions tabs;
} _WidgetTabsData;

static constexpr int TUI_TAB_MIN_WIDTH = 12;

static void _tui_widget_tabs_render(Widget *widget, Screen *screen, vec2i position){
    _WidgetTabsData *data = widget->data;

    size_t curr_value = data->storage ? *data->storage : 0;
    int selected_index = 0;
    for(size_t i = 0; i < data->tabs.count; i++){
        if(data->tabs.values[i].value == curr_value){
            selected_index = (int)i;
            break;
        }
    }

    int tab_x = position.x + PADDING;
    for(size_t i = 0; i < data->tabs.count; i++){
        int label_w = (int)utf8_str_display_width(data->tabs.values[i].label);
        int tab_w   = max(TUI_TAB_MIN_WIDTH, label_w + 4);
        bool active = (int)i == selected_index;

        //draw left side of tab
        auto fg_color = COLOR_BLACK;
        auto bg_color = COLOR_DARK_WHITE;
        if(active){
            fg_color = (widget->focused) ? COLOR_MAGENTA : COLOR_WHITE;
            bg_color = COLOR_DARK_MAGENTA;
         }

        screen_format(NORMAL, fg_color, bg_color);
        if(i != 0){
            screen_set_utf8(screen, tab_x, position.y, u8"│");
        }

        //draw tab

        for(int x = tab_x + 1; x < tab_x + tab_w; x++){
            screen_set_char(screen, x, position.y, ' ');
        }

        int lx = tab_x + 1 + (tab_w - 2 - label_w) / 2;
        screen_set_utf8_str(screen, lx, position.y, data->tabs.values[i].label);

        tab_x += tab_w;
    }

    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
}

static bool _tui_widget_tabs_input(Widget *widget, InputEvent input_event){
    _WidgetTabsData *data = widget->data;

    switch(input_event.input_type){
    case INPUT_KEY:
        switch(input_event.key_event.key){
        case KEY_LEFT:{
            int idx = -1;
            for(size_t i = 0; i < data->tabs.count; i++){
                if(data->tabs.values[i].value == *data->storage){ idx = (int)i; break; }
            }
            idx = (idx <= 0) ? (int)data->tabs.count - 1 : idx - 1;
            *data->storage = data->tabs.values[idx].value;
            return true;
        }
        case KEY_RIGHT:{
            int idx = -1;
            for(size_t i = 0; i < data->tabs.count; i++){
                if(data->tabs.values[i].value == *data->storage){ idx = (int)i; break; }
            }
            idx = (idx < 0 || idx >= (int)data->tabs.count - 1) ? 0 : idx + 1;
            *data->storage = data->tabs.values[idx].value;
            return true;
        }
        default:
            break;
        }
    default:
        break;
    }
    return false;
}

void tui_widget_tabs_(const char *widget_id, _WidgetTabsParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->tabs.count > 0);

    _WidgetTabsData *data = (_WidgetTabsData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(_WidgetTabsData)
    );
    data->storage = params->storage;
    data->tabs    = params->tabs;

    int total_w = PADDING * 2;
    for(size_t i = 0; i < params->tabs.count; i++){
        int label_w = (int)utf8_str_display_width(params->tabs.values[i].label);
        total_w += max(TUI_TAB_MIN_WIDTH, label_w + 4);
    }

    Widget new_widget = {
        .id        = widget_id,
        .data      = data,
        .size.w    = total_w,
        .size.h    = 1 + PADDING,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &_tui_widget_tabs_input,
        .render    = &_tui_widget_tabs_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_TABS_IMPL
#endif //TUI_WIDGET_TABS

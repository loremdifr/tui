#ifndef TUI_WIDGET_LABEL
#define TUI_WIDGET_LABEL

#include "tui_string.h"
#include "tui_layout.h"
#include "tui_screen.h"
// #include <stdlib.h>

typedef struct {
	const uint8_t *text;
    Lines          lines;
} WidgetLabelData;

//NOTE: because of how this is structured, it allows to be used like
//      this with the macro below: tui_widget_label(u8"test")
//      this is because in the expansion the order matters, so the *text must remain the first one!
typedef struct {
	const void    *text;
	bool           is_inline;
} WidgetLabelParams;

#define tui_widget_label(...) \
    tui_widget_label_(&(WidgetLabelParams){__VA_ARGS__})
void tui_widget_label_(WidgetLabelParams *params);

#ifdef TUI_WIDGET_LABEL_IMPL


private void tui_widget_label_render(Widget *widget, Screen *screen, vec2i position){
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
	WidgetLabelData *widget_data = widget->data;

    for(size_t i = 0; i < widget_data->lines.count; i++){
        screen_set_string(
            screen,
            position.x,
            position.y + i,
            &widget_data->lines.strings[i]
        );
    }
}

//public
void tui_widget_label_(WidgetLabelParams *params){
    Panel *panel = tui_get_panel_building();
    auto max_width = max(0, panel->inner_rect.size.width);

	WidgetLabelData *widget_data = (WidgetLabelData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetLabelData)
	);
    widget_data->text = (const uint8_t*)params->text;
    String text_str = string_from((uint8_t *)params->text, strlen(params->text));
    widget_data->lines = string_split_into_lines(&text_str, max_width - 1);
    size_t text_display_width = utf8_str_display_width((const uint8_t*)params->text);

    Widget new_widget  = {
        .id        = tui_create_widget_id(),
        .data      = widget_data,
        .size.w    = min(text_display_width, max_width),
        .size.h    = max(1, widget_data->lines.count + PADDING),
        .focusable = false,
        .is_inline = params->is_inline,
        .render    = &tui_widget_label_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_LABEL_IMPL
#endif //TUI_WIDGET_LABEL

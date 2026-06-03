#ifndef TUI_WIDGET_LABEL
#define TUI_WIDGET_LABEL

#include "tui_layout.h"
#include "tui_screen.h"
// #include <stdlib.h>

typedef struct {
	const uint8_t *text;
    size_t total_lines;
} WidgetLabelData;

#define tui_widget_label(text) tui_widget_label_((const uint8_t*)(text))
void tui_widget_label_(const uint8_t *text);

#ifdef TUI_WIDGET_LABEL_IMPL


private void tui_widget_label_render(Widget *widget, Screen *screen, vec2i position){
	WidgetLabelData *widget_data = widget->data;

    for(size_t i = 0; i < widget_data->total_lines; i++){
        //TODO: proper word wrap at the character level
        screen_set_utf8_str(
            screen,
            position.x,
            position.y + i,
            widget_data->text
        );
    }
}

//public
void tui_widget_label_(const uint8_t *text){
    Panel *curr_panel = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_curr];
    auto max_width   = curr_panel->inner_rect.size.width - PADDING * 2 - BORDER * 2;
    auto text_width  = utf8_str_length(text);

	WidgetLabelData *widget_data = (WidgetLabelData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetLabelData)
	);
    widget_data->text = text;
    widget_data->total_lines = text_width / max_width;
    Widget new_widget  = {
        .id        = tui_create_widget_id(),
        .data      = widget_data,
        .size.w    = min(text_width, max_width),
        .size.h    = widget_data->total_lines + PADDING,
        .focusable = false,
        .render    = &tui_widget_label_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_LABEL_IMPL
#endif //TUI_WIDGET_LABEL

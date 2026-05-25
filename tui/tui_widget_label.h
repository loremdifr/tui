#ifndef TUI_WIDGET_LABEL
#define TUI_WIDGET_LABEL

#include "tui_layout.h"
#include "tui_screen.h"
// #include <stdlib.h>

typedef struct {
	const uint8_t *text;
} WidgetLabelData;

#define tui_widget_label(text) tui_widget_label_((const uint8_t*)(text))
void tui_widget_label_(const uint8_t *text);

#ifdef TUI_WIDGET_LABEL_IMPL


private void tui_widget_label_render(Widget *widget, Screen *screen, vec2i position){
	WidgetLabelData *widget_data = widget->data;
    screen_set_utf8_str(
        screen,
        position.x,
        position.y,
        widget_data->text
    );
}

//public
void tui_widget_label_(const uint8_t *text){
	WidgetLabelData *widget_data = (WidgetLabelData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetLabelData)
	);
    widget_data->text = text;
    Widget new_widget  = {
        .id        = tui_create_widget_id(),
        .data      = widget_data,
        .size.w    = utf8_str_length(text),
        .size.h    = 1,
        .focusable = false,
        .render    = &tui_widget_label_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_LABEL_IMPL
#endif //TUI_WIDGET_LABEL

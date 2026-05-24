#ifndef TUI_WIDGET_LABEL
#define TUI_WIDGET_LABEL

#include "tui_layout.h"
#include "tui_screen.h"
// #include <stdlib.h>

typedef struct {
	const uint8_t *text;
} WidgetLabelState;

void tui_widget_label_utf8(const uint8_t *text);
void tui_widget_label(const char *text);

#ifdef TUI_WIDGET_LABEL_IMPL


private void tui_widget_label_render(Widget *widget, Screen *screen, vec2 position){
	WidgetLabelState *widget_state = widget->state;
    screen_set_utf8_str(
        screen,
        position.x,
        position.y,
        widget_state->text
    );
}

//public
void tui_widget_label_utf8(const uint8_t *text){
	WidgetLabelState *widget_state = (WidgetLabelState *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetLabelState)
	);
    widget_state->text = text;
    Widget new_widget  = {
        .id        = tui_create_widget_id(),
        .state     = widget_state,
        .size.w    = utf8_str_length(text),
        .size.h    = 1,
        .focusable = false,
        .render    = &tui_widget_label_render,
    };
    tui_widget_push(new_widget);
}

void tui_widget_label(const char *text){
    tui_widget_label_utf8((const uint8_t*)text);
}

#endif //TUI_WIDGET_LABEL_IMPL
#endif //TUI_WIDGET_LABEL

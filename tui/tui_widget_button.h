#ifndef TUI_WIDGET_BUTTON
#define TUI_WIDGET_BUTTON

#include "tui_layout.h"
// #include <stdlib.h>

typedef struct {
	const char *text;
	FunctionPointer on_click;
} WidgetButtonState;

void tui_widget_button(const char *widget_id, const char *text, FunctionPointer on_click);

#ifdef TUI_WIDGET_BUTTON_IMPL

private void tui_widget_button_render(Widget *widget, Screen *screen, vec2 position){
	WidgetButtonState *widget_state = widget->state;

	//any processing would be done here if needed

	if(widget->focused){
		screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
	}
	tui_draw_box(screen, (rect){
        .position = position,
        .size     = widget->size,
    });
    screen_set_str(
        screen,
        position.x + BORDER + PADDING,
        position.y + BORDER,
        widget_state->text
    );
}

private void tui_widget_button_input(Widget *widget, InputEvent input_event){
	WidgetButtonState *widget_state = widget->state;
	switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
    	case KEY_SPACE:
        case KEY_ENTER:
            widget_state->on_click();
            break;
		case KEY_NONE:
        default:
        break;
        }
    case INPUT_NONE:
    default:
    }

    //TODO: mouse
}


//public
void tui_widget_button(const char *widget_id, const char *text, FunctionPointer on_click){
	WidgetButtonState *widget_state = (WidgetButtonState *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetButtonState)
	);
    widget_state->text     = text;
    widget_state->on_click = on_click;
    Widget new_widget      = {
	    .id        = widget_id,
	    .state     = widget_state,
	    .size.w    = (int)strlen(text) + BORDER * 2 + PADDING * 2,
	    .size.h    = 1 + BORDER * 2,
	    .focusable = true,
	    .input     = &tui_widget_button_input,
	    .render    = &tui_widget_button_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_BUTTON_IMPL
#endif //TUI_WIDGET_BUTTON

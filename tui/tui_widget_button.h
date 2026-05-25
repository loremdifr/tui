#ifndef TUI_WIDGET_BUTTON
#define TUI_WIDGET_BUTTON

#include "tui_layout.h"
// #include <stdlib.h>

typedef struct {
    bool            is_inline; //base widget param
    const uint8_t  *label;
    FunctionPointer on_click;
} WidgetButtonParams;

#define tui_widget_button(widget_id, ...) \
        tui_widget_button_((widget_id), &(WidgetButtonParams){__VA_ARGS__})

void tui_widget_button_(const char *widget_id, WidgetButtonParams *params);

#ifdef TUI_WIDGET_BUTTON_IMPL

typedef struct {
	const uint8_t  *label;
	FunctionPointer on_click;
} WidgetButtonData;

private void tui_widget_button_render(Widget *widget, Screen *screen, vec2i position){
	WidgetButtonData *widget_data = widget->data;

	//any processing would be done here if needed

	if(widget->focused){
		screen_format(BOLD, COLOR_MAGENTA, COLOR_BLACK);
	}
	tui_draw_box(screen, (rect2i){
        .position = position,
        .size     = widget->size,
    });
    screen_set_utf8_str(
        screen,
        position.x + BORDER + PADDING,
        position.y + BORDER,
        widget_data->label
    );
}

private bool tui_widget_button_input(Widget *widget, InputEvent input_event){
	WidgetButtonData  *widget_data  = widget->data;
	switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
    	case KEY_SPACE:
        case KEY_ENTER:
            widget_data->on_click();
            break;
		case KEY_NONE:
        default:
        break;
        }
    case INPUT_NONE:
    default:
    }

    //TODO: mouse
    return false; //<- does not capture input
}


//public
void tui_widget_button_(const char *widget_id, WidgetButtonParams *params){
	WidgetButtonData *widget_data = (WidgetButtonData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetButtonData)
	);
    widget_data->label    = params->label;
    widget_data->on_click = params->on_click;
    Widget new_widget      = {
	    .id        = widget_id,
	    .data      = widget_data,
	    .size.w    = utf8_str_length(params->label)
    				+ BORDER * 2
    				+ PADDING * 2,
	    .size.h    = 1 + BORDER * 2,
	    .focusable = true,
	    .is_inline = params->is_inline,
	    .input     = &tui_widget_button_input,
	    .render    = &tui_widget_button_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_BUTTON_IMPL
#endif //TUI_WIDGET_BUTTON

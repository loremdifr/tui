#ifndef TUI_WIDGET_SPINNER
#define TUI_WIDGET_SPINNER

#include "tui_layout.h"
// #include <stdlib.h>

#define tui_widget_spinner(text) tui_widget_spinner_((const uint8_t*)(text))
void tui_widget_spinner_(const uint8_t *text);

#ifdef TUI_WIDGET_SPINNER_IMPL

typedef struct {
	const uint8_t  *label;
} WidgetSpinnerData;

typedef struct {
    double  animation_started;
    uint8_t frame_curr;
    uint8_t frame_target;
    uint8_t frame_start;
    int8_t  frame_step;
} WidgetSpinnerState;

private void tui_widget_spinner_render(Widget *widget, Screen *screen, vec2i position){
    WidgetSpinnerState *widget_state = widget->state;
    WidgetSpinnerData *widget_data   = widget->data;
    static const AnimationFrame frames[] = {

        {.text=u8"▘", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▀", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▜", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▐", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▗", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},

        {.text=u8"▄", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▙", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▌", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▛", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▀", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▝", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},

        {.text=u8"▐", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▟", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▄", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▙", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▌", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
        {.text=u8"▘", .fg_color=COLOR_CYAN, .bg_color=COLOR_BLACK},
    };
    static const size_t frames_count = arr_size(frames);
    static const double animation_speed = 1.5;
    static const double frame_speed = animation_speed / frames_count;

    widget_state->frame_target = frames_count;

    //determine expected animation frame based on the
    // time since it started / frame_speed
    // this should give me the animation index
    auto now = get_curr_time();
    auto animation_progress = now - widget_state->animation_started;
    auto animation_progress_frame = animation_progress / frame_speed;
    auto frame_progress_amount = animation_progress_frame * widget_state->frame_step;
    widget_state->frame_curr = clamp_overflow(
        widget_state->frame_start + frame_progress_amount, 0, frames_count-1
    );

    //start animation again
    if(animation_progress >= animation_speed){
        widget_state->animation_started = get_curr_time();
        widget_state->frame_step   = 1;
        widget_state->frame_start  = 0;
    }

    //render the animation FRAME
    const AnimationFrame *frame = &frames[widget_state->frame_curr];
    screen_format(BOLD, frame->fg_color, frame->bg_color);
    screen_set_utf8_str(
        screen,
        position.x + PADDING,
        position.y,
        frame->text
    );

    //render LABEL
    screen_format(ITALIC, COLOR_WHITE, COLOR_BLACK);
    screen_set_utf8_str(
        screen,
        position.x + PADDING + 2,
        position.y,
        widget_data->label
    );

}

//public
void tui_widget_spinner_(const uint8_t *text){
	WidgetSpinnerData *widget_data = (WidgetSpinnerData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetSpinnerData)
	);
    widget_data->label = text;

    auto widget_id = tui_create_widget_id();

    //widget state persist across frames
    auto widget_state = (WidgetSpinnerState *)tui_widget_state(
        widget_id,
        sizeof(WidgetSpinnerState)
    );

    Widget new_widget = {
	    .id        = widget_id,
	    .data      = widget_data,
        .state     = widget_state,
	    .size.w    = utf8_str_length(text)
    				+ PADDING * 2 + 1,
	    .size.h    = 1 + PADDING,
	    .focusable = false,
	    .render    = &tui_widget_spinner_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_SPINNER_IMPL
#endif //TUI_WIDGET_SPINNER

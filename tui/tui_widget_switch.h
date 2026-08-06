#ifndef TUI_WIDGET_SWITCH
#define TUI_WIDGET_SWITCH

#include "tui_layout.h"
// #include <stdlib.h>

typedef struct {
    bool            is_inline; //base widget param
    const uint8_t  *label;
    bool           *storage;
    FunctionPointer on_toggle; //this is optional, and it's a callback!
} _WidgetSwitchParams;

#define tui_widget_switch(widget_id, ...) \
        tui_widget_switch_((widget_id), &(_WidgetSwitchParams){__VA_ARGS__})

void tui_widget_switch_(const char *widget_id, _WidgetSwitchParams *params);

#ifdef TUI_WIDGET_SWITCH_IMPL

typedef struct {
	const uint8_t  *label;
    bool           *storage;
	FunctionPointer on_toggle;
} _WidgetSwitchData;

typedef struct {
    bool    animation_playing;
    double  animation_started;
    uint8_t frame_curr;
    uint8_t frame_target;
    uint8_t frame_start;
    int8_t  frame_step;
} _WidgetSwitchState;

static void _tui_widget_switch_render(Widget *widget, Screen *screen, vec2i position){
    //TODO: need to be able to get the color theme from here.
    _WidgetSwitchState *widget_state = widget->state;
    _WidgetSwitchData *widget_data   = widget->data;
    static const AnimationFrame frames[] = {
        {.text=u8"██  ", .fg_color=COLOR_FG_TEXT, .bg_color=COLOR_BG_DANGER},
        {.text=u8"▐█▌ ", .fg_color=COLOR_FG_TEXT, .bg_color=COLOR_BG_DANGER},
        {.text=u8"▐██ ", .fg_color=COLOR_FG_TEXT, .bg_color=COLOR_BG_DANGER},
        {.text=u8" ██▌", .fg_color=COLOR_FG_TEXT, .bg_color=COLOR_BG_SUCCESS},
        {.text=u8" ▐██", .fg_color=COLOR_FG_TEXT, .bg_color=COLOR_BG_SUCCESS},
        {.text=u8"  ██", .fg_color=COLOR_FG_TEXT, .bg_color=COLOR_BG_SUCCESS},
    };
    static const size_t frames_count = arr_size(frames);
    static const double animation_speed = 0.15;
    static const double frame_speed = animation_speed / frames_count;

    widget_state->frame_target = (*widget_data->storage == true) ? frames_count-1 : 0;

    //should animation start playing if it isn't?
    if(!widget_state->animation_playing
    && widget_state->frame_curr != widget_state->frame_target){
        //start animation
        widget_state->animation_playing = true;
        widget_state->animation_started = get_curr_time();
        widget_state->frame_step   = (widget_state->frame_target == 0) ? -1 : 1;
        widget_state->frame_start  = widget_state->frame_curr;
    }else if(widget_state->frame_curr == widget_state->frame_target){
        //stop animation
        widget_state->animation_playing = false;
    }

    //determine expected animation frame based on the
    // time since it started / frame_speed
    // this should give me the animation index
    auto now = get_curr_time();
    auto animation_progress = now - widget_state->animation_started;
    auto animation_progress_frame = animation_progress / frame_speed;
    auto frame_progress_amount = animation_progress_frame * widget_state->frame_step;
    widget_state->frame_curr = clamp(
        widget_state->frame_start + frame_progress_amount, 0, frames_count-1
    );

    //render LABEL
    if(widget->focused){
		screen_format(BOLD, COLOR_FG_PRIMARY, COLOR_BG_TEXT);
	}else{
        screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    }
    screen_set_utf8_str(
        screen,
        position.x + PADDING,
        position.y,
        widget_data->label
    );

    //render the animation FRAME
    const AnimationFrame *frame = &frames[widget_state->frame_curr];
    screen_format(BOLD, COLOR_WHITE, frame->bg_color);
    screen_set_utf8_str(
        screen,
        position.x + PADDING + utf8_str_display_width(widget_data->label),
        position.y,
        frame->text
    );
}

static bool _tui_widget_switch_input(Widget *widget, InputEvent input_event){
	_WidgetSwitchData *widget_data = widget->data;
	switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
    	case KEY_SPACE:
        case KEY_ENTER:
            *widget_data->storage = !(*widget_data->storage);
            if(widget_data->on_toggle != nullptr){
                widget_data->on_toggle();
            }
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
void tui_widget_switch_(const char *widget_id, _WidgetSwitchParams *params){
	_WidgetSwitchData *widget_data = (_WidgetSwitchData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(_WidgetSwitchData)
	);
    widget_data->storage   = params->storage;
    widget_data->label     = params->label;
    widget_data->on_toggle = params->on_toggle;

    //widget state persist across frames
    auto widget_state = (_WidgetSwitchState *)tui_widget_state(
        widget_id,
        sizeof(_WidgetSwitchState)
    );

    Widget new_widget      = {
	    .id        = widget_id,
	    .data      = widget_data,
        .state     = widget_state,
	    .size.w    = utf8_str_display_width(params->label)
    				+ PADDING * 2 + 4,
	    .size.h    = 1 + PADDING,
	    .focusable = true,
	    .is_inline = params->is_inline,
	    .input     = &_tui_widget_switch_input,
	    .render    = &_tui_widget_switch_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_SWITCH_IMPL
#endif //TUI_WIDGET_SWITCH

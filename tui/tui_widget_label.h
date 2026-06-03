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

#define tui_widget_label(text) tui_widget_label_((const uint8_t*)(text))
void tui_widget_label_(const uint8_t *text);

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
void tui_widget_label_(const uint8_t *text){
    Panel *curr_panel = &LAYOUT_STATE.panels[LAYOUT_STATE.panel_curr];

    auto max_width = curr_panel->inner_rect.size.width - PADDING * 2 - BORDER * 2;
    if(max_width <= 0) max_width = 1;

	WidgetLabelData *widget_data = (WidgetLabelData *)arena_alloc(
		LAYOUT_STATE.arena_frame, sizeof(WidgetLabelData)
	);
    widget_data->text = text;
    String text_str = string_from((uint8_t *)text, strlen((char *)text));
    widget_data->lines = string_split_into_lines(&text_str, max_width);
    size_t text_length = utf8_str_length(text);


    //split into lines for word wrap
    // if(text_length > 0){
    //

    //     if(text_length <= max_width){
    //         widget_data->lines.count = 1;
    //         widget_data->lines.strings = (String *)arena_alloc(
    //             LAYOUT_STATE.arena_frame, sizeof(String)
    //         );
    //         widget_data->lines.strings[0] = string_substr(&str, 0, text_length);
    //     } else {
    //         size_t line_count = string_word_wrap_count(&str, max_width);
    //         String *lines = (String *)arena_alloc(
    //             LAYOUT_STATE.arena_frame, sizeof(String) * line_count
    //         );
    //         string_word_wrap(&str, max_width, lines, line_count);
    //         widget_data->lines = (Lines){ .strings = lines, .count = line_count };
    //     }
    // }

    Widget new_widget  = {
        .id        = tui_create_widget_id(),
        .data      = widget_data,
        .size.w    = min(text_length, max_width),
        .size.h    = max(1, widget_data->lines.count + PADDING),
        .focusable = false,
        .render    = &tui_widget_label_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_LABEL_IMPL
#endif //TUI_WIDGET_LABEL

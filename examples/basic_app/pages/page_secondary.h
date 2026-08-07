#ifndef PAGE_SECONDARY_H
#define PAGE_SECONDARY_H

#include "tui/tui.h"

#define PAGE_SECONDARY_ID "PAGE_2"
extern Page PAGE_SECONDARY;

#ifdef PAGE_SECONDARY_IMPL

//variables de la pagina
static bool secondary_popup = false;

static void close_secondary_popup(void){
	secondary_popup = false;
}

static void page_secondary_init(void){
}

static bool page_secondary_input(InputEvent input_event){
    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_P:
            // toggle popup
            secondary_popup = !secondary_popup;
            return true;
        case KEY_NONE:
        default:
        break;
        }
    case INPUT_NONE:
    default:
    }
    return false;
}

static void page_secondary_process(float delta_time){
    (void)delta_time;
}

static void page_secondary_canvas_render(Screen *screen, vec2i position){
    // Test tui_draw_box
    tui_draw_box(screen, (rect2i){
        .pos = position,
        .size = {.w = 10, .h = 5}
    });

    // Test tui_draw_box_connected
    tui_draw_box_connected(screen, (rect2i){
        .pos = {.x = position.x + 12, .y = position.y},
        .size = {.w = 10, .h = 5}
    });
    // Add another box to test connection
    tui_draw_box_connected(screen, (rect2i){
        .pos = {.x = position.x + 17, .y = position.y + 2},
        .size = {.w = 10, .h = 5}
    });

    // Test tui_draw_line
    // screen_format(NORMAL, COLOR_RED, COLOR_BLACK);
    tui_draw_line(screen, (uint8_t *)u8"*",
        (vec2i){.x = position.x, .y = position.y + 6},
        (vec2i){.x = position.x + 20, .y = position.y + 6}
    );

    // Test tui_draw_line_bresenham (diagonal)
    // screen_format(NORMAL, COLOR_GREEN, COLOR_BLACK);
    tui_draw_line_bresenham(screen, (uint8_t *)u8"x",
        (vec2i){.x = position.x, .y = position.y + 7},
        (vec2i){.x = position.x + 10, .y = position.y + 12}
    );

    // Test tui_draw_rect
    // screen_format(NORMAL, COLOR_YELLOW, COLOR_BLACK);
    tui_draw_rect(screen, (uint8_t *)u8"█", (rect2i){
        .pos = {.x = position.x + 15, .y = position.y + 8},
        .size = {.w = 4, .h = 3}
    });

    // Test tui_draw_circ
    // screen_format(NORMAL, COLOR_CYAN, COLOR_BLACK);
    tui_draw_circ(screen, (uint8_t *)u8"o", (rect2i){
        .pos = {.x = position.x + 22, .y = position.y + 10},
        .size = {.w = 8, .h = 5}
    });

    // Test tui_draw_line_braille
    // screen_format(NORMAL, COLOR_MAGENTA, COLOR_BLACK);
    tui_draw_line_braille(screen,
        (vec2i){.x = position.x + 35, .y = position.y + 2},
        (vec2i){.x = position.x + 55, .y = position.y + 10}
    );

    // Test tui_draw_box_title
    rect2i box_with_title = {
        .pos = {.x = position.x + 40, .y = position.y + 12},
        .size = {.w = 15, .h = 3}
    };
    tui_draw_box(screen, box_with_title);
    auto title = string_from((uint8_t *)u8"Title", 5);
    tui_draw_box_title(screen, box_with_title, &title, BOX_TITLE_TOP_LEFT);
}

static void page_secondary_render(void){
	tui_panel_begin(SLOT_SIDEBAR);
		tui_widget_label(u8"SIDEBAR");
		tui_widget_label(u8"---------");
		tui_widget_label(u8"Nav Menu");
		tui_widget_label(u8"Info Panel");
	tui_panel_end();

	tui_panel_begin(SLOT_MAIN);

		tui_widget_label(u8"Welcome to the Secondary Page!");
		tui_widget_label(u8"Press [P] to toggle popup");

        tui_widget_canvas("CANVAS_TEST",
            .size = {.w = 60, .h = 15},
            .on_render = &page_secondary_canvas_render
        );

		tui_widget_button("BACK_BUTTON",
			.label=u8"Back",
			.on_click=&tui_navigate_back,
			// .is_inline=true,
		);
		tui_widget_button("BACK_BUTTON2",
			.label=u8"Back",
			.on_click=&tui_navigate_back,
			// .is_inline=true,
		);
		tui_widget_button("BACK_BUTTON3",
			.label=u8"Back",
			.on_click=&tui_navigate_back,
			// .is_inline=true,
		);

	tui_panel_end();

	if(!secondary_popup) return;

	tui_layer_begin(LAYER_OVERLAY, LAYOUT_SINGLE_PANEL);
		tui_panel_begin(SLOT_MAIN);
			tui_widget_label(u8"Secondary Popup!");
			tui_widget_button("SECONDARY_POPUP_BUTTON",
				.label=u8"OK!",
				.on_click=&close_secondary_popup,
			);
		tui_panel_end();
	tui_layer_end();
}

Page PAGE_SECONDARY = {
	.title   = u8"Secondary Page",
	.layout  = LAYOUT_SIDEBAR_LEFT,
	.init    = &page_secondary_init,
	.input   = &page_secondary_input,
	.process = &page_secondary_process,
	.render  = &page_secondary_render,
};

#endif //PAGE_SECONDARY_IMPL
#endif //PAGE_SECONDARY_H

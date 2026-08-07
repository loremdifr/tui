#ifndef TUI
#define TUI

#include <stdint.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "tui_i18n.h"
#include "tui_style.h"

#define TUI_UTILS_IMPL
#include "tui_utils.h"

#define TUI_STRING_IMPL
#include "tui_string.h"

#define TUI_ARENA_IMPL
#include "tui_arena.h"

#define TUI_PLATFORM_IMPL
#include "tui_platform.h"

#define TUI_SCREEN_IMPL
#include "tui_screen.h"

#define TUI_DRAW_IMPL
#include "tui_draw.h"

#define TUI_LAYOUT_IMPL
#include "tui_layout.h"

#define TUI_NAVIGATION_IMPL
#include "tui_navigation.h"

#define TUI_HOTKEYS_IMPL
#include "tui_hotkeys.h"

//widgets
#define TUI_WIDGET_BUTTON_IMPL
#include "tui_widget_button.h"

#define TUI_WIDGET_LABEL_IMPL
#include "tui_widget_label.h"

#define TUI_WIDGET_SWITCH_IMPL
#include "tui_widget_switch.h"

#define TUI_WIDGET_INPUT_CHECKBOX_IMPL
#include "tui_widget_input_checkbox.h"

#define TUI_WIDGET_INPUT_RADIO_IMPL
#include "tui_widget_input_radio.h"

#define TUI_WIDGET_INPUT_NUMBER_IMPL
#include "tui_widget_input_number.h"

#define TUI_WIDGET_INPUT_TEXT_IMPL
#include "tui_widget_input_text.h"

#define TUI_WIDGET_SPINNER_IMPL
#include "tui_widget_spinner.h"

#define TUI_WIDGET_SELECT_IMPL
#include "tui_widget_select.h"

#define TUI_WIDGET_SELECT_FILTER_IMPL
#include "tui_widget_select_filter.h"

#define TUI_WIDGET_CANVAS_IMPL
#include "tui_widget_canvas.h"

#define TUI_WIDGET_VIRTUAL_LIST_IMPL
#include "tui_widget_virtual_list.h"

#define TUI_WIDGET_TABLE_IMPL
#include "tui_widget_table.h"

#define TUI_WIDGET_TABS_IMPL
#include "tui_widget_tabs.h"

//API --------------------------------------------------------------------------

void   tui_run_loop(void); //run on the entry file to start the main loop
void   tui_quit(void);
void   tui_set_theme(Theme theme);
Theme  tui_get_theme();

//aux
void tui_move_to(int x, int y);
void tui_clear(void);
void tui_write_empty_char(void);

// IMPL ------------------------------------------------------------------------
#ifdef TUI_IMPL

typedef struct {
	bool    exit;
	int     fps;
	Screen  curr_screen;
	Screen  next_screen; //double buffered
	double  curr_frame_time;
	double  prev_frame_time;
	double  frame_delta;
	Theme   theme;
} _AppState; //TODO: UI state?
_AppState APP_STATE = {};

// Theme ------
void tui_set_theme(Theme theme){
	APP_STATE.theme = theme;
	APP_STATE.curr_screen.theme = theme;
	APP_STATE.next_screen.theme = theme;
	//TODO: might need to force a render here in case the theme is changed in the
	//      middle of the rendering process...?
}

Theme tui_get_theme(){
	return APP_STATE.theme;
}

//TUI loop ---------------------------------------------------------------------

static void _tui_render_header(Screen *screen){
	screen_format(NORMAL, screen->theme.colors[COLOR_TEXT]);

	if (NAV_HISTORY.count <= 0) return;

	size_t available_width = screen->size.w - 2;

	int count = NAV_HISTORY.count;
	const uint8_t *titles[count];
	size_t widths[count];
	for (int i = 0; i < count; i++){
		int idx = NAV_HISTORY.stack[i];
		titles[i] = PAGE_ROUTES.routes[idx].page->title;
		widths[i] = utf8_str_display_width(titles[i]);
	}

	uint8_t header_str[256] = {};
    // construct the breadcrumb string
    for (int i = 0; i < count; i++){
        if (i > 0) utf8_str_concat(header_str, NAV_HISTORY_SEPARATOR);
        utf8_str_concat(header_str, titles[i]);
    }

    screen_set_utf8_str(screen, 1, 0, header_str);
}

static void _tui_render(void){
	//display diff of screen buffers
	for(int x = 0; x < APP_STATE.curr_screen.size.x; x++){
		for(int y = 0; y < APP_STATE.curr_screen.size.y; y++){

			auto curr_cell = screen_get(&APP_STATE.curr_screen, x, y);
			auto next_cell = screen_get(&APP_STATE.next_screen, x, y);
			if(memcmp(curr_cell, next_cell, sizeof(Cell)) == 0){
				continue;
			}

			//skip cells that are the second column of a wide character
			//TODO: is this correct..? test more
			auto prev_column_cell = screen_get(&APP_STATE.next_screen, x - 1, y);
			bool is_second_column_of_wide_char = (x > 0 && prev_column_cell->display_width > 1);
			if(is_second_column_of_wide_char) continue;

			tui_move_to(x + 1, y + 1); //we add 1 because terminal pos is 1 based
			_tui_write_color(next_cell->text_format, next_cell->colors);
			if(next_cell->display_width == 0 || next_cell->bytes_used == 0){
				tui_write_empty_char();
			}else{
				tui_write_bytes(next_cell->bytes, next_cell->bytes_used);
			}

			//do we need to erase cells of a previous character,
			//if it was wider than this one?
			auto width_diff = next_cell->display_width - curr_cell->display_width;
			if(width_diff > 0){
				for(int i = 1; i <= width_diff; i++){
					auto target_x = x + i;
					if(target_x >= APP_STATE.curr_screen.size.x) break;
					tui_move_to(target_x + 1, y + 1); //we add 1 because terminal pos is 1 based
					tui_write_empty_char();
				}
			}
		}
	}

	// swap buffer
	Screen temp = APP_STATE.curr_screen;
	APP_STATE.curr_screen = APP_STATE.next_screen;
	APP_STATE.next_screen = temp;

	//clear next buffer
	screen_clear(&APP_STATE.next_screen);
}

static bool _tui_process_input_resize(InputEvent input_event){
	if(input_event.input_type != INPUT_WINDOW_RESIZE) return false;

	auto terminal_size = tui_size();

	screen_free(&APP_STATE.curr_screen);
	screen_free(&APP_STATE.next_screen);
	APP_STATE.curr_screen = screen_create(terminal_size);
	APP_STATE.next_screen = screen_create(terminal_size);

	tui_clear();
	return true;
}

void tui_run_loop(void){
	//App State Init
	auto terminal_size = tui_size();
	APP_STATE.curr_screen = screen_create(terminal_size);
	APP_STATE.next_screen = screen_create(terminal_size);
	APP_STATE.fps = 10;
	APP_STATE.prev_frame_time = get_curr_time();
	APP_STATE.curr_frame_time = get_curr_time();
	tui_set_theme(APP_STATE.theme); //in case the theme was set before creating the screens!
	const double frame_time = 1.0 / APP_STATE.fps;

	// tui_set_resize_callback(tui_resize);
	tui_init(); //platform init
	tui_clear();

	//init pages
	for(int i = 0; i < PAGE_ROUTES.routes_count; i++){
		//TODO: possible optimization, delay until first page visit
		if(PAGE_ROUTES.routes[i].page->init != nullptr){
			PAGE_ROUTES.routes[i].page->init();
		}
	}

#ifdef TUI_WINDOWS
	//windos is a naught ynaught boy
	Sleep(100);
	emit_resize_event();
#endif //TUI_WINDOWS

	while(!APP_STATE.exit){
        APP_STATE.curr_frame_time = get_curr_time();
        APP_STATE.frame_delta = APP_STATE.curr_frame_time - APP_STATE.prev_frame_time;

        // NOTE: rendering and logic framerate is coupled for now
        //       because we need the frame delta in both and i dont
        //       wanna mantain two separate deltas... it's probably fine
		if (APP_STATE.frame_delta < frame_time){
			//poll input until next frame to about busy-waiting!!!
			tui_input_read(frame_time - APP_STATE.frame_delta);
		}
		APP_STATE.prev_frame_time = APP_STATE.curr_frame_time;

		Page *active_page = tui_get_curr_page();
		if(active_page->process != nullptr){
			active_page->process(APP_STATE.frame_delta);
		}

		//prepare page render and create widgets,
		//this is done here because widgets need to exist
		//before the input processing pass!
		_tui_layout_prepare(&APP_STATE.next_screen, active_page->layout);
		active_page->render(); // <- this actually creates the widgets

		//NOTE: we HAVE to call this here because they must exists
		//      because hotkey registration time
		_tui_layout_build_widget_overlays();

		// RESET HOTKEYS:
		// we do this every frame to make it more convenient to the user
		_tui_reset_hotkeys();

		//NOTE: The order is important! it is from deeper control upwards.
		//      Any of those functions returning true will consume the event
		//      And it won't be passed to the next "layer"
		tui_input_process(&_tui_widget_focused_input);
		if(active_page->input != nullptr){
			tui_input_process(active_page->input);
		}
		tui_input_process(&_tui_process_input_hotkeys);

		screen_format(NORMAL, APP_STATE.theme.colors[COLOR_TEXT]); //reset format

		_tui_layout_render();
		_tui_render_header(&APP_STATE.next_screen);
		_tui_render_hotkeys(&APP_STATE.next_screen);

		//render the TUI diff to the screen
		_tui_render();

		//NOTE: we handle this at the end because it can destroy the screens!
		tui_input_process(&_tui_process_input_resize);
	}
	screen_free(&APP_STATE.curr_screen);
	screen_free(&APP_STATE.next_screen);
	tui_close();
	screen_format(NORMAL, APP_STATE.theme.colors[COLOR_TEXT]); //reset format
	tui_clear();
}

void tui_quit(void){
	APP_STATE.exit = true;
}

//aux --------------------------------------------------------------------------

void tui_move_to(int x, int y){
	tui_write_format("\033[%d;%dH", y, x);
}

void tui_clear(){
	// screen_format(NORMAL, APP_STATE.theme.colors[COLOR_TEXT]);
	tui_write(
		// "\033[48;2;30;40;60m" //this seesm to work?
		"\033[2J\033[H" // limpia la pantalla y mueve el cursor a 0,0
		"\033[3J" 		// limpia el scrollback
	);
}

void tui_write_empty_char(){
	screen_format(NORMAL, APP_STATE.theme.colors[COLOR_TEXT]);
	tui_write((const char*)EMPTY_U8);
}


#endif //TUI_IMPL
#endif //TUI

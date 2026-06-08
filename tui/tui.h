#ifndef TUI
#define TUI

#include <stdint.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "tui_i18n.h"

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

#define TUI_LAYOUT_IMPL
#include "tui_layout.h"

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

#define TUI_WIDGET_SELECT_AUTOCOMPLETE_IMPL
#include "tui_widget_select_autocomplete.h"

// Pages and nav ---------------------------------------------------------------

typedef void (*InitFunction   )(void);
typedef bool (*InputFunction  )(InputEvent input_event);
typedef void (*ProcessFunction)(float delta_time);
typedef void (*RenderFunction )(void);

typedef struct {
	const uint8_t  *title;
    PageLayout      layout;
    InitFunction    init;
    InputFunction   input;
    ProcessFunction process;
    RenderFunction  render;
} Page;

//API --------------------------------------------------------------------------

void tui_register_page(const char *page_id, Page *page);
void tui_register_key(Key key, ModKeys mod_keys, FunctionPointer action);
void tui_register_key_hint(const uint8_t *key, const uint8_t *hint);
void tui_navigate_to(const char *page_id);
void tui_navigate_back(void);
String tui_navigation_string(void); //TODO:
Page *tui_get_curr_page();
void tui_quit(void);

void tui_run_loop(void); //run on the entry file to start the main loop

//aux
void tui_move_to(int x, int y);
void tui_clear(void);

// IMPL ------------------------------------------------------------------------
#ifdef TUI_IMPL

// pages -----------------------------------------------------------------------

#define TUI_MAX_PAGES 64

typedef struct {
	char *page_id;
	Page *page;
} PageRoute;

typedef struct {
	PageRoute routes[TUI_MAX_PAGES];
	int routes_count;
} PageRoutes;

private PageRoutes PAGE_ROUTES = {};

void tui_register_page(const char *page_id, Page *page) {
	//TODO: might wanna assert here the app has not started yet
	assert(PAGE_ROUTES.routes_count <= TUI_MAX_PAGES);
	PageRoute *page_route = &PAGE_ROUTES.routes[PAGE_ROUTES.routes_count];
	page_route->page_id = calloc(strlen(page_id) + 1, sizeof(char));
	strcpy(page_route->page_id, page_id);
	page_route->page = page;
	PAGE_ROUTES.routes_count++;
}


// Hotkeys ---------------------------------------------------------------------

#define TUI_HOTKEYS_MAX 64

typedef struct {
	Key             key;
	ModKeys         mod_keys;
	FunctionPointer action;
} Hotkey;

typedef struct {
	Hotkey  hotkeys[TUI_HOTKEYS_MAX];
	uint8_t total;
} Hotkeys;

private Hotkeys HOTKEYS = {};

void tui_register_key(Key key, ModKeys mod_keys, FunctionPointer action){
	assert(HOTKEYS.total <= TUI_HOTKEYS_MAX);
	HOTKEYS.hotkeys[HOTKEYS.total++] = (Hotkey){
		.key      = key,
		.mod_keys = mod_keys,
		.action   = action,
	};
}

typedef struct {
	const uint8_t *key;
	const uint8_t *hint;
} HotkeyHint;

typedef struct {
	HotkeyHint hints[TUI_HOTKEYS_MAX];
	uint8_t    total;
} HotkeyHints;

private HotkeyHints HOTKEY_HINTS = {};

void tui_register_key_hint(const uint8_t *key, const uint8_t *hint){
	HOTKEY_HINTS.hints[HOTKEY_HINTS.total++] = (HotkeyHint){
        .key  = key,
        .hint = hint,
	};
}

// nav stuff -------------------------------------------------------------------

private constexpr int TUI_NAV_HISTORY_MAX = 64;

typedef struct {
	int stack[TUI_NAV_HISTORY_MAX];
	int count;
} NavigationHistory;

private NavigationHistory NAV_HISTORY;

//app state --------------------------------------------------------------------

typedef struct {
	bool    exit;
	int     fps;
	Screen  curr_screen;
	Screen  next_screen; //double buffered
	double  curr_frame_time;
	double  prev_frame_time;
	double  frame_delta;
} AppState; //TODO: UI state?
AppState APP_STATE = {};

//TUI loop ---------------------------------------------------------------------
// TUI HOTKEY KEYS PANEL
//TODO: not sure if this merits its own file
private bool HOTKEY_HELP_SHOW    = false;
private bool HOTKEY_HELP_ENABLED = false;
private void tui_toggle_help(void){
	if(HOTKEY_HELP_ENABLED){
    	HOTKEY_HELP_SHOW = !HOTKEY_HELP_SHOW;
	}else {
		HOTKEY_HELP_SHOW = false;
	}
}
private void tui_reset_hotkeys(void){
	HOTKEYS.total = 0;
	HOTKEY_HINTS.total = 0;

	//arrows to select widgets
	tui_register_key(KEY_LEFT,   KEY_MOD_NONE,  &tui_cursor_prev_widget);
	tui_register_key(KEY_RIGHT,  KEY_MOD_NONE,  &tui_cursor_next_widget);
	tui_register_key(KEY_UP,     KEY_MOD_NONE,  &tui_cursor_prev_widget);
	tui_register_key(KEY_DOWN,   KEY_MOD_NONE,  &tui_cursor_next_widget);
	tui_register_key_hint(I18N_HINT_SELECT_KEY, I18N_HINT_SELECT_TEXT);

	//enter to activate selected widget
	//TODO: if this is here--- shouldnt we have ENTER as an activate widget key??
	//      and not in the widget itself?
	tui_register_key_hint(I18N_HINT_OK_KEY, I18N_HINT_OK_TEXT);

	//scroll
	//TODO: this should somehow detect if the active panel is scrollable
	tui_register_key(KEY_PAGEUP,    KEY_MOD_NONE,  &tui_panel_scroll_up);
	tui_register_key(KEY_PAGEDOWN,  KEY_MOD_NONE,  &tui_panel_scroll_down);
	tui_register_key_hint(I18N_HINT_SCROLL_KEY, I18N_HINT_SCROLL_TEXT);

	//tab to select panels
	//TODO: this should be contingent on there being more than 1 panel
	tui_register_key_hint(I18N_HINT_SWITCH_PANEL_KEY, I18N_HINT_SWITCH_PANEL_TEXT);
	tui_register_key(KEY_TAB,      KEY_MOD_NONE,  &tui_cursor_next_panel);
	tui_register_key(KEY_BACKTAB,  KEY_MOD_SHIFT, &tui_cursor_prev_panel);

	//esc to navigate back
	//TODO: this should be contingent on there even being a back in the nav
	tui_register_key(KEY_ESCAPE, KEY_MOD_NONE,  &tui_navigate_back);
	tui_register_key_hint(I18N_HINT_BACK_KEY, I18N_HINT_BACK_TEXT);

	//? to toggle help overlay
	//NOTE: registered here instead of tui_render_hotkeys because hotkeys
	//      are reset every frame BEFORE input processing.
	//      we register the hint conditionally later.
	tui_register_key((Key)'?', KEY_MOD_NONE, &tui_toggle_help);
}

private void tui_render_header(Screen *screen){
	screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);

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
        if (i > 0) utf8_str_concat(header_str, u8" > ");
        utf8_str_concat(header_str, titles[i]);
    }

    screen_set_utf8_str(screen, 1, 0, header_str);
}

private void tui_render_hotkeys(Screen *screen){
    screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
    auto separator = u8" ● ";

    size_t max_width = screen->size.w; //in characters/cells
    size_t max_size = max_width * 4 + 1; //4 bytes for u8 + terminator
    size_t included_hints_count = 0;
    size_t hint_widths[TUI_HOTKEYS_MAX] = {}; //stores TOTAL width at each hint
    uint8_t display_str[max_size] = {};

    //first try to fill with all the hotkey hints, and see if they fit
    for(int i = 0; i < HOTKEY_HINTS.total; i++){
        HotkeyHint hkey = HOTKEY_HINTS.hints[i];
        hint_widths[included_hints_count] = strlen(
        	(const char*)display_str
    	);

        //create current hint text
        uint8_t hint_text[max_size] = {};
        utf8_str_concat(hint_text, hkey.key);
        utf8_str_concat(hint_text, u8" ");
        utf8_str_concat(hint_text, hkey.hint);
        if(i != HOTKEY_HINTS.total - 1){
        	//last hotkey doesnt have separator
        	//TODO: use the max_width instead?
            utf8_str_concat(hint_text, separator);
        }

        //add hint to the display_str
        size_t prev_width = utf8_str_display_width(display_str);
        utf8_str_concat_max(display_str, hint_text, max_width);

        //check if the hint was added fully or if it couldn't add it entirely
        size_t curr_size = utf8_str_display_width(display_str);
        if(curr_size < prev_width + utf8_str_display_width(hint_text)){
        	auto last_character = hint_widths[included_hints_count];
            display_str[last_character] = '\0';
            break;
        }
        included_hints_count++;
    }

    //check if we need more space
    if(included_hints_count < HOTKEY_HINTS.total){
    	//now we need to make room in the display_str for the help hint
	    static const uint8_t help_text[] = u8"[?] Help";
	    size_t help_width = utf8_str_display_width(help_text);

	    while(included_hints_count > 0){
	    	//remove hints until the help hint fits
	        size_t left_width = utf8_str_display_width(display_str);
	        if (left_width + help_width <= max_width){
	        	//it fits!
	        	break;
	        }
	        //remove one hint...
	        included_hints_count--;
	        auto last_character = hint_widths[included_hints_count];
            display_str[last_character] = '\0';
	    }

	    //show hint if needed and align to the right
	    size_t left_width = utf8_str_display_width(display_str);
        screen_set_utf8_str(
        	screen,
        	max_width - help_width,
        	screen->size.h, help_text
    	);

	    HOTKEY_HELP_ENABLED = true;
    }else{
    	HOTKEY_HELP_ENABLED = false;
    	HOTKEY_HELP_SHOW = false;
    }

    //actually render the display_str
    screen_set_utf8_str(screen, 0, screen->size.h, display_str);

    // hint help box overlay
    if(HOTKEY_HELP_SHOW){
        int line_count = HOTKEY_HINTS.total;

        //draw box
        int box_height = line_count + BORDER * 2 + PADDING * 2;
        int box_y      = max(0, screen->size.h - box_height);

        tui_draw_rect(screen, u8" ", (rect2i){
            .pos = {0, box_y},
            .size = {max_width, box_height}
        });
        tui_draw_box(screen, (rect2i){
            .pos = {0, box_y},
            .size = {max_width, box_height}
        });

        // box title and footer
        static const uint8_t *help_title = I18N_HELP_TITLE;
        static const uint8_t *help_footer = I18N_HELP_CLOSE;
        String help_title_str  = string_from((uint8_t *)help_title, strlen((char *)help_title));
        String help_footer_str = string_from((uint8_t *)help_footer, strlen((char *)help_footer));
        tui_draw_box_title(screen, (rect2i){
            .pos = {0, box_y},
            .size = {max_width, box_height}
        }, &help_title_str, BOX_TITLE_TOP_LEFT);
        tui_draw_box_title(screen, (rect2i){
            .pos = {0, box_y},
            .size = {max_width, box_height}
        }, &help_footer_str, BOX_TITLE_BOTTOM_RIGHT);

        // draw hints inside the box
        for(int i = 0; i < line_count; i++){
            uint8_t hint_text[max_size] = {};
            utf8_str_concat(hint_text, HOTKEY_HINTS.hints[i].key);
            utf8_str_concat(hint_text, u8" ");
            utf8_str_concat(hint_text, HOTKEY_HINTS.hints[i].hint);
            screen_set_utf8_str(
            	screen,
            	BORDER + PADDING,
            	box_y + BORDER + PADDING + i,
            	hint_text
        	);
        }
    }
}


private void tui_render(void){
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
			tui_write_color(next_cell->text_format, next_cell->fg_color, next_cell->bg_color);
			if(next_cell->display_width == 0 || next_cell->bytes_used == 0){
				tui_write(" ");
			}else{
				tui_write_bytes(next_cell->bytes, next_cell->bytes_used);
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

private bool tui_process_input_hotkeys(InputEvent input_event){
	//process hotkeys
	if(input_event.input_type != INPUT_KEY) return false;
	//check if key is registered and execute its action
	//NOTE: we check in reverse order so that if there are
	//      any duplicate keys, we only execute the last one
	for(int k = HOTKEYS.total - 1; k >= 0; k--){
		auto hkey = HOTKEYS.hotkeys[k];
		if(input_event.key_event.key != hkey.key) continue;

		bool ctrl  = (hkey.mod_keys & KEY_MOD_CTRL)  != 0;
		bool alt   = (hkey.mod_keys & KEY_MOD_ALT)   != 0;
		bool shift = (hkey.mod_keys & KEY_MOD_SHIFT) != 0;

		if(input_event.key_event.ctrl  != ctrl)  continue;
		if(input_event.key_event.alt   != alt)   continue;
		if(input_event.key_event.shift != shift) continue;

		hkey.action();
		return true;
	}
	return false;
}

private bool tui_process_input_resize(InputEvent input_event){
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
	const double frame_time = 1.0 / APP_STATE.fps;

	// tui_set_resize_callback(tui_resize);
	tui_init(); //platform init
	tui_clear();

	//init pages
	for(int i = 0; i < PAGE_ROUTES.routes_count; i++){
		//TODO: possible optimization, delay until first page visit
		PAGE_ROUTES.routes[i].page->init();
	}

#ifdef TUI_WINDOWS
	//windos is a naught ynaught boy
	Sleep(100);
	emit_resize_event();
#endif //TUI_WINDOWS

	while(!APP_STATE.exit){
		// RESET HOTKEYS:
		// we do this every frame to make it more convenient to the user
		tui_reset_hotkeys();

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
		active_page->process(APP_STATE.frame_delta);

		//prepare page render and create widgets,
		//this is done here because widgets need to exist
		//before the input processing pass!
		tui_layout_prepare(&APP_STATE.next_screen);
		active_page->render(); // <- this actually creates the widgets

		//NOTE: The order is important! it is from deeper control upwards.
		//      Any of those functions returning true will consume the event
		//      And it won't be passed to the next "layer"
		tui_input_process(&tui_widget_focused_input);
		tui_input_process(active_page->input);
		tui_input_process(&tui_process_input_hotkeys);

		screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK); //reset format

		tui_layout_render();
		tui_render_header(&APP_STATE.next_screen);
		tui_render_hotkeys(&APP_STATE.next_screen);

		//render the TUI diff to the screen
		tui_render();

		//NOTE: we handle this at the end because it can destroy the screens!
		tui_input_process(&tui_process_input_resize);
	}
	screen_free(&APP_STATE.curr_screen);
	screen_free(&APP_STATE.next_screen);
	tui_close();
}

void tui_navigate_to(const char *page_id){
	int page_index  = 0;
	bool page_found = false;
	for(; page_index < PAGE_ROUTES.routes_count; page_index++){
		page_found = strcmp(PAGE_ROUTES.routes[page_index].page_id, page_id) == 0;
		if(page_found) break;
	}
	assert(page_found); // did you try to navigate to a page that doesn't exist?
	assert(NAV_HISTORY.count < TUI_NAV_HISTORY_MAX - 1); //stack overflow
	NAV_HISTORY.stack[NAV_HISTORY.count++] = page_index;
	tui_layout_reset();
}

void tui_navigate_back(void){
	if (NAV_HISTORY.count <= 1) return; //check 1 because root is part of history!
	NAV_HISTORY.count--;
	tui_layout_reset();
}

Page *tui_get_curr_page(){
	assert(NAV_HISTORY.count > 0); //There's no pages in the history, don't forget to init!
	int curr_page_index = NAV_HISTORY.stack[NAV_HISTORY.count - 1];
	return PAGE_ROUTES.routes[curr_page_index].page;
}

void tui_quit(void){
	APP_STATE.exit = true;
}

//aux --------------------------------------------------------------------------

void tui_move_to(int x, int y){
	tui_write_format("\033[%d;%dH", y, x);
}

void tui_clear(){
	tui_write(
		"\033[2J\033[H" // limpia la pantalla y mueve el cursor a 0,0
		"\033[3J" 		// limpia el scrollback
	);
}


#endif //TUI_IMPL
#endif //TUI

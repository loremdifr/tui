#ifndef TUI
#define TUI

#include <stdint.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>

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

#define TUI_WIDGET_INPUT_TEXT_IMPL
#include "tui_widget_input_text.h"

#define TUI_WIDGET_SPINNER_IMPL
#include "tui_widget_spinner.h"

// Pages and nav ---------------------------------------------------------------

typedef void (*InitFunction   )(void);
typedef bool (*InputFunction  )(InputEvent input_event);
typedef void (*ProcessFunction)(float delta_time);
typedef void (*RenderFunction )(void);

typedef struct {
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

// write color + text format -----------------------------

constexpr uint8_t FORMAT_PARAMS_MAX = 8;
typedef struct{
    uint8_t params[FORMAT_PARAMS_MAX];
    char    str[80];
    uint8_t used;
} FormatParams;
private FormatParams FORMAT_PARAMS = {};
private inline void format_params_push(uint8_t param){
    FORMAT_PARAMS.params[FORMAT_PARAMS.used++] = param;
}
private inline void format_params_reset(){
    FORMAT_PARAMS.str[0] = '\0';
    FORMAT_PARAMS.used = 0;
}

private inline void tui_write_color(TextFormat text_format, Color fg_color, Color bg_color){
	static const char start[]     = "\033[";
    static const char separator[] = ";";
    static const char end[]       = "m";

    //formato
    if(text_format == NORMAL){
        format_params_push(0);
    }else{
        if(text_format & BOLD)      format_params_push(1);
        if(text_format & ITALIC)    format_params_push(3);
        if(text_format & UNDERLINE) format_params_push(4);
        if(text_format & BLINKING)  format_params_push(5);
    }

    //color frente
    switch (fg_color) {
    	case COLOR_DEFAULT:
        case COLOR_WHITE:   format_params_push(39); break;
        case COLOR_BLACK:   format_params_push(30); break;
        case COLOR_RED:     format_params_push(31); break;
        case COLOR_GREEN:   format_params_push(32); break;
        case COLOR_BLUE:    format_params_push(34); break;
        case COLOR_YELLOW:  format_params_push(33); break;
        case COLOR_MAGENTA: format_params_push(95); break;
        case COLOR_GRAY:    format_params_push(90); break;
        case COLOR_CYAN:    format_params_push(36); break;
        default: assert(false);

    }

    //color fondo
    switch(bg_color){
        case COLOR_WHITE:   format_params_push(47);  break;
    	case COLOR_DEFAULT:
        case COLOR_BLACK:   format_params_push(49);  break;
        case COLOR_RED:     format_params_push(41);  break;
        case COLOR_GREEN:   format_params_push(42);  break;
        case COLOR_BLUE:    format_params_push(44);  break;
        case COLOR_YELLOW:  format_params_push(33);  break;
        case COLOR_MAGENTA: format_params_push(105); break;
        case COLOR_GRAY:    format_params_push(100); break;
        case COLOR_CYAN:    format_params_push(46);  break;
        default: assert(false);
    }

    //concatenar formato
    int terminator_pos = sprintf(FORMAT_PARAMS.str, "%s", start);
    char *next_str     = FORMAT_PARAMS.str + terminator_pos;
    for(size_t i = 0; i < FORMAT_PARAMS.used; i++){
        //ultimo param no usa el separator
        if(i == FORMAT_PARAMS.used - 1){
            next_str += sprintf(next_str, "%d%s", FORMAT_PARAMS.params[i], end);
        }else{
            next_str += sprintf(next_str, "%d%s", FORMAT_PARAMS.params[i], separator);
        }
    }

    tui_write(FORMAT_PARAMS.str);
    format_params_reset();
}


//TUI loop ---------------------------------------------------------------------

private void tui_render(void){
	//display diff of screen buffers
	for(int x = 0; x < APP_STATE.curr_screen.size.x; x++){
		for(int y = 0; y < APP_STATE.curr_screen.size.y; y++){

			auto curr_cell = screen_get(&APP_STATE.curr_screen, x, y);
			auto next_cell = screen_get(&APP_STATE.next_screen, x, y);
			if(memcmp(curr_cell, next_cell, sizeof(Cell)) == 0){
				continue;
			}
			tui_move_to(x + 1, y + 1); //we add 1 because terminal pos is 1 based
			tui_write_color(next_cell->text_format, next_cell->fg_color, next_cell->bg_color);
			if(next_cell->bytes_used > 0){
				tui_write_bytes(next_cell->bytes, next_cell->bytes_used);
			}else{
				tui_write(" ");
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

private void tui_reset_hotkeys(void){
	HOTKEYS.total = 0;
	HOTKEY_HINTS.total = 0;

	//arrows to select widgets
	tui_register_key(KEY_LEFT,   KEY_MOD_NONE,  &tui_cursor_prev_widget);
	tui_register_key(KEY_RIGHT,  KEY_MOD_NONE,  &tui_cursor_next_widget);
	tui_register_key(KEY_UP,     KEY_MOD_NONE,  &tui_cursor_prev_widget);
	tui_register_key(KEY_DOWN,   KEY_MOD_NONE,  &tui_cursor_next_widget);
	tui_register_key_hint(u8"[↑] [↓]", u8"Select");

	//enter to activate selected widget
	//TODO: if this is here--- shouldnt we have ENTER as an activate widget key??
	//      and not in the widget itself?
	tui_register_key_hint(u8"[ENTER]", u8"OK");
	tui_register_key_hint(u8"[TAB] [SHIFT+TAB]", u8"Switch Panel");

	//tab to select panels
	//TODO: this should be contingent on there being more than 1 panel
	tui_register_key(KEY_TAB,    KEY_MOD_NONE,  &tui_cursor_next_panel);
	tui_register_key(KEY_TAB,    KEY_MOD_SHIFT, &tui_cursor_prev_panel);

	//esc to navigate back
	//TODO: this should be contingent on there even being a back in the nav
	tui_register_key(KEY_ESCAPE, KEY_MOD_NONE,  &tui_navigate_back);
	tui_register_key_hint(u8"[ESC]", u8"Back");
}

private void tui_render_hotkeys(Screen *screen){
	//TODO: if we're going over the screen length,
	//      truncate the last one and add a way to "see more"
	screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);

	// como se si me pase?
	// si al concatenar, es mas grande de lo esperado, me pasé.
	// en cuyo caso:
	// 	repetir
	// 		vuelvo un paso atras.
	// 		intento poner el texto de ayuda
	// 		si puedo break

	auto separator = u8" ● ";
	const auto max_width = screen->size.w;
	uint8_t utf8_str[max_width] = {};

	for(int i = 0; i < HOTKEY_HINTS.total; i++){
		HotkeyHint hkey = HOTKEY_HINTS.hints[i];
		//TODO: max_width here is characters whereas expected
		//      limit in utf8_str_concat_max is in "bytes"!
		utf8_str_concat_max(utf8_str, hkey.key,  max_width);
		utf8_str_concat_max(utf8_str, u8" ",     max_width);
		utf8_str_concat_max(utf8_str, hkey.hint, max_width);

		if(i != HOTKEY_HINTS.total - 1){
			utf8_str_concat_max(utf8_str, separator, max_width);
		}
	}
	screen_set_utf8_str(screen, 0, screen->size.h, utf8_str);
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
		//TODO: check modifiers
		// if(input_event.key_event.shift != hkey.key) continue;
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
		tui_layout_prepare(&APP_STATE.next_screen, active_page->layout);
		active_page->render(); // <- this actually creates the widgets

		//NOTE: The order is important! it is from deeper control upwards.
		//      Any of those functions returning true will consume the event
		//      And it won't be passed to the next "layer"
		tui_input_process(&tui_widget_focused_input);
		tui_input_process(active_page->input);
		tui_input_process(&tui_process_input_hotkeys);

		screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK); //reset format

		tui_layout_render();
		tui_render_hotkeys(&APP_STATE.next_screen);
		//TODO: render window title, menu, etc..

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
}

void tui_navigate_back(void){
	//TODO: if there's only a single page, ask to quit app?
	if (NAV_HISTORY.count <= 0) return;
	NAV_HISTORY.count--;
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

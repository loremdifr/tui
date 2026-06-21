#ifndef TUI_HOTKEYS
#define TUI_HOTKEYS

#include <stdint.h>

#include "tui_platform.h"
#include "tui_navigation.h"

// API -----
void tui_register_key(Key key, ModKeys mod_keys, FunctionPointer action);
void tui_register_key_hint(const uint8_t *key, const uint8_t *hint);


void tui_quit(void); // this is actually implemented in tui.h but declared here
					 // to simplify the dependancies

// IMPL ------------------------------------------------------------------------
#ifdef TUI_HOTKEYS_IMPL

#define TUI_HOTKEYS_MAX 64

typedef struct {
	Key             key;
	ModKeys         mod_keys;
	FunctionPointer action;
} _Hotkey;

typedef struct {
	_Hotkey  hotkeys[TUI_HOTKEYS_MAX];
	uint8_t  total;
} _Hotkeys;

static _Hotkeys HOTKEYS = {};

void tui_register_key(Key key, ModKeys mod_keys, FunctionPointer action){
	assert(HOTKEYS.total <= TUI_HOTKEYS_MAX);
	HOTKEYS.hotkeys[HOTKEYS.total++] = (_Hotkey){
		.key      = key,
		.mod_keys = mod_keys,
		.action   = action,
	};
}

typedef struct {
	const uint8_t *key;
	const uint8_t *hint;
} _HotkeyHint;

typedef struct {
	_HotkeyHint hints[TUI_HOTKEYS_MAX];
	uint8_t    total;
} _HotkeyHints;

static _HotkeyHints HOTKEY_HINTS = {};

void tui_register_key_hint(const uint8_t *key, const uint8_t *hint){
	HOTKEY_HINTS.hints[HOTKEY_HINTS.total++] = (_HotkeyHint){
        .key  = key,
        .hint = hint,
	};
}

// TUI HOTKEY KEYS PANEL ---------------------------------------
static bool HOTKEY_HELP_SHOW    = false;
static bool HOTKEY_HELP_ENABLED = false;

static void _tui_toggle_help(void){
	if(HOTKEY_HELP_ENABLED){
    	HOTKEY_HELP_SHOW = !HOTKEY_HELP_SHOW;
	}else {
		HOTKEY_HELP_SHOW = false;
	}
}

static void _tui_reset_hotkeys(void){
	HOTKEYS.total = 0;
	HOTKEY_HINTS.total = 0;

	//arrows to select widgets
	tui_register_key(KEY_LEFT,   KEY_MOD_NONE,  &_tui_cursor_prev_widget);
	tui_register_key(KEY_RIGHT,  KEY_MOD_NONE,  &_tui_cursor_next_widget);
	tui_register_key(KEY_UP,     KEY_MOD_NONE,  &_tui_cursor_prev_widget);
	tui_register_key(KEY_DOWN,   KEY_MOD_NONE,  &_tui_cursor_next_widget);
	tui_register_key_hint(I18N_HINT_SELECT_KEY, I18N_HINT_SELECT_TEXT);

	//enter to activate selected widget
	//TODO: if this is here--- shouldnt we have ENTER as an activate widget key??
	//      and not in the widget itself?
	tui_register_key_hint(I18N_HINT_OK_KEY, I18N_HINT_OK_TEXT);

	//scroll
	auto panel = _tui_get_panel_focused();
	if(_tui_is_panel_scrollable(panel)){
		tui_register_key(KEY_PAGEUP,    KEY_MOD_NONE,  &tui_panel_scroll_up);
		tui_register_key(KEY_PAGEDOWN,  KEY_MOD_NONE,  &tui_panel_scroll_down);
		tui_register_key_hint(I18N_HINT_SCROLL_KEY, I18N_HINT_SCROLL_TEXT);
	}

	//tab to select panels
	auto layer = _tui_get_layer_focused();
	if(layer->panel_count > 1){
		tui_register_key_hint(I18N_HINT_SWITCH_PANEL_KEY, I18N_HINT_SWITCH_PANEL_TEXT);
		tui_register_key(KEY_TAB,  KEY_MOD_NONE,  &_tui_cursor_next_panel);
		tui_register_key(KEY_TAB,  KEY_MOD_SHIFT, &_tui_cursor_prev_panel);
	}

	//esc to navigate back
	if(NAV_HISTORY.count > 1){
		tui_register_key(KEY_ESCAPE, KEY_MOD_NONE,  &tui_navigate_back);
		tui_register_key_hint(I18N_HINT_BACK_KEY, I18N_HINT_BACK_TEXT);
	}

	//ALT+Q to Close the App!
	tui_register_key(KEY_Q, KEY_MOD_ALT,  &tui_quit);
	tui_register_key_hint(I18N_HINT_QUIT_KEY, I18N_HINT_QUIT_TEXT);
	tui_register_key(KEY_ESCAPE, KEY_MOD_ALT, &tui_quit);

	//? to toggle help overlay
	//NOTE: registered here instead of tui_render_hotkeys because hotkeys
	//      are reset every frame BEFORE input processing.
	//      we register the hint conditionally later.
	//      this does mean this key is always registered even when not needed
	//      but it effectively does nothing when not needed.
	tui_register_key((Key)'?', KEY_MOD_NONE, &_tui_toggle_help);
	tui_register_key(KEY_F1,   KEY_MOD_ALT, &_tui_toggle_help);
}

static void _tui_render_hotkeys(Screen *screen){
    screen_format(NORMAL, COLOR_FG_SECONDARY, COLOR_BG_SECONDARY);

    size_t max_width = screen->size.w; //in characters/cells
    size_t max_size  = max_width * 4 + 1; //4 bytes for u8 + terminator
    size_t included_hints_count = 0;
    size_t hint_widths[TUI_HOTKEYS_MAX] = {}; //stores TOTAL width at each hint
    uint8_t display_str[max_size] = {};

    //first try to fill with all the hotkey hints, and see if they fit
    for(int i = 0; i < HOTKEY_HINTS.total; i++){
        _HotkeyHint hkey = HOTKEY_HINTS.hints[i];
        hint_widths[included_hints_count] = strlen(
        	(const char*)display_str
    	);

        //create current hint text
        uint8_t hint_text[max_size] = {};
        utf8_str_concat(hint_text, hkey.key);
        utf8_str_concat(hint_text, EMPTY_U8);
        utf8_str_concat(hint_text, hkey.hint);
        if(i != HOTKEY_HINTS.total - 1){
        	//last hotkey doesnt have separator
        	//TODO: use the max_width instead?
        	screen_format(NORMAL, COLOR_FG_INFO, COLOR_BG_SECONDARY);
            utf8_str_concat(hint_text, KEY_HINT_SEPARATOR);
            screen_format(NORMAL, COLOR_FG_SECONDARY, COLOR_BG_SECONDARY);
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
	    size_t help_width = utf8_str_display_width(I18N_HELP_OPEN);

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
        	screen->size.h, I18N_HELP_OPEN
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

        tui_draw_rect(screen, EMPTY_U8, (rect2i){
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
            utf8_str_concat(hint_text, EMPTY_U8);
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

static bool _tui_process_input_hotkeys(InputEvent input_event){
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


#endif //TUI_HOTKEYS_IMPL
#endif //TUI_HOTKEYS

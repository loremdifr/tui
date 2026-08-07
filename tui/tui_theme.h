#ifndef TUI_THEME
#define TUI_THEME

 #include "tui_screen.h"

auto example_theme = (Theme){
	.name = "Example Theme",
	.colors = {
		[COLOR_TEXT]        = {.fg=COLOR_HEX(0xeeeeee), .bg=COLOR_HEX(0x222222)},
		[COLOR_TEXT_FOCUS]  = {.fg=COLOR_HEX(0x222222), .bg=COLOR_HEX(0xeeeeee)},
		[COLOR_PANEL]       = {.fg=COLOR_HEX(0x9999ee), .bg=COLOR_HEX(0x222222)},
		[COLOR_PANEL_FOCUS] = {.fg=COLOR_HEX(0xeeeeee), .bg=COLOR_HEX(0x222222)},
		[COLOR_PRIMARY]     = {.fg=COLOR_HEX(0x22aaff), .bg=COLOR_HEX(0x222222)},
		[COLOR_SECONDARY]   = {.fg=COLOR_HEX(0x9999ee), .bg=COLOR_HEX(0x222222)},
		[COLOR_SUCCESS]     = {.fg=COLOR_HEX(0x22cc22), .bg=COLOR_HEX(0x222222)},
		[COLOR_INFO]        = {.fg=COLOR_HEX(0xaaaaff), .bg=COLOR_HEX(0x222222)},
		[COLOR_WARNING]     = {.fg=COLOR_HEX(0xffcc22), .bg=COLOR_HEX(0x222222)},
		[COLOR_DANGER]      = {.fg=COLOR_HEX(0xff4422), .bg=COLOR_HEX(0x222222)},
	}
};

#endif //TUI_THEME
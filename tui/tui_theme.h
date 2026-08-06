#ifndef TUI_THEME
#define TUI_THEME

 #include "tui_utils.h"
#include <stdint.h>

typedef struct{
	vec3i fg;
	vec3i bg;
} ColorPair;

typedef enum{
	COLOR_TEXT,
	COLOR_TEXT_FOCUS,
	COLOR_PANEL,
	COLOR_PANEL_FOCUS,
	COLOR_PRIMARY,
	COLOR_SECONDARY,
	COLOR_SUCCESS,
	COLOR_INFO,
	COLOR_WARNING,
	COLOR_DANGER,
	THEME_COLOR_COUNT,
} ThemeColor;

typedef struct {
	char *name;
	ColorPair colors[THEME_COLOR_COUNT];
} Theme;

vec3i color_hex(uint32_t hex){
	return (vec3i){
		.red   = (hex >> 16) & 0xFF,
		.green = (hex >> 8)  & 0xFF,
		.blue  = (hex >> 0)  & 0xFF,
	};
}

auto example_theme = (Theme){
	.name = "Example Theme",
	.colors = {
		[COLOR_PRIMARY]     = {.fg=color_hex(0xdddddd), .bg=color_hex(0x222222)},
		[COLOR_TEXT]        = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_TEXT_FOCUS]  = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_PANEL]       = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_PANEL_FOCUS] = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_PRIMARY]     = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_SECONDARY]   = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_SUCCESS]     = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_INFO]        = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_WARNING]     = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
		[COLOR_DANGER]      = {.fg=color_hex(0x111111), .bg=color_hex(0x000000)},
	}
};

#endif //TUI_THEME
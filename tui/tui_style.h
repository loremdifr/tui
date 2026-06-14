#ifndef TUI_STYLE
#define TUI_STYLE

#define EMPTY_CHAR ' '
#define EMPTY_U8 u8" "
#define NAV_HISTORY_SEPARATOR u8" > "

// HOTKEY HINTS
#define KEY_HINT_SEPARATOR u8" • "

// BOX CHARACTERS --------------------------------------------------------------
#define BOX    u8"▢"
#define BOX_H  u8"─"
#define BOX_V  u8"│"
#define BOX_TL u8"╭"
#define BOX_TR u8"╮"
#define BOX_BL u8"╰"
#define BOX_BR u8"╯"
#define BOX_VR u8"├"
#define BOX_VL u8"┤"
#define BOX_HB u8"┬"
#define BOX_HT u8"┴"
#define BOX_HV u8"┼"
#define BOX_TITLE_CAP_LEFT  u8"╸"
#define BOX_TITLE_CAP_RIGHT u8"╺"

// SCROLLBARS ------------------------------------------------------------------

#define SCROLL_CAP_TOP     u8"╵"
#define SCROLL_CAP_BOTTOM  u8"╷"
#define SCROLL_V_BG        u8"▋"
#define SCROLL_V_KNOB      u8"▋"
#define SCROLL_CAP_LEFT    u8"╴"
#define SCROLL_CAP_RIGHT   u8"╶"
#define SCROLL_H_BG        u8"▂"
#define SCROLL_H_KNOB      u8"▂"

// BRAILLE ---------------------------------------------------------------------

// COLORS ----------------------------------------------------------------------
#define COLOR_FG_PANEL            COLOR_GRAY
#define COLOR_FG_PANEL_FOCUS   	  COLOR_WHITE
#define COLOR_FG_TEXT			  COLOR_WHITE
#define COLOR_FG_PRIMARY		  COLOR_CYAN
#define COLOR_FG_SECONDARY  	  COLOR_DARK_WHITE
#define COLOR_FG_SUCCESS		  COLOR_GREEN
#define COLOR_FG_DANGER			  COLOR_RED
#define COLOR_FG_WARNING          COLOR_YELLOW
#define COLOR_FG_INFO             COLOR_CYAN

#define COLOR_BG_PANEL            COLOR_BLACK
#define COLOR_BG_PANEL_FOCUS      COLOR_BLACK
#define COLOR_BG_TEXT             COLOR_BLACK
#define COLOR_BG_PRIMARY          COLOR_DARK_CYAN
#define COLOR_BG_SECONDARY        COLOR_BLACK
#define COLOR_BG_SUCCESS          COLOR_DARK_GREEN
#define COLOR_BG_DANGER			  COLOR_DARK_RED
#define COLOR_BG_WARNING		  COLOR_DARK_YELLOW
#define COLOR_BG_INFO			  COLOR_DARK_BLUE

#endif //TUI_STYLE

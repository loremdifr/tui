#ifndef TUI_WIDGET_FIGLET
#define TUI_WIDGET_FIGLET

#include "tui_string.h"
#include "tui_layout.h"
#include "tui_screen.h"

// The Standard figlet font (http://www.figlet.org)
// Height: 6 lines per character
// ASCII printable characters (32-126)

typedef struct {
    const uint8_t *text;
    Lines          lines;
} WidgetFigletData;

#define tui_widget_figlet(text) tui_widget_figlet_((const uint8_t*)(text))
void tui_widget_figlet_(const uint8_t *text);

#ifdef TUI_WIDGET_FIGLET_IMPL

// Figlet font character art for ASCII 32-126
// Each character has 6 rows (strings), padded to consistent width
private const uint8_t *FIGLET_FONT_CHARS[95][6] = {
    // 32 ( )
    {
        (const uint8_t*)"  ",
        (const uint8_t*)"  ",
        (const uint8_t*)"  ",
        (const uint8_t*)"  ",
        (const uint8_t*)"  ",
        (const uint8_t*)"  ",
    },
    // 33 (!)
    {
        (const uint8_t*)"  _ ",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" |_|",
        (const uint8_t*)" (_)",
        (const uint8_t*)"    ",
    },
    // 34 (")
    {
        (const uint8_t*)"  _ _ ",
        (const uint8_t*)" ( | )",
        (const uint8_t*)"  V V ",
        (const uint8_t*)"      ",
        (const uint8_t*)"      ",
        (const uint8_t*)"      ",
    },
    // 35 (#)
    {
        (const uint8_t*)"    _  _   ",
        (const uint8_t*)"  _| || |_ ",
        (const uint8_t*)" |_  ..  _|",
        (const uint8_t*)" |_      _|",
        (const uint8_t*)"   |_||_|  ",
        (const uint8_t*)"           ",
    },
    // 36 ($)
    {
        (const uint8_t*)"   _  ",
        (const uint8_t*)"  | | ",
        (const uint8_t*)" / __)",
        (const uint8_t*)" \\__ \\",
        (const uint8_t*)" (   /",
        (const uint8_t*)"  |_| ",
    },
    // 37 (%)
    {
        (const uint8_t*)"  _  __",
        (const uint8_t*)" (_)/ /",
        (const uint8_t*)"   / / ",
        (const uint8_t*)"  / /_ ",
        (const uint8_t*)" /_/(_)",
        (const uint8_t*)"       ",
    },
    // 38 (&)
    {
        (const uint8_t*)"   ___   ",
        (const uint8_t*)"  ( _ )  ",
        (const uint8_t*)"  / _ \\/\\",
        (const uint8_t*)" | (_>  <",
        (const uint8_t*)"  \\___/\\/",
        (const uint8_t*)"         ",
    },
    // 39 (')
    {
        (const uint8_t*)"  _ ",
        (const uint8_t*)" ( )",
        (const uint8_t*)" |/ ",
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
    },
    // 40 (()
    {
        (const uint8_t*)"   __",
        (const uint8_t*)"  / /",
        (const uint8_t*)" | | ",
        (const uint8_t*)" | | ",
        (const uint8_t*)" | | ",
        (const uint8_t*)"  \\_\\",
    },
    // 41 ())
    {
        (const uint8_t*)" __  ",
        (const uint8_t*)" \\ \\ ",
        (const uint8_t*)"  | |",
        (const uint8_t*)"  | |",
        (const uint8_t*)"  | |",
        (const uint8_t*)" /_/ ",
    },
    // 42 (*)
    {
        (const uint8_t*)"       ",
        (const uint8_t*)" __/\\__",
        (const uint8_t*)" \\    /",
        (const uint8_t*)" /_  _\\",
        (const uint8_t*)"   \\/  ",
        (const uint8_t*)"       ",
    },
    // 43 (+)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"    _   ",
        (const uint8_t*)"  _| |_ ",
        (const uint8_t*)" |_   _|",
        (const uint8_t*)"   |_|  ",
        (const uint8_t*)"        ",
    },
    // 44 (,)
    {
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
        (const uint8_t*)"  _ ",
        (const uint8_t*)" ( )",
        (const uint8_t*)" |/ ",
    },
    // 45 (-)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"        ",
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" |_____|",
        (const uint8_t*)"        ",
        (const uint8_t*)"        ",
    },
    // 46 (.)
    {
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
        (const uint8_t*)"  _ ",
        (const uint8_t*)" (_)",
        (const uint8_t*)"    ",
    },
    // 47 (/)
    {
        (const uint8_t*)"     __",
        (const uint8_t*)"    / /",
        (const uint8_t*)"   / / ",
        (const uint8_t*)"  / /  ",
        (const uint8_t*)" /_/   ",
        (const uint8_t*)"       ",
    },
    // 48 (0)
    {
        (const uint8_t*)"   ___  ",
        (const uint8_t*)"  / _ \\ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\___/ ",
        (const uint8_t*)"        ",
    },
    // 49 (1)
    {
        (const uint8_t*)"  _ ",
        (const uint8_t*)" / |",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" |_|",
        (const uint8_t*)"    ",
    },
    // 50 (2)
    {
        (const uint8_t*)"  ____  ",
        (const uint8_t*)" |___ \\ ",
        (const uint8_t*)"   __) |",
        (const uint8_t*)"  / __/ ",
        (const uint8_t*)" |_____|",
        (const uint8_t*)"        ",
    },
    // 51 (3)
    {
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" |___ / ",
        (const uint8_t*)"   |_ \\ ",
        (const uint8_t*)"  ___) |",
        (const uint8_t*)" |____/ ",
        (const uint8_t*)"        ",
    },
    // 52 (4)
    {
        (const uint8_t*)"  _  _   ",
        (const uint8_t*)" | || |  ",
        (const uint8_t*)" | || |_ ",
        (const uint8_t*)" |__   _|",
        (const uint8_t*)"    |_|  ",
        (const uint8_t*)"         ",
    },
    // 53 (5)
    {
        (const uint8_t*)"  ____  ",
        (const uint8_t*)" | ___| ",
        (const uint8_t*)" |___ \\ ",
        (const uint8_t*)"  ___) |",
        (const uint8_t*)" |____/ ",
        (const uint8_t*)"        ",
    },
    // 54 (6)
    {
        (const uint8_t*)"   __   ",
        (const uint8_t*)"  / /_  ",
        (const uint8_t*)" | '_ \\ ",
        (const uint8_t*)" | (_) |",
        (const uint8_t*)"  \\___/ ",
        (const uint8_t*)"        ",
    },
    // 55 (7)
    {
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" |___  |",
        (const uint8_t*)"    / / ",
        (const uint8_t*)"   / /  ",
        (const uint8_t*)"  /_/   ",
        (const uint8_t*)"        ",
    },
    // 56 (8)
    {
        (const uint8_t*)"   ___  ",
        (const uint8_t*)"  ( _ ) ",
        (const uint8_t*)"  / _ \\ ",
        (const uint8_t*)" | (_) |",
        (const uint8_t*)"  \\___/ ",
        (const uint8_t*)"        ",
    },
    // 57 (9)
    {
        (const uint8_t*)"   ___  ",
        (const uint8_t*)"  / _ \\ ",
        (const uint8_t*)" | (_) |",
        (const uint8_t*)"  \\__, |",
        (const uint8_t*)"    /_/ ",
        (const uint8_t*)"        ",
    },
    // 58 (:)
    {
        (const uint8_t*)"    ",
        (const uint8_t*)"  _ ",
        (const uint8_t*)" (_)",
        (const uint8_t*)"  _ ",
        (const uint8_t*)" (_)",
        (const uint8_t*)"    ",
    },
    // 59 (;)
    {
        (const uint8_t*)"    ",
        (const uint8_t*)"  _ ",
        (const uint8_t*)" (_)",
        (const uint8_t*)"  _ ",
        (const uint8_t*)" ( )",
        (const uint8_t*)" |/ ",
    },
    // 60 (<)
    {
        (const uint8_t*)"   __",
        (const uint8_t*)"  / /",
        (const uint8_t*)" / / ",
        (const uint8_t*)" \\ \\ ",
        (const uint8_t*)"  \\_\\",
        (const uint8_t*)"     ",
    },
    // 61 (=)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" |_____|",
        (const uint8_t*)" |_____|",
        (const uint8_t*)"        ",
        (const uint8_t*)"        ",
    },
    // 62 (>)
    {
        (const uint8_t*)" __  ",
        (const uint8_t*)" \\ \\ ",
        (const uint8_t*)"  \\ \\",
        (const uint8_t*)"  / /",
        (const uint8_t*)" /_/ ",
        (const uint8_t*)"     ",
    },
    // 63 (?)
    {
        (const uint8_t*)"  ___ ",
        (const uint8_t*)" |__ \\",
        (const uint8_t*)"   / /",
        (const uint8_t*)"  |_| ",
        (const uint8_t*)"  (_) ",
        (const uint8_t*)"      ",
    },
    // 64 (@)
    {
        (const uint8_t*)"    ____  ",
        (const uint8_t*)"   / __ \\ ",
        (const uint8_t*)"  / / _` |",
        (const uint8_t*)" | | (_| |",
        (const uint8_t*)"  \\ \\__,_|",
        (const uint8_t*)"   \\____/ ",
    },
    // 65 (A)
    {
        (const uint8_t*)"     _    ",
        (const uint8_t*)"    / \\   ",
        (const uint8_t*)"   / _ \\  ",
        (const uint8_t*)"  / ___ \\ ",
        (const uint8_t*)" /_/   \\_\\",
        (const uint8_t*)"          ",
    },
    // 66 (B)
    {
        (const uint8_t*)"  ____  ",
        (const uint8_t*)" | __ ) ",
        (const uint8_t*)" |  _ \\ ",
        (const uint8_t*)" | |_) |",
        (const uint8_t*)" |____/ ",
        (const uint8_t*)"        ",
    },
    // 67 (C)
    {
        (const uint8_t*)"   ____ ",
        (const uint8_t*)"  / ___|",
        (const uint8_t*)" | |    ",
        (const uint8_t*)" | |___ ",
        (const uint8_t*)"  \\____|",
        (const uint8_t*)"        ",
    },
    // 68 (D)
    {
        (const uint8_t*)"  ____  ",
        (const uint8_t*)" |  _ \\ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)" |____/ ",
        (const uint8_t*)"        ",
    },
    // 69 (E)
    {
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" | ____|",
        (const uint8_t*)" |  _|  ",
        (const uint8_t*)" | |___ ",
        (const uint8_t*)" |_____|",
        (const uint8_t*)"        ",
    },
    // 70 (F)
    {
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" |  ___|",
        (const uint8_t*)" | |_   ",
        (const uint8_t*)" |  _|  ",
        (const uint8_t*)" |_|    ",
        (const uint8_t*)"        ",
    },
    // 71 (G)
    {
        (const uint8_t*)"   ____ ",
        (const uint8_t*)"  / ___|",
        (const uint8_t*)" | |  _ ",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\____|",
        (const uint8_t*)"        ",
    },
    // 72 (H)
    {
        (const uint8_t*)"  _   _ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)" |  _  |",
        (const uint8_t*)" |_| |_|",
        (const uint8_t*)"        ",
    },
    // 73 (I)
    {
        (const uint8_t*)"  ___ ",
        (const uint8_t*)" |_ _|",
        (const uint8_t*)"  | | ",
        (const uint8_t*)"  | | ",
        (const uint8_t*)" |___|",
        (const uint8_t*)"      ",
    },
    // 74 (J)
    {
        (const uint8_t*)"      _ ",
        (const uint8_t*)"     | |",
        (const uint8_t*)"  _  | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\___/ ",
        (const uint8_t*)"        ",
    },
    // 75 (K)
    {
        (const uint8_t*)"  _  __",
        (const uint8_t*)" | |/ /",
        (const uint8_t*)" | ' / ",
        (const uint8_t*)" | . \\ ",
        (const uint8_t*)" |_|\\_\\",
        (const uint8_t*)"       ",
    },
    // 76 (L)
    {
        (const uint8_t*)"  _     ",
        (const uint8_t*)" | |    ",
        (const uint8_t*)" | |    ",
        (const uint8_t*)" | |___ ",
        (const uint8_t*)" |_____|",
        (const uint8_t*)"        ",
    },
    // 77 (M)
    {
        (const uint8_t*)"  __  __ ",
        (const uint8_t*)" |  \\/  |",
        (const uint8_t*)" | |\\/| |",
        (const uint8_t*)" | |  | |",
        (const uint8_t*)" |_|  |_|",
        (const uint8_t*)"         ",
    },
    // 78 (N)
    {
        (const uint8_t*)"  _   _ ",
        (const uint8_t*)" | \\ | |",
        (const uint8_t*)" |  \\| |",
        (const uint8_t*)" | |\\  |",
        (const uint8_t*)" |_| \\_|",
        (const uint8_t*)"        ",
    },
    // 79 (O)
    {
        (const uint8_t*)"   ___  ",
        (const uint8_t*)"  / _ \\ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\___/ ",
        (const uint8_t*)"        ",
    },
    // 80 (P)
    {
        (const uint8_t*)"  ____  ",
        (const uint8_t*)" |  _ \\ ",
        (const uint8_t*)" | |_) |",
        (const uint8_t*)" |  __/ ",
        (const uint8_t*)" |_|    ",
        (const uint8_t*)"        ",
    },
    // 81 (Q)
    {
        (const uint8_t*)"   ___  ",
        (const uint8_t*)"  / _ \\ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\__\\_\\",
        (const uint8_t*)"        ",
    },
    // 82 (R)
    {
        (const uint8_t*)"  ____  ",
        (const uint8_t*)" |  _ \\ ",
        (const uint8_t*)" | |_) |",
        (const uint8_t*)" |  _ < ",
        (const uint8_t*)" |_| \\_\\",
        (const uint8_t*)"        ",
    },
    // 83 (S)
    {
        (const uint8_t*)"  ____  ",
        (const uint8_t*)" / ___| ",
        (const uint8_t*)" \\___ \\ ",
        (const uint8_t*)"  ___) |",
        (const uint8_t*)" |____/ ",
        (const uint8_t*)"        ",
    },
    // 84 (T)
    {
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" |_   _|",
        (const uint8_t*)"   | |  ",
        (const uint8_t*)"   | |  ",
        (const uint8_t*)"   |_|  ",
        (const uint8_t*)"        ",
    },
    // 85 (U)
    {
        (const uint8_t*)"  _   _ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\___/ ",
        (const uint8_t*)"        ",
    },
    // 86 (V)
    {
        (const uint8_t*)" __     __",
        (const uint8_t*)" \\ \\   / /",
        (const uint8_t*)"  \\ \\ / / ",
        (const uint8_t*)"   \\ V /  ",
        (const uint8_t*)"    \\_/   ",
        (const uint8_t*)"          ",
    },
    // 87 (W)
    {
        (const uint8_t*)" __        __",
        (const uint8_t*)" \\ \\      / /",
        (const uint8_t*)"  \\ \\ /\\ / / ",
        (const uint8_t*)"   \\ V  V /  ",
        (const uint8_t*)"    \\_/\\_/   ",
        (const uint8_t*)"             ",
    },
    // 88 (X)
    {
        (const uint8_t*)" __  __",
        (const uint8_t*)" \\ \\/ /",
        (const uint8_t*)"  \\  / ",
        (const uint8_t*)"  /  \\ ",
        (const uint8_t*)" /_/\\_\\",
        (const uint8_t*)"       ",
    },
    // 89 (Y)
    {
        (const uint8_t*)" __   __",
        (const uint8_t*)" \\ \\ / /",
        (const uint8_t*)"  \\ V / ",
        (const uint8_t*)"   | |  ",
        (const uint8_t*)"   |_|  ",
        (const uint8_t*)"        ",
    },
    // 90 (Z)
    {
        (const uint8_t*)"  _____",
        (const uint8_t*)" |__  /",
        (const uint8_t*)"   / / ",
        (const uint8_t*)"  / /_ ",
        (const uint8_t*)" /____|",
        (const uint8_t*)"       ",
    },
    // 91 ([)
    {
        (const uint8_t*)"  __ ",
        (const uint8_t*)" | _|",
        (const uint8_t*)" | | ",
        (const uint8_t*)" | | ",
        (const uint8_t*)" | | ",
        (const uint8_t*)" |__|",
    },
    // 92 (\\)
    {
        (const uint8_t*)" __    ",
        (const uint8_t*)" \\ \\   ",
        (const uint8_t*)"  \\ \\  ",
        (const uint8_t*)"   \\ \\ ",
        (const uint8_t*)"    \\_\\",
        (const uint8_t*)"       ",
    },
    // 93 (])
    {
        (const uint8_t*)"  __ ",
        (const uint8_t*)" |_ |",
        (const uint8_t*)"  | |",
        (const uint8_t*)"  | |",
        (const uint8_t*)"  | |",
        (const uint8_t*)" |__|",
    },
    // 94 (^)
    {
        (const uint8_t*)"  /\\ ",
        (const uint8_t*)" |/\\|",
        (const uint8_t*)"     ",
        (const uint8_t*)"     ",
        (const uint8_t*)"     ",
        (const uint8_t*)"     ",
    },
    // 95 (_)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"        ",
        (const uint8_t*)"        ",
        (const uint8_t*)"        ",
        (const uint8_t*)"  _____ ",
        (const uint8_t*)" |_____|",
    },
    // 96 (`)
    {
        (const uint8_t*)"  _ ",
        (const uint8_t*)" ( )",
        (const uint8_t*)"  \\|",
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
        (const uint8_t*)"    ",
    },
    // 97 (a)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"   __ _ ",
        (const uint8_t*)"  / _` |",
        (const uint8_t*)" | (_| |",
        (const uint8_t*)"  \\__,_|",
        (const uint8_t*)"        ",
    },
    // 98 (b)
    {
        (const uint8_t*)"  _     ",
        (const uint8_t*)" | |__  ",
        (const uint8_t*)" | '_ \\ ",
        (const uint8_t*)" | |_) |",
        (const uint8_t*)" |_.__/ ",
        (const uint8_t*)"        ",
    },
    // 99 (c)
    {
        (const uint8_t*)"       ",
        (const uint8_t*)"   ___ ",
        (const uint8_t*)"  / __|",
        (const uint8_t*)" | (__ ",
        (const uint8_t*)"  \\___|",
        (const uint8_t*)"       ",
    },
    // 100 (d)
    {
        (const uint8_t*)"      _ ",
        (const uint8_t*)"   __| |",
        (const uint8_t*)"  / _` |",
        (const uint8_t*)" | (_| |",
        (const uint8_t*)"  \\__,_|",
        (const uint8_t*)"        ",
    },
    // 101 (e)
    {
        (const uint8_t*)"       ",
        (const uint8_t*)"   ___ ",
        (const uint8_t*)"  / _ \\",
        (const uint8_t*)" |  __/",
        (const uint8_t*)"  \\___|",
        (const uint8_t*)"       ",
    },
    // 102 (f)
    {
        (const uint8_t*)"   __ ",
        (const uint8_t*)"  / _|",
        (const uint8_t*)" | |_ ",
        (const uint8_t*)" |  _|",
        (const uint8_t*)" |_|  ",
        (const uint8_t*)"      ",
    },
    // 103 (g)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"   __ _ ",
        (const uint8_t*)"  / _` |",
        (const uint8_t*)" | (_| |",
        (const uint8_t*)"  \\__, |",
        (const uint8_t*)"  |___/ ",
    },
    // 104 (h)
    {
        (const uint8_t*)"  _     ",
        (const uint8_t*)" | |__  ",
        (const uint8_t*)" | '_ \\ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" |_| |_|",
        (const uint8_t*)"        ",
    },
    // 105 (i)
    {
        (const uint8_t*)"  _ ",
        (const uint8_t*)" (_)",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" |_|",
        (const uint8_t*)"    ",
    },
    // 106 (j)
    {
        (const uint8_t*)"    _ ",
        (const uint8_t*)"   (_)",
        (const uint8_t*)"   | |",
        (const uint8_t*)"   | |",
        (const uint8_t*)"  _/ |",
        (const uint8_t*)" |__/ ",
    },
    // 107 (k)
    {
        (const uint8_t*)"  _    ",
        (const uint8_t*)" | | __",
        (const uint8_t*)" | |/ /",
        (const uint8_t*)" |   < ",
        (const uint8_t*)" |_|\\_\\",
        (const uint8_t*)"       ",
    },
    // 108 (l)
    {
        (const uint8_t*)"  _ ",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" |_|",
        (const uint8_t*)"    ",
    },
    // 109 (m)
    {
        (const uint8_t*)"            ",
        (const uint8_t*)"  _ __ ___  ",
        (const uint8_t*)" | '_ ` _ \\ ",
        (const uint8_t*)" | | | | | |",
        (const uint8_t*)" |_| |_| |_|",
        (const uint8_t*)"            ",
    },
    // 110 (n)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"  _ __  ",
        (const uint8_t*)" | '_ \\ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" |_| |_|",
        (const uint8_t*)"        ",
    },
    // 111 (o)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"   ___  ",
        (const uint8_t*)"  / _ \\ ",
        (const uint8_t*)" | (_) |",
        (const uint8_t*)"  \\___/ ",
        (const uint8_t*)"        ",
    },
    // 112 (p)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"  _ __  ",
        (const uint8_t*)" | '_ \\ ",
        (const uint8_t*)" | |_) |",
        (const uint8_t*)" | .__/ ",
        (const uint8_t*)" |_|    ",
    },
    // 113 (q)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"   __ _ ",
        (const uint8_t*)"  / _` |",
        (const uint8_t*)" | (_| |",
        (const uint8_t*)"  \\__, |",
        (const uint8_t*)"     |_|",
    },
    // 114 (r)
    {
        (const uint8_t*)"       ",
        (const uint8_t*)"  _ __ ",
        (const uint8_t*)" | '__|",
        (const uint8_t*)" | |   ",
        (const uint8_t*)" |_|   ",
        (const uint8_t*)"       ",
    },
    // 115 (s)
    {
        (const uint8_t*)"      ",
        (const uint8_t*)"  ___ ",
        (const uint8_t*)" / __|",
        (const uint8_t*)" \\__ \\",
        (const uint8_t*)" |___/",
        (const uint8_t*)"      ",
    },
    // 116 (t)
    {
        (const uint8_t*)"  _   ",
        (const uint8_t*)" | |_ ",
        (const uint8_t*)" | __|",
        (const uint8_t*)" | |_ ",
        (const uint8_t*)"  \\__|",
        (const uint8_t*)"      ",
    },
    // 117 (u)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"  _   _ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\__,_|",
        (const uint8_t*)"        ",
    },
    // 118 (v)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)" __   __",
        (const uint8_t*)" \\ \\ / /",
        (const uint8_t*)"  \\ V / ",
        (const uint8_t*)"   \\_/  ",
        (const uint8_t*)"        ",
    },
    // 119 (w)
    {
        (const uint8_t*)"           ",
        (const uint8_t*)" __      __",
        (const uint8_t*)" \\ \\ /\\ / /",
        (const uint8_t*)"  \\ V  V / ",
        (const uint8_t*)"   \\_/\\_/  ",
        (const uint8_t*)"           ",
    },
    // 120 (x)
    {
        (const uint8_t*)"       ",
        (const uint8_t*)" __  __",
        (const uint8_t*)" \\ \\/ /",
        (const uint8_t*)"  >  < ",
        (const uint8_t*)" /_/\\_\\",
        (const uint8_t*)"       ",
    },
    // 121 (y)
    {
        (const uint8_t*)"        ",
        (const uint8_t*)"  _   _ ",
        (const uint8_t*)" | | | |",
        (const uint8_t*)" | |_| |",
        (const uint8_t*)"  \\__, |",
        (const uint8_t*)"  |___/ ",
    },
    // 122 (z)
    {
        (const uint8_t*)"      ",
        (const uint8_t*)"  ____",
        (const uint8_t*)" |_  /",
        (const uint8_t*)"  / / ",
        (const uint8_t*)" /___|",
        (const uint8_t*)"      ",
    },
    // 123 ({)
    {
        (const uint8_t*)"    __",
        (const uint8_t*)"   / /",
        (const uint8_t*)"  | | ",
        (const uint8_t*)" < <  ",
        (const uint8_t*)"  | | ",
        (const uint8_t*)"   \\_\\",
    },
    // 124 (|)
    {
        (const uint8_t*)"  _ ",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" | |",
        (const uint8_t*)" |_|",
    },
    // 125 (})
    {
        (const uint8_t*)" __   ",
        (const uint8_t*)" \\ \\  ",
        (const uint8_t*)"  | | ",
        (const uint8_t*)"   > >",
        (const uint8_t*)"  | | ",
        (const uint8_t*)" /_/  ",
    },
    // 126 (~)
    {
        (const uint8_t*)"  /\\/|",
        (const uint8_t*)" |/\\/ ",
        (const uint8_t*)"      ",
        (const uint8_t*)"      ",
        (const uint8_t*)"      ",
        (const uint8_t*)"      ",
    },
};

private const int FIGLET_FONT_HEIGHT = 6;

private void tui_widget_figlet_render(Widget *widget, Screen *screen, vec2i position){
    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
    WidgetFigletData *widget_data = widget->data;
    for(size_t i = 0; i < widget_data->lines.count; i++){
        screen_set_string(
            screen,
            position.x,
            position.y + i,
            &widget_data->lines.strings[i]
        );
    }
}

void tui_widget_figlet_(const uint8_t *text){
    Panel *panel = tui_get_panel_building();
    auto max_width = max(0, panel->inner_rect.size.width);

    WidgetFigletData *widget_data = (WidgetFigletData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetFigletData)
    );
    widget_data->text = text;

    // Build figlet output lines
    // We need a temporary buffer for each row
    uint8_t row_buf[6][4096] = {};
    size_t row_widths[6] = {};

    for(size_t i = 0; text[i] != '\0'; i++){
        uint8_t c = text[i];
        if(c < 32 || c > 126){
            c = ' '; // replace non-printable with space
        }
        int char_idx = c - 32;

        for(int row = 0; row < FIGLET_FONT_HEIGHT; row++){
            const uint8_t *art = FIGLET_FONT_CHARS[char_idx][row];
            if(row_widths[row] > 0){
                utf8_str_concat(row_buf[row], (const uint8_t*)" ");
            }
            utf8_str_concat(row_buf[row], art);
        }
    }

    // Measure output width
    size_t output_width = 0;
    for(int row = 0; row < FIGLET_FONT_HEIGHT; row++){
        size_t w = utf8_str_display_width(row_buf[row]);
        if(w > output_width) output_width = w;
    }

    // Truncate rows if they exceed max_width
    if(output_width > (size_t)max_width){
        size_t limit = (size_t)max_width;
        for(int row = 0; row < FIGLET_FONT_HEIGHT; row++){
            size_t dw = 0;
            size_t byte_pos = 0;
            while(row_buf[row][byte_pos] != '\0'){
                uint8_t char_len = utf8_char_length(row_buf[row][byte_pos]);
                uint8_t char_bytes[4] = {};
                for(uint8_t i = 0; i < char_len; i++){
                    char_bytes[i] = row_buf[row][byte_pos + i];
                }
                uint8_t char_dw = utf8_char_display_width(char_bytes);
                if(dw + char_dw > limit){
                    row_buf[row][byte_pos] = '\0';
                    break;
                }
                dw += char_dw;
                byte_pos += char_len;
            }
        }
        output_width = limit;
    }

    // Store output lines
    widget_data->lines.count = 0;
    for(int row = 0; row < FIGLET_FONT_HEIGHT; row++){
        if(strlen((const char*)row_buf[row]) == 0) continue;
        widget_data->lines.strings[row] = string_from(row_buf[row], strlen((const char*)row_buf[row]));
        widget_data->lines.count++;
    }

    Widget new_widget = {
        .id        = tui_create_widget_id(),
        .data      = widget_data,
        .size.w    = (int)min(output_width, (size_t)max_width),
        .size.h    = max(1, (int)widget_data->lines.count + PADDING),
        .focusable = false,
        .render    = &tui_widget_figlet_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_FIGLET_IMPL
#endif //TUI_WIDGET_FIGLET

## tui.h - C23 TUI framework

> [!WARNING]
> Work in Progress

Immediate mode, STB-style TUI framework for writing small C applications. Because I didn't like ncurses.

Currently targeting only Linux + Modern Windows.

This framework is at the moment, highly opinionated. It's not made to do every possible program in it. It simplifies and streamlines the creation of certain types of programs.
I intend to develop it alongside several other C apps I'm building so it will have some flexibility, but probably not enough to do something crazy with it.

## USAGE

### 1. Download

 Download the `tui/` directory and vendor it, or include the whole app as a git submodule.

### 2. Include it

Add `tui.h` to your main.c file, with the proper `#define` beforehand:

```c
#define TUI_IMPL
#include "tui/tui.h"
```

### 3. Create your Pages

Each "screen" of the application is expected to be defined as a Page struct, following a similar mental model as a website's router. Each "route" (a Page's string ID) maps to a Page struct that defines how the screen behaves and renders.

You can define your `Page`s in the same document but I recommend following the single-header file style to make things easier to navigate (See the example app for guidance):

```c
// pages/page_example.h
#ifndef PAGE_EXAMPLE_H
#define PAGE_EXAMPLE_H

#include "tui/tui.h"

//define an ID for the router, make sure it's unique:
#define PAGE_EXAMPLE_ID "PAGE_EXAMPLE_1"

// depending on how you're building, you might wanna define the page struct
// as extern and hide it under the implementation guard below
extern Page PAGE_EXAMPLE;

#ifdef PAGE_EXAMPLE_IMPL
//all your page's code here
// ........
//this is the important part:

Page PAGE_EXAMPLE = {
	.title   = u8"Page Example",
	.layout  = LAYOUT_SINGLE_PANEL,
	.init    = &page_example_init,     //optional
	.input   = &page_example_input,    //optional
	.process = &page_example_process,  //optional
	.render  = &page_example_render,
};

#endif //PAGE_EXAMPLE_IMPL
#endif //PAGE_EXAMPLE_H

```

A Page declares:
- Its title as a utf-8 string
- The layout it wants to use (See layouts below for more info)
- Several function pointers, the most important of which is the render function

### 4. The Page's Render function

A Page's render function is where you're supposed to declare which widgets go in which parts of the interface and how they're configured.

```c
bool show_popup = false;
void show_example_popup(void){  show_popup = true;  }
void close_example_popup(void){ show_popup = false; }

//used in the Page struct
void page_example_render(void){
	tui_panel_begin(SLOT_MAIN);
		tui_widget_label(u8" 🌎 Hello World! 🌎 ");
		tui_widget_button("BUTTON_1", //WIDGET_ID, do not repeat!
			.label     = u8"Show Popup!",
			.on_click  = &show_example_popup,
			.is_inline = false,
		);
	tui_panel_end();

	if(!show_popup) return;

	tui_layer_begin(LAYER_OVERLAY, LAYOUT_SINGLE_PANEL);
		tui_panel_begin(SLOT_MAIN);
			tui_widget_label("Hello from popup!");
			tui_widget_button("BUTTON_POPUP",
				.label    = u8"OK!",
				.on_click = &close_example_popup,
			);
		tui_panel_end();
	tui_layer_end();
}

```

This function is called every frame, and for the most part you're in charge of taking care of the state.

See the examples for more ways of doing this.

For a list of all available widgets, grep for `tui_widget_` inside the tui directory.

### 5. Setting up the main app loop

I call tui.h a _framework_ rather than a library because it expects your app to be built in a specific way to work properly.

Your main.c file should probably look something like this:

```c

//include tui.h first
#define TUI_IMPL
#include "tui/tui.h"

//then include your pages
#define PAGE_EXAMPLE_IMPL
#include "pages/page_example.h"

int main(void){
	//register all your pages first
	tui_register_page(PAGE_EXAMPLE_ID, &PAGE_EXAMPLE);

	//navigate to the starting/landing page
	tui_navigate_to(PAGE_EXAMPLE_ID);

	//start the main app loop!
	tui_run_loop();

	return 0;
}
```

## NAVIGATION

The framework exposes a few core functions to handle navigating between the pages:

```c
void tui_navigate_to(const char *page_id);
void tui_navigate_back(void);
void tui_quit(void);
```

The framework holds a stack of the navigation history so you can simply call `tui_navigate_back();` on a page's "exit" if you want to simplify things.

Navigating back on the root page *does not* exit the application. You have to explicitly call `tui_quit()` to close it.

## LAYOUTS

Several different layouts are available, with more to come in the future:

```c
typedef enum {
    LAYOUT_SINGLE_PANEL,
    LAYOUT_SIDEBAR_LEFT,
    LAYOUT_SIDEBAR_RIGHT,
    LAYOUT_SPLIT_VERTICAL,
    LAYOUT_WITH_HEADER,
    LAYOUT_WITH_FOOTER,
    LAYOUT_WITH_HEADER_AND_FOOTER,
    LAYOUT_SPLIT_VERTICAL_WITH_HEADER,
} PageLayout;
```

Each layout allows a different set of panel slots, but for the most part they're pretty intuitive to figure out:

```c
typedef enum {
    SLOT_MAIN,
    SLOT_SIDEBAR,
    SLOT_TOP,
    SLOT_BOTTOM,
    SLOT_LEFT,
    SLOT_RIGHT,
} PanelSlot;
```

## HANDLING INPUT

> [!WARNING]
> Mouse support is not yet implemented, but it is planned

Each Page can declare a function pointer to their input handling function. It receives the next input event in the queue:

```c
bool page_example_input(InputEvent input_event){
    switch (input_event.input_type){
    case INPUT_KEY:
        switch (input_event.key_event.key){
        case KEY_Q:
            tui_quit(); //for example!
            break;
		case KEY_NONE:
        default:
        break;
        }
    case INPUT_NONE:
    default:
    }

    return false; //<- does not capture input
}
```

The input function returns a boolean that decides whether you have captured this input event or if you want to let it bubble up for the panel/other widgets to catch.

You can find the the full list of InputEvents in `tui_platform.h`

## HOTKEYS / KEYBINDS

Needing to register hotkeys is pretty common so we have a much easier way of doing it rather than directly handling the input:

```c
void page_example_process(float delta_time){
	//register hotkeys
	tui_register_key(KEY_T, KEY_MOD_NONE, &show_example_popup);
	tui_register_key_hint(u8"[T]", u8"Show Popup");
}
```

Just make sure you register them before the render function, usually the process function is fine.

## LOCALIZATION / TRANSLATION / i18n

All the internal strings used are defined in `tui_i18n.h` as macros.
You can easily override them before including `tui.h` to localize your application:

```c
//spanish
#define I18N_HINT_SELECT_TEXT u8"Seleccionar"
#define I18N_HELP_TITLE u8"Atajos de Teclado"
#define I18N_HELP_OPEN u8"[?] Ayuda"
#define I18N_HELP_CLOSE u8"[?] Ocultar Ayuda"
//.... etc

#define TUI_IMPL
#include "tui/tui.h"
```

At the moment, the framework does not support runtime language selection.

You could however save the selected language to a config file and conditionally define the macros when the program starts, effectively requiring the user to restart the app to change the language.

## BUILDING

For the most part, if you use a unity build it'll be very straightforward. tui.h includes all other required files so you don't have to do much else.

Only caveat is you might need to link against math.h properly.

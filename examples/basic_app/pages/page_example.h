#ifndef PAGE_EXAMPLE_H
#define PAGE_EXAMPLE_H

#include "tui/tui.h"
#include "page_virtual_list.h"
#include "page_table.h"
#include <string.h>

#define PAGE_EXAMPLE_ID "PAGE_EXAMPLE_1"
extern Page PAGE_EXAMPLE;

#ifdef PAGE_EXAMPLE_IMPL

//variables de la pagina ---------

bool show_popup = false;
bool accept_checkbox1 = false;
bool accept_checkbox2 = false;

//radio buttons
size_t selected_radio = 0;
static const size_t RADIO_OPTION_A = 0;
static const size_t RADIO_OPTION_B = 1;
static const size_t RADIO_OPTION_C = 2;
int selected_number = 7;

//select options
size_t selected_option = 0;
static WidgetSelectOption SELECT_OPTIONS[] = {
	{.value = 0, .label = u8"Uno"},
	{.value = 1, .label = u8"Dos"},
	{.value = 2, .label = u8"Tres"},
};

//select with filter
size_t selected_option2 = 0;
static WidgetSelectOption SELECT_OPTIONS2[] = {
	{.value = 0, .label = u8"La mejor Opcion"},
	{.value = 1, .label = u8"La peor opcion"},
	{.value = 2, .label = u8"opcion 3"},
	{.value = 3, .label = u8"otra"},
	{.value = 4, .label = u8"opcion premium"},
	{.value = 5, .label = u8"opcion pirata"},
};

constexpr size_t name_length_max = 255;
uint8_t *name;

//tabs
size_t TAB_SELECTED = 0;
static WidgetTabOption TAB_OPTIONS[] = {
    {.label = u8"Info",     .value = 0},
    {.label = u8"Settings", .value = 1},
    {.label = u8"About",    .value = 2},
};

static WidgetSelectOptions filter_options(const uint8_t *query){
	const size_t max_options = 16;
	WidgetSelectOptions opts = {
		.values = (WidgetSelectOption *)arena_alloc(
			LAYOUT_STATE.arena_frame,
			sizeof(WidgetSelectOption) * max_options)
	};
	bool is_query_empty = strlen((const char*)query) == 0;
	for(size_t i = 0; i < arr_size(SELECT_OPTIONS2); i++){
		if(opts.count >= max_options) break; //check the limit!
		if(is_query_empty){
			opts.values[opts.count++] = SELECT_OPTIONS2[i];
		}else{
			// here you can do for example a filter function ...
			bool equals = strcmp((const char*)SELECT_OPTIONS2[i].label, (const char*)query) == 0;
			if(equals){
				opts.values[opts.count++] = SELECT_OPTIONS2[i];
			}
		}
	}
	return opts;
}

static void show_example_popup(void){
	show_popup = true;
	// name = utf8_str_concat(name, u8"Another Name!");
}

static void close_example_popup(void){
	show_popup = false;
	// memset(name, '\0', name_length_max * sizeof(uint8_t));
}

static void navigate_to_secondary(void){
	tui_navigate_to(PAGE_SECONDARY_ID);
}

static void navigate_to_virtual_list(void){
	tui_navigate_to(PAGE_VIRTUAL_LIST_ID);
}

static void navigate_to_table(void){
	tui_navigate_to(PAGE_TABLE_ID);
}

static void page_example_init(void){
	name = (uint8_t *)calloc(name_length_max, sizeof(uint8_t));
	// name = utf8_str_concat(name, u8"Mr. Smith");
}

static bool page_example_input(InputEvent input_event){
	//we don't need this for now, but we can have access
	//to direct user input if needed

    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_Q:
            // tui_quit();
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

static void page_example_process(float delta_time){
	assert(delta_time > -1);
	//register hotkeys
	tui_register_key(KEY_T, KEY_MOD_NONE, &show_example_popup);
	tui_register_key_hint(u8"[T]", u8"Show Popup");
	// if(show_popup){
	// 	tui_register_key(KEY_W, KEY_MOD_NONE, &close_example_popup);
	// }else{
	// 	tui_register_key(KEY_W, KEY_MOD_NONE, &show_example_popup);
	// }
	// printf("DELTA TIME: %.2f", delta_time);
}

static void page_example_render(void){
	tui_panel_begin(SLOT_MAIN);

		tui_widget_spinner("Cargando...");

		if(show_popup){
			tui_widget_label("show_popup!");
		}else{
			tui_widget_label("show_popup: false");
		}
		tui_widget_label(u8" 🌎 Hello World! 🌎 ");

		tui_widget_switch("SWITCH_1",
			.label=u8"Mostrar Popup ",
			.storage=&show_popup,
			.on_toggle=nullptr
		);

		tui_widget_input_checkbox("CHECKBOX_1",
			.label=u8"Acepto checkbox",
			.storage=&accept_checkbox1,
			.on_toggle=nullptr
		);
		tui_widget_input_checkbox("CHECKBOX_2",
			.label=u8"Otro checkbox",
			.storage=&accept_checkbox2,
			.on_toggle=nullptr
		);

		tui_widget_input_radio("RADIO_1",
			.label=u8"Primera Opcion",
			.storage=&selected_radio,
			.storage_size=sizeof(selected_radio),
			.value=&RADIO_OPTION_A
		);
		tui_widget_input_radio("RADIO_2",
			.label=u8"Segunda Opcion...",
			.storage=&selected_radio,
			.storage_size=sizeof(selected_radio),
			.value=&RADIO_OPTION_B
		);
		tui_widget_input_radio("RADIO_3",
			.label=u8"Ultima!",
			.storage=&selected_radio,
			.storage_size=sizeof(selected_radio),
			.value=&RADIO_OPTION_C
		);

		tui_widget_input_number("NUMBER_1",
			.label=u8"Number: ",
			.storage=&selected_number,
			.min_value=-50,
			.max_value=200,
			.step=5
		);

		tui_widget_input_text("INPUT_TEXT_1", //WIDGET_ID, do not repeat!
			.label=u8"Nombre: ",
			.placeholder=u8"Placeholder text...",
			.storage=name,
			.capacity=name_length_max
		);

		tui_widget_select("SELECT_DUMMY_1",
			.label=u8"Dummy Select: ",
			.storage=&selected_option,
			.options=(WidgetSelectOptions){
				.values = SELECT_OPTIONS,
				.count  = arr_size(SELECT_OPTIONS)
			}
		);

		tui_widget_select_filter("SELECT_AUTO_1",
			.label=u8"Autocomplete: ",
			.storage=&selected_option2,
			.options_function=&filter_options,
			.placeholder=u8"Type to filter...",
			.empty_options_label=u8"No options found",
			// .options_capacity=arr_size(SELECT_OPTIONS2)
		);

		tui_widget_button("BUTTON_1", //WIDGET_ID, do not repeat!
			.label=u8"Mostrar Popup",
			.on_click=&show_example_popup,
			.is_inline=true,
		);
		tui_widget_button("BUTTON_GO_SECONDARY",
			.label=u8"Go to Page 2",
			.on_click=&navigate_to_secondary,
			.is_inline=true,
		);
		tui_widget_button("BUTTON_GO_VIRTUAL_LIST",
			.label=u8"Virtual List Demo",
			.on_click=&navigate_to_virtual_list,
			.is_inline=true,
		);
		tui_widget_button("BUTTON_GO_TABLE",
			.label=u8"Table Demo",
			.on_click=&navigate_to_table,
			.is_inline=true,
		);
		tui_widget_button("BUTTON_2",
			.label=u8"Salir",
			.on_click=&tui_quit,
			.is_inline=true,
		);

		tui_widget_tabs("TABS_1",
			.storage = &TAB_SELECTED,
			.tabs = {
				.values = TAB_OPTIONS,
				.count = arr_size(TAB_OPTIONS),
			},
		);

		tui_widget_label("Press [Q] to quit");

		tui_widget_label(
			"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod"
			"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,"
		);
		tui_widget_label(
			"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo"
			"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse"
		);
		tui_widget_label(
			"cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non"
			"proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
		);
		tui_widget_label(
			"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod"
			"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,"
		);
		tui_widget_label(
			"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo"
			"consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse"
		);
		tui_widget_label(
			"cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non"
			"proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
		);
	tui_panel_end();

	if(!show_popup) return;

	tui_layer_begin(LAYER_OVERLAY, LAYOUT_SINGLE_PANEL);
		tui_panel_begin(SLOT_MAIN);
			tui_widget_label("Hello from popup!");
			tui_widget_button("BUTTON_POPUP",
				.label=u8"OK!",
				.on_click=&close_example_popup,
			);
		tui_panel_end();
	tui_layer_end();
}

Page PAGE_EXAMPLE = {
	.title   = u8"Page Example",
	.layout  = LAYOUT_SINGLE_PANEL,
	.init    = &page_example_init,
	.input   = &page_example_input,
	.process = &page_example_process,
	.render  = &page_example_render,
};


#endif //PAGE_EXAMPLE_IMPL
#endif //PAGE_EXAMPLE_H

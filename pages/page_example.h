#ifndef PAGE_EXAMPLE_H
#define PAGE_EXAMPLE_H

#include "../tui/tui.h"
#include <string.h>

#define PAGE_EXAMPLE_ID "page_example"
extern Page PAGE_EXAMPLE;

#ifdef PAGE_EXAMPLE_IMPL

bool show_popup = false;
constexpr size_t name_length_max = 255;
uint8_t *name;

private void show_example_popup(void){
	show_popup = true;
	// name = utf8_str_concat(name, u8"Another Name!");
}

private void close_example_popup(void){
	show_popup = false;
	// memset(name, '\0', name_length_max * sizeof(uint8_t));
}

private void page_example_init(void){
	name = (uint8_t *)calloc(name_length_max, sizeof(uint8_t));
	// name = utf8_str_concat(name, u8"Mr. Smith");
}

private bool page_example_input(InputEvent input_event){
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

private void page_example_process(float delta_time){
	assert(delta_time > -1);
	//register hotkeys
	tui_register_key(KEY_Q, KEY_MOD_NONE, &tui_quit);
	tui_register_key_hint(u8"[Q]", u8"Salir");
	// if(show_popup){
	// 	tui_register_key(KEY_W, KEY_MOD_NONE, &close_example_popup);
	// }else{
	// 	tui_register_key(KEY_W, KEY_MOD_NONE, &show_example_popup);
	// }
	// printf("DELTA TIME: %.2f", delta_time);
}

private void page_example_render(void){
	tui_panel_begin(SLOT_MAIN);
		if(show_popup){
			tui_widget_label("show_popup!");
		}else{
			tui_widget_label("show_popup: false");
		}
		tui_widget_label(u8"Hello World! 🌎");
		tui_widget_label("Press [Q] to quit");

		tui_widget_input_text("INPUT_TEXT_1", //WIDGET_ID, do not repeat!
			.label=u8"Nombre: ",
			.placeholder=u8"Emmanuel Etcheber",
			.storage=name,
			.capacity=name_length_max
		);
		// tui_widget_input_textarea();
		// tui_widget_input_numbers();
		// tui_widget_input_select();
		// tui_widget_input_select_suggestions();
		// tui_widget_input_select_radio();
		// tui_widget_input_select_checkbox();

		tui_widget_button("BUTTON_1", //WIDGET_ID, do not repeat!
			.label=u8"Mostrar Popup",
			.on_click=&show_example_popup
		);
		tui_widget_button("BUTTON_2", .label=u8"Salir", .on_click=&tui_quit);
	tui_panel_end();

	if(!show_popup) return;

	// tui_popup_begin();
	// 	tui_widget_label("Hello from popup!");
	// 	tui_widget_button("OK!", close_example_popup);
	// tui_popup_end();
}

Page PAGE_EXAMPLE = {
	.layout  = LAYOUT_SINGLE_PANEL,
	.init    = &page_example_init,
	.input   = &page_example_input,
	.process = &page_example_process,
	.render  = &page_example_render,
};


#endif //PAGE_EXAMPLE_IMPL
#endif //PAGE_EXAMPLE_H

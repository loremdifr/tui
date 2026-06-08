#ifndef PAGE_SECONDARY_H
#define PAGE_SECONDARY_H

#include "tui/tui.h"

#define PAGE_SECONDARY_ID "PAGE_2"
extern Page PAGE_SECONDARY;

#ifdef PAGE_SECONDARY_IMPL

//variables de la pagina
private bool secondary_popup = false;

private void close_secondary_popup(void){
	secondary_popup = false;
}

private void page_secondary_init(void){
}

private bool page_secondary_input(InputEvent input_event){
    switch (input_event.input_type) {
    case INPUT_KEY:
        switch (input_event.key_event.key) {
        case KEY_P:
            // toggle popup
            secondary_popup = !secondary_popup;
            return true;
        case KEY_NONE:
        default:
        break;
        }
    case INPUT_NONE:
    default:
    }
    return false;
}

private void page_secondary_process(float delta_time){
    (void)delta_time;
}

private void page_secondary_render(void){
	tui_panel_begin(SLOT_SIDEBAR);
		tui_widget_label(u8"SIDEBAR");
		tui_widget_label(u8"---------");
		tui_widget_label(u8"Nav Menu");
		tui_widget_label(u8"Info Panel");
	tui_panel_end();

	tui_panel_begin(SLOT_MAIN);

		tui_widget_label(u8"Welcome to the Secondary Page!");
		tui_widget_label(u8"Press [P] to toggle popup");

		tui_widget_button("BACK_BUTTON",
			.label=u8"Back",
			.on_click=&tui_navigate_back,
			.is_inline=true,
		);

	tui_panel_end();

	if(!secondary_popup) return;

	tui_panel_begin(SLOT_OVERLAY);
		tui_widget_label(u8"Secondary Popup!");
		tui_widget_button("SECONDARY_POPUP_BUTTON",
			.label=u8"OK!",
			.on_click=&close_secondary_popup,
		);
	tui_panel_end();
}

Page PAGE_SECONDARY = {
	.title   = u8"Secondary Page",
	.layout  = LAYOUT_SIDEBAR_LEFT,
	.init    = &page_secondary_init,
	.input   = &page_secondary_input,
	.process = &page_secondary_process,
	.render  = &page_secondary_render,
};

#endif //PAGE_SECONDARY_IMPL
#endif //PAGE_SECONDARY_H

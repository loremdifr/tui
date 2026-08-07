#ifndef _WIN32
#define _GNU_SOURCE //required here for the clock_gettime in tui_utils.h
#endif

#include <stdio.h>
#include <stdbool.h>

#define I18N_HINT_SELECT_TEXT u8"Seleccionar"
#define I18N_HELP_TITLE u8"Atajos de Teclado"
#define I18N_HELP_OPEN u8"[?] Ayuda"
#define I18N_HELP_CLOSE u8"[?] Ocultar Ayuda"

#define TUI_IMPL
#include "tui/tui.h"

#define PAGE_VIRTUAL_LIST_IMPL
#include "pages/page_virtual_list.h"

#define PAGE_SECONDARY_IMPL
#include "pages/page_secondary.h"

#define PAGE_EXAMPLE_IMPL
#include "pages/page_example.h"

#define PAGE_TABLE_IMPL
#include "pages/page_table.h"

#include "tui/tui_theme.h"

int main(void)
{
	//setup de la app
	tui_register_page(PAGE_EXAMPLE_ID, &PAGE_EXAMPLE);
	tui_register_page(PAGE_SECONDARY_ID, &PAGE_SECONDARY);
	tui_register_page(PAGE_VIRTUAL_LIST_ID, &PAGE_VIRTUAL_LIST);
	// tui_register_page(PAGE_TABLE_ID, &PAGE_TABLE);

	//comienzo de la app
	tui_set_theme(example_theme);
	tui_navigate_to(PAGE_EXAMPLE_ID); //definimos la primera pagina o home
	tui_run_loop();

	//mensaje de salida
	printf("\n - PROGRAMA FINALIZADO - \n");
	return 0;
}

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

#define PAGE_EXAMPLE_IMPL
#include "pages/page_example.h"

#define PAGE_SECONDARY_IMPL
#include "pages/page_secondary.h"

int main(void)
{
	//setup de la app
	tui_register_page(PAGE_EXAMPLE_ID, &PAGE_EXAMPLE);
	tui_register_page(PAGE_SECONDARY_ID, &PAGE_SECONDARY);

	//comienzo de la app
	tui_navigate_to(PAGE_EXAMPLE_ID); //definimos la primera pagina o home
	tui_run_loop();

	//mensaje de salida
	tui_clear();
	printf("\n - PROGRAMA FINALIZADO - \n");
	return 0;
}

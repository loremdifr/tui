#ifndef _GNU_SOURCE
	#define _GNU_SOURCE //required here for the clock_gettime in tui_utils.h
#endif

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#ifndef TUI_IMPL
#define TUI_IMPL
#endif //TUI_IMPL
#include "tui/tui.h"

#ifndef PAGE_EXAMPLE_IMPL
#define PAGE_EXAMPLE_IMPL
#endif //PAGE_EXAMPLE_IMPL
#include "pages/page_example.h"

int main(void)
{
	//setup de la app
	tui_register_page(PAGE_EXAMPLE_ID, &PAGE_EXAMPLE);
	// tui_register_page(MI_PAGINITA_ID, &MI_PAGINITA_ID);

	//comienzo de la app
	tui_navigate_to(PAGE_EXAMPLE_ID); //definimos la primera pagina o home
	tui_run_loop();

	//mensaje de salida
	tui_clear();
	printf("\n - PROGRAMA FINALIZADO - \n");
	return 0;
}


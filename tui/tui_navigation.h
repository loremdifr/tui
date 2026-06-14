#ifndef TUI_NAVIGATION
#define TUI_NAVIGATION

#include <stdint.h>
#include "tui_platform.h"
#include "tui_layout.h"

typedef void (*InitFunction   )(void);
typedef bool (*InputFunction  )(InputEvent input_event);
typedef void (*ProcessFunction)(float delta_time);
typedef void (*RenderFunction )(void);

typedef struct {
	const uint8_t  *title;
    PageLayout      layout;
    InitFunction    init;
    InputFunction   input;
    ProcessFunction process;
    RenderFunction  render;
} Page;

// API -------------------------------------------------------
void  tui_register_page(const char *page_id, Page *page);
void  tui_navigate_to(const char *page_id);
void  tui_navigate_back(void);
Page *tui_get_curr_page();


// IMPL ------------------------------------------------------------------------
#ifdef TUI_NAVIGATION_IMPL

static constexpr int TUI_NAV_HISTORY_MAX = 64;

typedef struct {
	int stack[TUI_NAV_HISTORY_MAX];
	int count;
} _NavigationHistory;

static _NavigationHistory NAV_HISTORY;

#define TUI_MAX_PAGES 64

typedef struct {
	char *page_id;
	Page *page;
} _PageRoute;

typedef struct {
	_PageRoute routes[TUI_MAX_PAGES];
	int routes_count;
} _PageRoutes;

static _PageRoutes PAGE_ROUTES = {};

void tui_register_page(const char *page_id, Page *page) {
	//TODO: might wanna assert here the app has not started yet
	assert(PAGE_ROUTES.routes_count <= TUI_MAX_PAGES);
	_PageRoute *page_route = &PAGE_ROUTES.routes[PAGE_ROUTES.routes_count];
	page_route->page_id = calloc(strlen(page_id) + 1, sizeof(char));
	strcpy(page_route->page_id, page_id);
	page_route->page = page;
	PAGE_ROUTES.routes_count++;
}

void tui_navigate_to(const char *page_id){
	int page_index  = 0;
	bool page_found = false;
	for(; page_index < PAGE_ROUTES.routes_count; page_index++){
		page_found = strcmp(PAGE_ROUTES.routes[page_index].page_id, page_id) == 0;
		if(page_found) break;
	}
	assert(page_found); // did you try to navigate to a page that doesn't exist?
	assert(NAV_HISTORY.count < TUI_NAV_HISTORY_MAX - 1); //stack overflow
	NAV_HISTORY.stack[NAV_HISTORY.count++] = page_index;
	_tui_layout_reset();
}

void tui_navigate_back(void){
	if (NAV_HISTORY.count <= 1) return; //check 1 because root is part of history!
	NAV_HISTORY.count--;
	_tui_layout_reset();
}

Page *tui_get_curr_page(){
	assert(NAV_HISTORY.count > 0); //There's no pages in the history, don't forget to init!
	int curr_page_index = NAV_HISTORY.stack[NAV_HISTORY.count - 1];
	return PAGE_ROUTES.routes[curr_page_index].page;
}

#endif //TUI_NAVIGATION_IMPL
#endif //TUI_NAVIGATION

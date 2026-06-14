#ifndef PAGE_VIRTUAL_LIST_H
#define PAGE_VIRTUAL_LIST_H

#include "tui/tui.h"

#define PAGE_VIRTUAL_LIST_ID "PAGE_VIRTUAL_LIST"
extern Page PAGE_VIRTUAL_LIST;

#ifdef PAGE_VIRTUAL_LIST_IMPL

size_t VIRTUAL_LIST_SELECTED = 0;

static WidgetVirtualListItem VIRTUAL_LIST_ITEMS[] = {
    {.label = u8"Boca Juniors"},
    {.label = u8"River Plate"},
    {.label = u8"San Lorenzo"},
    {.label = u8"Huracán"},
    {.label = u8"Vélez Sarsfield"},
    {.label = u8"Argentinos Juniors"},
    {.label = u8"Racing Club"},
    {.label = u8"Independiente"},
    {.label = u8"Talleres de Córdoba"},
    {.label = u8"Barracas Central"},
    {.label = u8"Platense"},
    {.label = u8"Tigre"},
    {.label = u8"Deportivo Riestra"},
    {.label = u8"Defensa y Justicia"},
    {.label = u8"Estudiantes (LP)"},
    {.label = u8"Newell's"},
    {.label = u8"Rosario Central"},
    {.label = u8"Belgrano"},
    {.label = u8"Atlético Tucumán"},
    {.label = u8"Lanús"},
    {.label = u8"Banfield"},
    {.label = u8"Gimnasia (LP)"},
    {.label = u8"Instituto"},
    {.label = u8"Central Córdoba (SdE)"},
    {.label = u8"Sarmiento"},
    {.label = u8"Aldosivi"},
    {.label = u8"Gimnasia Mendoza"},
    {.label = u8"Estudiantes Río Cuarto"},
    {.label = u8"Independiente Rivadavia"},
    {.label = u8"Unión"},
    {.label = u8"Godoy Cruz"},
    {.label = u8"Colón"},
    {.label = u8"Quilmes"},
    {.label = u8"Arsenal"},
    {.label = u8"Chacarita"},
    {.label = u8"Ferro"},
};

static void page_virtual_list_init(void){
    VIRTUAL_LIST_SELECTED = 0;
}

static bool page_virtual_list_input(InputEvent input_event){
    (void)input_event;
    return false;
}

static void page_virtual_list_process(float delta_time){
    (void)delta_time;
}

static void page_virtual_list_render(void){
    tui_panel_begin(SLOT_MAIN);
        tui_widget_virtual_list(
            "VIRTUAL_LIST",
            .storage   = &VIRTUAL_LIST_SELECTED,
            .items     = {
                .items = VIRTUAL_LIST_ITEMS,
                .count = arr_size(VIRTUAL_LIST_ITEMS)
            },
            .on_select = nullptr,
            .on_click  = &tui_navigate_back,
        );
    tui_panel_end();
}

Page PAGE_VIRTUAL_LIST = {
    .title   = u8"Virtual List",
    .layout  = LAYOUT_SINGLE_PANEL,
    .init    = &page_virtual_list_init,
    .input   = &page_virtual_list_input,
    .process = &page_virtual_list_process,
    .render  = &page_virtual_list_render,
};

#endif //PAGE_VIRTUAL_LIST_IMPL
#endif //PAGE_VIRTUAL_LIST_H

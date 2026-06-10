#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "tui/tui.h"

#define PAGE_TABLE_ID "PAGE_TABLE"
extern Page PAGE_TABLE;

#ifdef PAGE_TABLE_IMPL

size_t TABLE_SELECTED = 0;

static const uint8_t *TABLE_HEADERS[] = { u8"Name", u8"Age", u8"City" };

static WidgetTableCell TABLE_CELLS[] = {
    {.label = u8"Alice"},   {.label = u8"30"}, {.label = u8"NYC"},
    {.label = u8"Bob"},     {.label = u8"25"}, {.label = u8"London"},
    {.label = u8"Charlie"}, {.label = u8"35"}, {.label = u8"Paris"},
    {.label = u8"Diana"},   {.label = u8"28"}, {.label = u8"Berlin"},
    {.label = u8"Eve"},     {.label = u8"32"}, {.label = u8"Tokyo"},
    {.label = u8"Frank"},   {.label = u8"40"}, {.label = u8"Madrid"},
    {.label = u8"Grace"},   {.label = u8"27"}, {.label = u8"Rome"},
    {.label = u8"Henry"},   {.label = u8"33"}, {.label = u8"Lisbon"},
    {.label = u8"Ivy"},     {.label = u8"29"}, {.label = u8"Vienna"},
    {.label = u8"Jack"},    {.label = u8"31"}, {.label = u8"Prague"},
    {.label = u8"Kate"},    {.label = u8"26"}, {.label = u8"Warsaw"},
    {.label = u8"Leo"},     {.label = u8"38"}, {.label = u8"Budapest"},
    {.label = u8"Mia"},     {.label = u8"34"}, {.label = u8"Dublin"},
    {.label = u8"Noah"},    {.label = u8"36"}, {.label = u8"Athens"},
    {.label = u8"Olivia"},  {.label = u8"22"}, {.label = u8"Oslo"},
    {.label = u8"Paul"},    {.label = u8"45"}, {.label = u8"Stockholm"},
    {.label = u8"Quinn"},   {.label = u8"37"}, {.label = u8"Helsinki"},
    {.label = u8"Ryan"},    {.label = u8"41"}, {.label = u8"Copenhagen"},
    {.label = u8"Sara"},    {.label = u8"39"}, {.label = u8"Brussels"},
};

private void page_table_init(void){
    TABLE_SELECTED = 0;
}

private bool page_table_input(InputEvent input_event){
    (void)input_event;
    return false;
}

private void page_table_process(float delta_time){
    (void)delta_time;
}

private void page_table_render(void){
    tui_panel_begin(SLOT_MAIN);
        tui_widget_table("TABLE_1",
            .storage = &TABLE_SELECTED,
            .table = {
                .headers = TABLE_HEADERS,
                .cells = TABLE_CELLS,
                .row_count = 20,
                .column_count = 3,
            },
            .on_select = nullptr,
            .on_click = &tui_navigate_back,
        );
    tui_panel_end();
}

Page PAGE_TABLE = {
    .title   = u8"Table Demo",
    .layout  = LAYOUT_SINGLE_PANEL,
    .init    = &page_table_init,
    .input   = &page_table_input,
    .process = &page_table_process,
    .render  = &page_table_render,
};

#endif //PAGE_TABLE_IMPL
#endif //PAGE_TABLE_H

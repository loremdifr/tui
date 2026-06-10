#ifndef TUI_WIDGET_TABLE
#define TUI_WIDGET_TABLE

#include "tui_layout.h"

typedef struct {
    const uint8_t *label;
} WidgetTableCell;

typedef struct {
    const uint8_t **headers;      // [column_count] header strings
    WidgetTableCell *cells;       // [row_count * column_count] flat row-major
    size_t           row_count;
    size_t           column_count;
} WidgetTableData;

typedef struct {
    bool               is_inline;
    size_t            *storage;
    WidgetTableData    table;
    FunctionPointer    on_select;
    FunctionPointer    on_click;
} WidgetTableParams;

#define tui_widget_table(widget_id, ...) \
        tui_widget_table_((widget_id), &(WidgetTableParams){__VA_ARGS__})

void tui_widget_table_(const char *widget_id, WidgetTableParams *params);

#ifdef TUI_WIDGET_TABLE_IMPL

typedef struct {
    size_t            *storage;
    WidgetTableData    table;
    FunctionPointer    on_select;
    FunctionPointer    on_click;
    int               *column_widths; // [column_count] computed display widths
    int                total_width;
    Panel             *panel;
} WidgetTableDataInternal;

typedef struct {
    size_t selected_index;
    int    last_render_y;
    bool   has_rendered;
} WidgetTableState;

private int tui_widget_table_compute_column_widths(WidgetTableDataInternal *data){
    int total = 0;
    for(size_t col = 0; col < data->table.column_count; col++){
        int max_w = (int)utf8_str_display_width(data->table.headers[col]);
        for(size_t row = 0; row < data->table.row_count; row++){
            auto cell = &data->table.cells[row * data->table.column_count + col];
            int w = (int)utf8_str_display_width(cell->label);
            if(w > max_w) max_w = w;
        }
        data->column_widths[col] = max_w + 2; // padding
        total += data->column_widths[col];
    }
    return total + (int)(data->table.column_count - 1); // separators
}

private void tui_widget_table_auto_scroll(WidgetTableState *state){
    if(!state->has_rendered) return;

    Panel *panel   = tui_get_panel_focused();
    rect2i visible = panel->inner_rect;
    // first visible data row (below header + separator)
    int first_visible = max(0, visible.pos.y - (state->last_render_y + 1) + 1);
    int last_visible  = first_visible + max(0, visible.size.h - 2);

    if((int)state->selected_index < first_visible){
        tui_panel_scroll((int)state->selected_index - first_visible);
    }else if((int)state->selected_index >= last_visible){
        tui_panel_scroll((int)state->selected_index - last_visible + 1);
    }
}

private void tui_widget_table_render(Widget *widget, Screen *screen, vec2i position){
    WidgetTableDataInternal *data  = widget->data;
    WidgetTableState        *state = widget->state;

    state->last_render_y = position.y;
    state->has_rendered  = true;

    rect2i visible = data->panel->inner_rect;

    // --- sticky header ---
    int header_y = visible.pos.y;
    screen_format(BOLD, COLOR_GRAY, COLOR_BLACK);
    int hx = position.x + PADDING;
    for(size_t col = 0; col < data->table.column_count; col++){
        screen_set_utf8_str(screen, hx, header_y, data->table.headers[col]);
        hx += data->column_widths[col];
        if(col < data->table.column_count - 1){
            screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
            screen_set_utf8(screen, hx, header_y, u8"│");
            hx += 1;
            screen_format(BOLD, COLOR_GRAY, COLOR_BLACK);
        }
    }

    // --- sticky separator ---
    int sep_y = header_y + 1;
    screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
    int sx = position.x + PADDING;
    for(size_t col = 0; col < data->table.column_count; col++){
        for(int w = 0; w < data->column_widths[col]; w++){
            screen_set_utf8(screen, sx + w, sep_y, u8"─");
        }
        sx += data->column_widths[col];
        if(col < data->table.column_count - 1){
            screen_set_utf8(screen, sx, sep_y, u8"┼");
            sx += 1;
        }
    }

    // --- data rows ---
    int data_top = position.y + 1;
    int min_y    = sep_y + 1;
    int max_y    = visible.pos.y + visible.size.h;

    int first = max(0, min_y - data_top);
    int last  = min((int)data->table.row_count, max_y - data_top);

    for(int i = first; i < last; i++){
        bool selected = (i == (int)state->selected_index);
        int row_y = data_top + i;

        if(selected && widget->focused){
            screen_format(NORMAL, COLOR_BLACK, COLOR_MAGENTA);
        }else if(selected){
            screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
        }else{
            screen_format(NORMAL, COLOR_DARK_WHITE, COLOR_BLACK);
        }

        int dx = position.x + PADDING;
        for(size_t col = 0; col < data->table.column_count; col++){
            auto cell = &data->table.cells[i * data->table.column_count + col];
            screen_set_utf8_str(screen, dx, row_y, cell->label);
            dx += data->column_widths[col];
            if(col < data->table.column_count - 1){
                screen_format(NORMAL, COLOR_GRAY, COLOR_BLACK);
                screen_set_utf8(screen, dx, row_y, u8"│");
                dx += 1;
                if(selected && widget->focused){
                    screen_format(NORMAL, COLOR_BLACK, COLOR_MAGENTA);
                }else if(selected){
                    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
                }else{
                    screen_format(NORMAL, COLOR_DARK_WHITE, COLOR_BLACK);
                }
            }
        }
    }

    screen_format(NORMAL, COLOR_WHITE, COLOR_BLACK);
}

private bool tui_widget_table_input(Widget *widget, InputEvent input_event){
    WidgetTableDataInternal *data  = widget->data;
    WidgetTableState        *state = widget->state;

    switch(input_event.input_type){
    case INPUT_KEY:
        switch(input_event.key_event.key){
        case KEY_UP:
            if(state->selected_index == 0){
                state->selected_index = data->table.row_count - 1;
            }else{
                state->selected_index--;
            }
            memcpy(data->storage, &state->selected_index, sizeof(size_t));
            if(data->on_select != nullptr) data->on_select();
            tui_widget_table_auto_scroll(state);
            return true;
        case KEY_DOWN:
            state->selected_index = (state->selected_index + 1) % data->table.row_count;
            memcpy(data->storage, &state->selected_index, sizeof(size_t));
            if(data->on_select != nullptr) data->on_select();
            tui_widget_table_auto_scroll(state);
            return true;
        case KEY_ENTER:
            if(data->on_click != nullptr){
                data->on_click();
                return true;
            }
            return false;
        default:
            break;
        }
    default:
        break;
    }
    return false;
}

void tui_widget_table_(const char *widget_id, WidgetTableParams *params){
    assert(params != nullptr);
    assert(params->storage != nullptr);
    assert(params->table.column_count > 0);
    assert(params->table.row_count > 0);

    WidgetTableDataInternal *data = (WidgetTableDataInternal *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(WidgetTableDataInternal)
    );
    data->storage   = params->storage;
    data->table     = params->table;
    data->on_select = params->on_select;
    data->on_click  = params->on_click;
    data->panel     = tui_get_panel_building();

    int col_widths_size = (int)params->table.column_count * sizeof(int);
    data->column_widths = (int *)arena_alloc(
        LAYOUT_STATE.arena_frame, (size_t)col_widths_size
    );
    data->total_width = tui_widget_table_compute_column_widths(data);

    WidgetTableState *state = (WidgetTableState *)tui_widget_state(
        widget_id, sizeof(WidgetTableState)
    );
    state->selected_index = *params->storage;

    Widget new_widget = {
        .id        = widget_id,
        .data      = data,
        .state     = state,
        .size.w    = data->total_width + PADDING * 2,
        .size.h    = 1 + (int)params->table.row_count,
        .focusable = true,
        .is_inline = params->is_inline,
        .input     = &tui_widget_table_input,
        .render    = &tui_widget_table_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_TABLE_IMPL
#endif //TUI_WIDGET_TABLE

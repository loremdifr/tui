#ifndef TUI_WIDGET_CANVAS
#define TUI_WIDGET_CANVAS

#include "tui_layout.h"

#define tui_widget_canvas(widget_id, ...) \
        tui_widget_canvas_((widget_id), &(_WidgetCanvasParams){__VA_ARGS__})

typedef void (*CanvasRenderFunction)(Screen *screen, vec2i position);

typedef struct {
    vec2i size;
    CanvasRenderFunction on_render;
} _WidgetCanvasData;

typedef struct {
    vec2i size;
    CanvasRenderFunction on_render;
} _WidgetCanvasParams;

void tui_widget_canvas_(const char *widget_id, _WidgetCanvasParams *params);

#ifdef TUI_WIDGET_CANVAS_IMPL

static void _tui_widget_canvas_render(Widget *widget, Screen *screen, vec2i position){
    _WidgetCanvasData *widget_data = widget->data;
    if(widget_data->on_render){
        widget_data->on_render(screen, position);
    }
}

void tui_widget_canvas_(const char *widget_id, _WidgetCanvasParams *params){
    _WidgetCanvasData *widget_data = (_WidgetCanvasData *)arena_alloc(
        LAYOUT_STATE.arena_frame, sizeof(_WidgetCanvasData)
    );
    widget_data->size      = params->size;
    widget_data->on_render = params->on_render;

    Widget new_widget = {
        .id        = widget_id,
        .data      = widget_data,
        .size      = params->size,
        .focusable = false,
        .render    = &_tui_widget_canvas_render,
    };
    tui_widget_push(new_widget);
}

#endif //TUI_WIDGET_CANVAS_IMPL
#endif //TUI_WIDGET_CANVAS

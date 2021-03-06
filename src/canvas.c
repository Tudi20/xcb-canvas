#include "canvas.h"

#ifndef CANVAS_PATH_TYPES
#define CANVAS_PATH_TYPES
/* Encapsulated types */
struct arc_t {
    int16_t start_radius_or_cp2_x;
    int16_t end_radius_or_cp2_y;
    uint16_t radius;
};

enum sub_path_type_t {
    SUBPATH_TYPE_MOVE,
    SUBPATH_TYPE_LINE,
    SUBPATH_TYPE_ARC,
    SUBPATH_TYPE_ARC_TO,
    SUBPATH_TYPE_QUADRATIC_CURVE,
    SUBPATH_TYPE_CUBIC_CURVE,
    SUBPATH_TYPE_CLOSE
};

struct sub_path_t {
    xcb_point_t point;
    arc_t arc;
    sub_path_type_t type;
};
#endif /* CANVAS_PATH_TYPES */

void canvas_dealloc(canvas_rendering_context_t* rendering_context)
{
    free(rendering_context->canvas);
    free(rendering_context->path);
    free(rendering_context->font);
    free(rendering_context);
}

void canvas_stroke_rectangle(
    canvas_rendering_context_t* rendering_context,
    int16_t x, int16_t y,
    uint16_t width, uint16_t height
)
{
    xcbcanvas_stroke_rectangle(rendering_context->canvas, x, y, width, height);
}

void canvas_fill_rectangle(
    canvas_rendering_context_t* rendering_context,
    int16_t x, int16_t y,
    uint16_t width, uint16_t height
)
{
    xcbcanvas_fill_rectangle(rendering_context->canvas, x, y, width, height);
}

void canvas_clear_rectangle(
    canvas_rendering_context_t* rendering_context,
    int16_t x, int16_t y,
    uint16_t width, uint16_t height
)
{
    uint32_t stack_color = rendering_context->strokeColor;
    xcbcanvas_set_color(rendering_context->canvas, 0);
    xcbcanvas_fill_rectangle(rendering_context->canvas, x, y, width, height);
    xcbcanvas_set_color(rendering_context->canvas, stack_color);
}

void canvas_fill_text(
    canvas_rendering_context_t* rendering_context,
    const char* text,
    int16_t x, int16_t y
)
{
    xcbcanvas_draw_text(rendering_context->canvas, x, y, text);
}

void canvas_font4(
    canvas_rendering_context_t* rendering_context,
    const char* family,
    uint16_t size,
    const char* weight,
    _Bool italic
)
{
    xcbcanvas_font_t font;
    font.family = family;
    font.size = size;
    font.weight = weight;
    font.italic = italic;
    ctx_update_font(rendering_context, font);
}

void canvas_font2(
    canvas_rendering_context_t* rendering_context,
    const char* family,
    uint16_t size
)
{
    canvas_font4(rendering_context, family, size, "Medium", 0);
}

void canvas_set_color(
    canvas_rendering_context_t* rendering_context,
    uint8_t red, uint8_t green, uint8_t blue
)
{
    uint32_t color = (red << 16) | (green << 8) | blue;
    xcbcanvas_set_color(rendering_context->canvas, color);
}

void canvas_set_line_width(
    canvas_rendering_context_t* rendering_context,
    uint16_t width
)
{
    xcbcanvas_set_stroke_width(rendering_context->canvas, width);
}

void canvas_begin_path(
    canvas_rendering_context_t* rendering_context
)
{
    rendering_context->path->sub_path_count = 0;
    rendering_context->path->sub_paths = malloc(sizeof(sub_path_t) * 10);
}

void canvas_move_to(
    canvas_rendering_context_t* rendering_context,
    int16_t x, int16_t y
)
{
    if (rendering_context->path->sub_path_count % 10 == 9)
    {
        rendering_context->path->sub_paths = realloc(rendering_context->path->sub_paths, sizeof(sub_path_t) * (rendering_context->path->sub_path_count + 10));
    }
    sub_path_t* sub_path = &rendering_context->path->sub_paths[rendering_context->path->sub_path_count];
    sub_path->point.x = x;
    sub_path->point.y = y;
    sub_path->type = SUBPATH_TYPE_MOVE;
    rendering_context->path->sub_path_count++;
}

void canvas_line_to(
    canvas_rendering_context_t* rendering_context,
    int16_t x, int16_t y
)
{
    if (rendering_context->path->sub_path_count % 10 == 9)
    {
        rendering_context->path->sub_paths = realloc(rendering_context->path->sub_paths, sizeof(sub_path_t) * (rendering_context->path->sub_path_count + 10));
    }
    sub_path_t* sub_path = &rendering_context->path->sub_paths[rendering_context->path->sub_path_count];
    sub_path->point.x = x;
    sub_path->point.y = y;
    sub_path->type = SUBPATH_TYPE_LINE;
    rendering_context->path->sub_path_count++;
}

void canvas_arc(
    canvas_rendering_context_t* rendering_context,
    int16_t x, int16_t y,
    uint16_t radius,
    uint16_t start_angle,
    uint16_t end_angle,
    _Bool clockwise
)
{
    if (rendering_context->path->sub_path_count % 10 == 9)
    {
        rendering_context->path->sub_paths = realloc(rendering_context->path->sub_paths, sizeof(sub_path_t) * (rendering_context->path->sub_path_count + 10));
    }
    sub_path_t* sub_path = &rendering_context->path->sub_paths[rendering_context->path->sub_path_count];
    sub_path->point.x = x;
    sub_path->point.y = y;
    sub_path->arc.radius = radius;
    if (!clockwise)
    {
        sub_path->arc.start_radius_or_cp2_x = abs(start_angle);
        sub_path->arc.end_radius_or_cp2_y = abs(end_angle);
    }
    else
    {
        sub_path->arc.start_radius_or_cp2_x = abs(start_angle) * -1;
        sub_path->arc.end_radius_or_cp2_y = abs(end_angle) * -1;
    }
    sub_path->type = SUBPATH_TYPE_ARC;
    rendering_context->path->sub_path_count++;
}

void canvas_arc_to(
    canvas_rendering_context_t* rendering_context,
    int16_t x1, int16_t y1,
    int16_t x2, int16_t y2,
    uint16_t radius
)
{
    if (rendering_context->path->sub_path_count % 10 == 9)
    {
        rendering_context->path->sub_paths = realloc(rendering_context->path->sub_paths, sizeof(sub_path_t) * (rendering_context->path->sub_path_count + 10));
    }
    sub_path_t* sub_path = &rendering_context->path->sub_paths[rendering_context->path->sub_path_count];
    sub_path->point.x = x1;
    sub_path->point.y = y1;
    sub_path->arc.start_radius_or_cp2_x = x2;
    sub_path->arc.end_radius_or_cp2_y = y2;
    sub_path->arc.radius = radius;
    sub_path->type = SUBPATH_TYPE_ARC_TO;
    rendering_context->path->sub_path_count++;
}

void canvas_close_path(
    canvas_rendering_context_t* rendering_context
)
{
    if (rendering_context->path->sub_path_count % 10 == 9)
    {
        rendering_context->path->sub_paths = realloc(rendering_context->path->sub_paths, sizeof(sub_path_t) * (rendering_context->path->sub_path_count + 10));
    }
    sub_path_t* sub_path = &rendering_context->path->sub_paths[rendering_context->path->sub_path_count];
    sub_path->point = (xcb_point_t){ 0, 0 };
    sub_path->type = SUBPATH_TYPE_CLOSE;
    rendering_context->path->sub_path_count++;
}

void canvas_stroke(
    canvas_rendering_context_t* rendering_context
)
{
    xcbcanvas_draw_path(rendering_context->canvas, rendering_context->path);
}

void canvas_fill(
    canvas_rendering_context_t* rendering_context
)
{
    rendering_context->path->filled = 1;
    canvas_close_path(rendering_context);
    xcbcanvas_draw_path(rendering_context->canvas, rendering_context->path);
}
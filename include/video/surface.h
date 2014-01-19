#ifndef _SURFACE_H
#define _SURFACE_H

#include "video/image.h"
#include "video/screen_palette.h"

typedef struct {
    int w;
    int h;
    int type;
    char *data;
    char *stencil;
} surface;

enum {
    SURFACE_TYPE_RGBA,
    SURFACE_TYPE_PALETTE
};

enum {
    SUB_METHOD_NONE,
    SUB_METHOD_MIRROR
};

void surface_create(surface *sur, int type, int w, int h);
void surface_create_from_image(surface *sur, image *img);
void surface_create_from_data(surface *sur, int type, int w, int h, const char *src);
void surface_copy(surface *dst, surface *src);
void surface_free(surface *sur);
void surface_sub(surface *dst, 
                 surface *src,
                 int dst_x, int dst_y,
                 int src_x, int src_y,
                 int w, int h,
                 int method);
void surface_convert_to_rgba(surface *sur, screen_palette *pal, int pal_offset);
int surface_get_type(surface *sur);
void surface_to_rgba(surface *sur,
                     char *dst,
                     screen_palette *pal, 
                     char *remap_table,
                     uint8_t pal_offset);

#endif // _SURFACE_H
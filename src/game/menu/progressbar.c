#include <stdlib.h>

#include "game/menu/progressbar.h"
#include "game/menu/widget.h"
#include "video/surface.h"
#include "video/image.h"
#include "video/video.h"
#include "utils/miscmath.h"

const progressbar_theme _progressbar_theme_health = {
    .border_topleft_color = {60,0,60,255},
    .border_bottomright_color = {178,0,223,255},
    .bg_color = {89,40,101,255},
    .bg_color_alt = {89,40,101,255},
    .int_topleft_color = {255,0,0,255},
    .int_bottomright_color = {158,0,0,255},
    .int_bg_color = {255,56,109,255},
};

const progressbar_theme _progressbar_theme_endurance = {
    .border_topleft_color = {60,0,60,255},
    .border_bottomright_color = {178,0,223,255},
    .bg_color = {89,40,101,255},
    .bg_color_alt = {178,0,223,255},
    .int_topleft_color = {24,117,138,255},
    .int_bottomright_color = {0,69,93,255},
    .int_bg_color = {97,150,186,255},
};

const progressbar_theme _progressbar_theme_melee = {
    .border_topleft_color = {0, 96, 0, 255},
    .border_bottomright_color = {0, 96, 0, 255},
    .bg_color = {80, 220, 80, 0},
    .bg_color_alt = {80, 220, 80, 0},
    .int_topleft_color = {0, 255, 0, 255},
    .int_bottomright_color = {0, 125, 0, 255},
    .int_bg_color = {0, 190, 0, 255},
};

typedef struct {
    surface *background;
    surface *background_alt;
    surface *block;
    int orientation;
    int percentage;
    progressbar_theme theme;
    int flashing;
    int rate;
    int state;
    int tick;
    int refresh;
} progressbar;

void progressbar_set_progress(component *c, int percentage) {
    progressbar *bar = widget_get_obj(c);
    int tmp = clamp(percentage, 0, 100);
    if(!bar->refresh)
        bar->refresh = (tmp != bar->percentage);
    bar->percentage = tmp;
}

void progressbar_set_flashing(component *c, int flashing, int rate) {
    progressbar *bar = widget_get_obj(c);
    if(flashing != bar->flashing) {
        bar->tick = 0;
        bar->state = 0;
    }
    bar->flashing = clamp(flashing, 0, 1);
    bar->rate = (rate < 0) ? 0 : rate;
}

static void progressbar_render(component *c) {
    progressbar *bar = widget_get_obj(c);

    // If necessary, refresh the progress block
    if(bar->refresh) {
        bar->refresh = 0;

        // Free old block first ...
        surface_free(bar->block);

        // ... Then draw the new one
        float prog = bar->percentage / 100.0f;
        int w = c->w * prog;
        int h = c->h;
        if(w > 1 && h > 1) {
            image tmp;
            image_create(&tmp, w, h);
            image_clear(&tmp, bar->theme.int_bg_color);
            image_rect_bevel(&tmp,
                             0, 0,
                             w - 1, h - 1,
                             bar->theme.int_topleft_color,
                             bar->theme.int_bottomright_color,
                             bar->theme.int_bottomright_color,
                             bar->theme.int_topleft_color);
            surface_create_from_image(bar->block, &tmp);
            surface_disable_cache(bar->block, 1);
            image_free(&tmp);
        }
    }

    // Render backgrond (flashing or not)
    if(bar->state) {
        video_render_sprite(bar->background_alt, c->x, c->y, BLEND_ALPHA, 0);
    } else {
        video_render_sprite(bar->background, c->x, c->y, BLEND_ALPHA, 0);
    }

    // Render block
    if(bar->block != NULL) {
        video_render_sprite(
            bar->block,
            c->x + (bar->orientation == PROGRESSBAR_LEFT ? 0 : c->w - bar->block->w + 1),
            c->y,
            BLEND_ALPHA,
            0);
    }
}

static void progressbar_tick(component *c) {
    progressbar *bar = widget_get_obj(c);
    if(bar->flashing) {
        if(bar->tick > bar->rate) {
            bar->tick = 0;
            bar->state = !bar->state;
        }
        bar->tick++;
    }
}

static void progressbar_free(component *c) {
    progressbar *bar = widget_get_obj(c);
    free(bar->block);
    free(bar->background);
    free(bar->background_alt);
    free(bar);
}

static void progressbar_layout(component *c, int x, int y, int w, int h) {
    image tmp;
    progressbar *bar = widget_get_obj(c);

    // Allocate everything
    bar->background = malloc(sizeof(surface));
    bar->background_alt = malloc(sizeof(surface));
    bar->block = malloc(sizeof(surface));

    // Background,
    image_create(&tmp, w, h);
    image_clear(&tmp, bar->theme.bg_color);
    image_rect_bevel(&tmp,
                     0, 0, w-1, h-1,
                     bar->theme.border_topleft_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_topleft_color);
    surface_create_from_image(bar->background, &tmp);
    image_free(&tmp);

    image_create(&tmp, w, h);
    image_clear(&tmp, bar->theme.bg_color_alt);
    image_rect_bevel(&tmp,
                     0, 0, w-1, h-1,
                     bar->theme.border_topleft_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_bottomright_color,
                     bar->theme.border_topleft_color);
    surface_create_from_image(bar->background_alt, &tmp);
    image_free(&tmp);

    surface_create(bar->block, SURFACE_TYPE_RGBA, 0, 0);
    surface_disable_cache(bar->block, 1);
}

component* progressbar_create(progressbar_theme theme, int orientation, int percentage) {
    component *c = widget_create();
    c->supports_disable = 0;
    c->supports_select = 0;
    c->supports_focus = 0;

    progressbar *local = malloc(sizeof(progressbar));
    memset(local, 0, sizeof(progressbar));
    local->theme = theme;
    local->orientation = clamp(orientation, 0, 1);
    local->percentage = clamp(percentage, 0, 100);
    local->refresh = 1;

    widget_set_obj(c, local);
    widget_set_render_cb(c, progressbar_render);
    widget_set_tick_cb(c, progressbar_tick);
    widget_set_free_cb(c, progressbar_free);
    widget_set_layout_cb(c, progressbar_layout);

    return c;
}
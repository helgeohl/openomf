#include "game/scene.h"
#include "game/scenes/intro.h"
#include "game/animation.h"
#include "game/animationplayer.h"
#include "utils/array.h"
#include "utils/log.h"
#include "video/video.h"
#include <SDL2/SDL.h>
#include <shadowdive/shadowdive.h>

int scene_load(scene *scene, unsigned int scene_id) {
    scene->bk = sd_bk_create();
    int ret = 0;
    
    // Load BK
    switch(scene_id) {
        case SCENE_INTRO: ret = sd_bk_load(scene->bk, "resources/INTRO.BK"); break;
        case SCENE_MENU:  ret = sd_bk_load(scene->bk, "resources/MAIN.BK");  break;
        default:
            sd_bk_delete(scene->bk);
            PERROR("Unknown scene_id!");
            return 1;
    }
    if(ret) {
        sd_bk_delete(scene->bk);
        PERROR("Unable to load BK file!");
        return 1;
    }
    scene->this_id = scene_id;
    scene->next_id = scene_id;
    scene->ticks = 0;
    
    // Load specific stuff
    switch(scene_id) {
        case SCENE_INTRO: intro_load(scene); break;
        //case SCENE_MENU: menu_load(scene); break;
        default: 
            scene->render = 0;
            scene->event = 0;
    }
    
    // Convert background
    sd_rgba_image *bg = sd_vga_image_decode(scene->bk->background, scene->bk->palettes[0], -1);
    texture_create(&scene->background, bg->data, bg->w, bg->h);
    video_set_background(&scene->background);
    sd_rgba_image_delete(bg);
    
    // Players list
    list_create(&scene->players);
    
    // Handle animations
    animation *ani;
    sd_bk_anim *bka;
    array_create(&scene->animations);
    for(int i = 0; i < 50; i++) {
        bka = scene->bk->anims[i];
        if(bka) {
            // Create animation + textures, etc.
            ani = malloc(sizeof(animation));
            animation_create(ani, bka->animation, scene->bk->palettes[0], -1, scene->bk->soundtable);
            array_insert(&scene->animations, i, ani);
            
            // Start playback on those animations, that have load_on_start flag as true
            if(bka->load_on_start) {
                DEBUG("Loading animation %u on startup!", i);
                animationplayer *player = malloc(sizeof(animationplayer));
                animationplayer_create(player, ani);
                list_push_last(&scene->players, player);
            }
        }
    }
    
    // All done
    DEBUG("Scene %i loaded!", scene_id);
    return 0;
}

// Return 0 if event was handled here
int scene_handle_event(scene *scene, SDL_Event *event) {
    if(scene->event) {
        scene->event(scene, event);
    }
    return 1;
}

void scene_render(scene *scene, unsigned int delta) {
    scene->ticks += delta;
    list_iterator it;
    list_iter(&scene->players, &it);
    animationplayer *player;
    while((player = list_next(&it)) != 0) {
        animationplayer_run(player, delta);
    }
    if(scene->render) {
        scene->render(scene, delta);
    }
}

void scene_free(scene *scene) {
    if(!scene) return;
    
    // Release background
    video_set_background(0);
    texture_free(&scene->background);
    
    // Free players
    list_iterator lit;
    animationplayer *player = 0;
    list_iter(&scene->players, &lit);
    while((player = list_next(&lit)) != 0) {
        animationplayer_free(player);
        free(player);
    }
    list_free(&scene->players);
    
    // Free animations
    array_iterator it;
    animation *ani = 0;
    array_iter(&scene->animations, &it);
    while((ani = array_next(&it)) != 0) {
        animation_free(ani);
        free(ani);
    }
    array_free(&scene->animations);

    // Free BK
    sd_bk_delete(scene->bk);
}
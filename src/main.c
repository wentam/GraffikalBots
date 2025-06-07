#define _WIN32_WINNT 0x0500
#include "main.h"

#ifdef LINUX
/* forward renderer for anti-aliasing */
#define RENDERER GFKS_RENDERER_FORWARD
#endif
#ifdef _WIN32
/* deferred renderer because the forward renderer doesn't work on windows yet */
#define RENDERER GFKS_RENDERER_DEFERRED
#endif

#include "scan_arc.h"

#include <graffiks/graffiks.h>
#include <graffiks/material.h>
#include <graffiks/model_loaders/obj_loader.h>
#include <graffiks/renderer/renderer.h>
#include <graffiks/object.h>
#include <graffiks/lights.h>
#include <graffiks/camera.h>

#include <bots/bots.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <signal.h>

bots_world *g;
gfks_object **bots;
gfks_object **bot_turrets;
gfks_object **arcs;
gfks_object **shots;
gfks_point_light *l;
int bot_count = 0;

void intHandler(int i){
  exit(0);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, intHandler);

  // initialize engine
  bots_world_config my_config = bots_default_world_config();
  g = bots_world_new(&my_config);

  if (argc == 3) {
    bots_world_add_bot(g, argv[1]);
    bots_world_add_bot(g, argv[2]);
  } else {
    printf("Please provide two robots\n");
    return 0;
  }

  bot_count = 2;

  bots_world_place_bots(g);

  // initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("Failed to initialize SDL: %s\n",SDL_GetError());
  }

//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

  //SDL_Window *window = SDL_CreateWindow(
  //    "bots", 0, 0, 1024, 768,
  //    SDL_WINDOW_OPENGL);
  SDL_Window *window = SDL_CreateWindow(
      "bots", 0, 0, 1024, 768,
      0);

  if (window == NULL) {
    printf("Failed to create window: %s\n",SDL_GetError());
  }

  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  if (SDL_GetWindowWMInfo(window, &wmInfo) == SDL_FALSE) {
    printf("Failed to get window info: %s\n",SDL_GetError());
  }

  gfks_init_with_window_dt(wmInfo.info.x11.display, wmInfo.info.x11.window, init, update, done);
}

void init(int *width, int *height) {
  gfks_init_renderers(RENDERER);

  bots = malloc(sizeof(gfks_object *) * bot_count);
  bot_turrets = malloc(sizeof(gfks_object *) * bot_count);
  arcs = malloc(sizeof(gfks_object *) * bot_count);

  for (int i = 0; i < bot_count; i++) {
    bots[i] = gfks_load_obj(RENDERER, "bot.obj");
    bot_turrets[i] = gfks_load_obj(RENDERER, "bot_turret.obj");
    arcs[i] = create_scan_arc(RENDERER, 128, 500);
    arcs[i]->location_z = -0.12;
  }

  l = gfks_add_point_light();
  l->z = 5;

  gfks_camera *c = gfks_create_camera();
  gfks_set_camera_location(c, 0, 0, 12);
  gfks_set_active_camera(c);
}

//void handle_engine_event(bots_event *e) {
//  switch (e->event_type) {
//    case BOTS_EVENT_SCAN:
//   //   printf("scan, showing scan arc, %i\n",e->bot_id);
//      break;
//    case BOTS_EVENT_FIRE:
//    //  printf("bang!\n");
//      break;
//    case  BOTS_EVENT_HIT:
//     // printf("hit tank %i\n", e->bot_id);
//      break;
//    case BOTS_EVENT_DEATH:
//      //printf("bot %i ded\n", e->bot_id);
//      break;
//  }
//}

int previous_shot_count = 0;
float time_bottle = 0;

void update(float time_step) {
  int i;
  //bots_events *bots_events;

  time_bottle += time_step;

  if (time_bottle > 16.6) {
    bots_tick(g);

    //for (int i = 0; i < bots_events->event_count; i++) {
    //  handle_engine_event(bots_events->events+i);
    //}

    time_bottle -= 16.6;
  } 

  // mouse test
  SDL_Event e;
  while (SDL_PollEvent(&e)!=0) {
  }
 
  int x = 5;
  int y = 5;
  SDL_GetMouseState(&x, &y);
  //printf("mouse location: %i,%i\n",x,y);

  // loop over bots
  for (i = 0; i < bot_count; i++) {
    // bot location
    bots[i]->location_x = ((float)bots_get_tank(g,i)->x) / 200;
    bots[i]->location_y = ((float)bots_get_tank(g,i)->y) / 200;

    // bot angle
    bots[i]->angle = ((float)bots_get_tank(g,i)->heading) * 1.4;
    bots[i]->rot_x = 0;
    bots[i]->rot_y = 0;
    bots[i]->rot_z = -1;

    // bot turret location
    bot_turrets[i]->location_x = (((float)bots_get_tank(g,i)->x) / 200);
    bot_turrets[i]->location_y = (((float)bots_get_tank(g,i)->y) / 200);

    // bot turret angle
    bot_turrets[i]->angle =
        ((float)bots_get_tank(g,i)->heading + (float)bots_get_tank(g,i)->turret_offset) * 1.4;
    bot_turrets[i]->rot_x = 0;
    bot_turrets[i]->rot_y = 0;
    bot_turrets[i]->rot_z = -1;

    // scan arc
    //int scan_arc = g->cpus[i]->memory[0xfee5];
    int scan_arc = 30; // TODO tmp, interface changed and now we don't have memory or access to this number

    if (scan_arc > 64) {
      scan_arc = 64;
    }

    if (i == 0) {
    printf("scan arc %i\n", scan_arc);
    }

    //int scan_range = g->cpus[i]->memory[0xfee6] << 8;
    //scan_range |= g->cpus[i]->memory[0xfee7];
    int scan_range = 100; // TODO tmp, interface changed and now we don't have memory or access to this number

    update_scan_arc(arcs[i], scan_arc * 2, scan_range);

    arcs[i]->location_x = bots[i]->location_x;
    arcs[i]->location_y = bots[i]->location_y;
    float angle = (float)bots_get_tank(g, i)->heading +
                  (float)bots_get_tank(g, i)->scanner_offset;
    angle *= 1.4;
    angle += (scan_arc * 1.4);

    arcs[i]->angle = angle;
    arcs[i]->rot_x = 0;
    arcs[i]->rot_y = 0;
    arcs[i]->rot_z = -1;

    // if this bot is dead, hide it
    if (bots_get_tank(g, i)->health <= 0) {
      gfks_hide_object(bots[i]);
      gfks_hide_object(bot_turrets[i]);
      gfks_hide_object(arcs[i]);
    }
  }

  // shots
  // TODO we don't have access to the data needed to display shots anymore
  //int shot_count = 0;
  //i = 0;
  //while (1) {
  //  if (g->shots[i] == NULL) {
  //    shot_count = i;
  //    break;
  //  }

  //  i++;
  //}

  //printf("shot_count: %i\n", shot_count);

  //if (previous_shot_count > shot_count) {
  //  for (i = previous_shot_count - 1; i >= shot_count; i--) {
  //    gfks_remove_object(shots[i]);
  //  }
  //}

  //shots = realloc(shots, sizeof(gfks_object *) * shot_count);

  //if (shot_count > previous_shot_count) {
  //  for (i = previous_shot_count; i < shot_count; i++) {
  //    shots[i] = gfks_load_obj(RENDERER, "bullet.obj");
  //  }
  //}

  //previous_shot_count = shot_count;

  //for (i = 0; i < shot_count; i++) {
  //  shots[i]->location_x = ((float)g->shots[i]->x) / 200;
  //  shots[i]->location_y = ((float)g->shots[i]->y) / 200;

  //  shots[i]->angle = ((float)g->shots[i]->heading) * 1.4;
  //  shots[i]->rot_x = 0;
  //  shots[i]->rot_y = 0;
  //  shots[i]->rot_z = -1;
  //}
}

void done() {
  gfks_terminate_renderers(RENDERER);
  free(bots);
  free(bot_turrets);
  free(arcs);
  bots_world_free(g);
}

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
  g = bots_create_world();

  if (argc == 3) {
    if (!bots_add_bot_from_file(g, argv[1]) ||
        !bots_add_bot_from_file(g, argv[2])) {
      printf("Unable to load bots\n");
      return 1;
    }
  } else {
    printf("Please provide two robots\n");
    return 0;
  }

  bot_count = 2;

  /* hack the bots into position
   *
   * We only do this because bots_add_bot doesn't position the bots at all
   * right now. In the future, it'll position them with some standard
   * algorithm.
   */
  g->tanks[0]->x = 200;
  g->tanks[0]->y = 200;

  // initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO)) {
    printf("Failed to initialize SDL: %s\n",SDL_GetError());
  }

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

  SDL_Window *window = SDL_CreateWindow(
      "GraffikalBots", 0, 0, 1024, 768, 
      SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);


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

  int i;
  for (i = 0; i < bot_count; i++) {
    bots[i] = gfks_load_obj(RENDERER, "bot.obj");
    bot_turrets[i] = gfks_load_obj(RENDERER, "bot_turret.obj");
    arcs[i] = create_scan_arc(RENDERER, 128, 500);
    arcs[i]->location_z = -0.11;
  }

  l = gfks_add_point_light();
  l->z = 5;

  gfks_camera *c = gfks_create_camera();
  gfks_set_camera_location(c, 0, 0, 7);
  gfks_set_active_camera(c);
}

int previous_shot_count = 0;
float time_bottle = 0;

void update(float time_step) {
  int i;
  bots_events *bots_events;

  time_bottle += time_step;

  if (time_bottle > 16.6) {
    bots_events = bots_tick(g);

    for (int i = 0; i < bots_events->event_count; i++) {
      printf("got event %i\n",bots_events->events[i].event_type);
    }

    time_bottle -= 16.6;
  } 

  // mouse test
  SDL_Event e;
  while (SDL_PollEvent(&e)!=0) {
  }

 
  int x = 5;
  int y = 5;
  SDL_GetMouseState(&x, &y);
  printf("mouse location: %i,%i\n",x,y);

  // loop over bots
  for (i = 0; i < bot_count; i++) {
    // bot location
    bots[i]->location_x = ((float)g->tanks[i]->x) / 200;
    bots[i]->location_y = ((float)g->tanks[i]->y) / 200;

    // bot angle
    bots[i]->angle = ((float)g->tanks[i]->heading) * 1.4;
    bots[i]->rot_x = 0;
    bots[i]->rot_y = 0;
    bots[i]->rot_z = -1;

    // bot turret location
    bot_turrets[i]->location_x = (((float)g->tanks[i]->x) / 200);
    bot_turrets[i]->location_y = (((float)g->tanks[i]->y) / 200);

    // bot turret angle
    bot_turrets[i]->angle =
        ((float)g->tanks[i]->heading + (float)g->tanks[i]->turret_offset) * 1.4;
    bot_turrets[i]->rot_x = 0;
    bot_turrets[i]->rot_y = 0;
    bot_turrets[i]->rot_z = -1;

    // scan arc
    int scan_arc = g->cpus[i]->ports[14] << 8;
    scan_arc |= g->cpus[i]->ports[15];

    if (scan_arc > 64) {
      scan_arc = 64;
    }

    int scan_range = g->cpus[i]->ports[16] << 8;
    scan_range |= g->cpus[i]->ports[17];

    update_scan_arc(arcs[i], scan_arc * 2, scan_range);

    arcs[i]->location_x = bots[i]->location_x;
    arcs[i]->location_y = bots[i]->location_y;
    float angle = (float)g->tanks[i]->heading +
                  (float)g->tanks[i]->scanner_offset;
    angle *= 1.4;
    angle += (scan_arc * 1.4);

    arcs[i]->angle = angle;
    arcs[i]->rot_x = 0;
    arcs[i]->rot_y = 0;
    arcs[i]->rot_z = -1;

    // if this bot is dead, hide it
    if (g->tanks[i]->health <= 0) {
      gfks_hide_object(bots[i]);
      gfks_hide_object(bot_turrets[i]);
      gfks_hide_object(arcs[i]);
    }
  }

  // shots
  int shot_count = 0;
  i = 0;
  while (1) {
    if (g->shots[i] == NULL) {
      shot_count = i;
      break;
    }

    i++;
  }

  //printf("shot_count: %i\n", shot_count);

  if (previous_shot_count > shot_count) {
    for (i = previous_shot_count - 1; i >= shot_count; i--) {
      gfks_remove_object(shots[i]);
    }
  }

  shots = realloc(shots, sizeof(gfks_object *) * shot_count);

  if (shot_count > previous_shot_count) {
    for (i = previous_shot_count; i < shot_count; i++) {
      shots[i] = gfks_load_obj(RENDERER, "bullet.obj");
    }
  }

  previous_shot_count = shot_count;

  for (i = 0; i < shot_count; i++) {
    shots[i]->location_x = ((float)g->shots[i]->x) / 200;
    shots[i]->location_y = ((float)g->shots[i]->y) / 200;

    shots[i]->angle = ((float)g->shots[i]->heading) * 1.4;
    shots[i]->rot_x = 0;
    shots[i]->rot_y = 0;
    shots[i]->rot_z = -1;
  }
}

void done() {
  gfks_terminate_renderers(RENDERER);
  free(bots);
  free(bot_turrets);
  free(arcs);
  bots_free_world(g);
}

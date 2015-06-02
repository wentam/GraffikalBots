#define _WIN32_WINNT 0x0500
#include "main.h"

#ifdef LINUX
#include <graffiks/driver/driver-linux.h>
/* forward renderer for anti-aliasing */
#define RENDERER GRAFFIKS_RENDERER_FORWARD
#endif
#ifdef _WIN32
#include <graffiks/driver/driver-windows.h>
#include <windows.h>
/* deferred renderer because the forward renderer doesn't work on windows yet */
#define RENDERER GRAFFIKS_RENDERER_DEFERRED
#endif

#include <graffiks/material/material.h>
#include <graffiks/mesh/cube_mesh.h>
#include <graffiks/object/obj_loader.h>
#include <graffiks/renderer/renderer.h>
#include <graffiks/object/object.h>
#include <graffiks/lights.h>
#include <bots/bots.h>
#include "scan_arc.h"

bots_world *g;
object **bots;
object **bot_turrets;
object **arcs;
object **shots;
point_light *l;
int bot_count = 0;

int main(int argc, char *argv[]) {
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

#ifdef LINUX
  init_graffiks_xorg(1024, 768, "GraffikalBots", init, update, done);
#endif
#ifdef _WIN32
  HWND hwndConsole = GetConsoleWindow();
  HINSTANCE hInstance =
      (HINSTANCE)GetWindowLongPtr(hwndConsole, GWLP_HINSTANCE);

  init_graffiks_windows(1024, 768, "GraffikalBots", init, update, done,
                        hInstance);
#endif
}

void init(int *width, int *height) {
  init_renderers(RENDERER);

  bots = malloc(sizeof(object *) * bot_count);
  bot_turrets = malloc(sizeof(object *) * bot_count);
  arcs = malloc(sizeof(object *) * bot_count);

  int i;
  for (i = 0; i < bot_count; i++) {
    bots[i] = load_obj(RENDERER, "bot.obj");
    bot_turrets[i] = load_obj(RENDERER, "bot_turret.obj");
    arcs[i] = create_scan_arc(RENDERER, 128, 500);
    arcs[i]->location_z = -0.11;
  }

  l = add_point_light();
  l->z = 5;
}

int previous_shot_count = 0;

void update(float time_step) {
  int i;

  bots_tick(g);

  // loop over bots
  for (i = 0; i < bot_count; i++) {

    // if this bot is dead, hide it
    if (g->tanks[i]->health <= 0) {
      hide_object(bots[i]);
      hide_object(bot_turrets[i]);
    }

    // bot location
    bots[i]->location_x = ((float)g->tanks[i]->x) / 200;
    bots[i]->location_y = ((float)g->tanks[i]->y) / 200;

    // bot angle
    bots[i]->angle = ((float)g->tanks[i]->heading) * 1.4;
    bots[i]->rot_x = 0;
    bots[i]->rot_y = 0;
    bots[i]->rot_z = 1;

    // bot turret location
    bot_turrets[i]->location_x = (((float)g->tanks[i]->x) / 200);
    bot_turrets[i]->location_y = (((float)g->tanks[i]->y) / 200);

    // bot turret angle
    bot_turrets[i]->angle =
        ((float)g->tanks[0]->heading + (float)g->tanks[i]->turret_offset) * 1.4;
    bot_turrets[i]->rot_x = 0;
    bot_turrets[i]->rot_y = 0;
    bot_turrets[i]->rot_z = 1;

    // scan arc
    int scan_arc = g->cpus[i]->ports[14] << 8;
    scan_arc |= g->cpus[i]->ports[15];

    int scan_range = g->cpus[i]->ports[16];
    scan_range |= g->cpus[i]->ports[17];

    update_scan_arc(arcs[i], scan_arc, scan_range * 20);

    arcs[i]->location_x = bots[i]->location_x;
    arcs[i]->location_y = bots[i]->location_y;

    arcs[i]->angle = (bot_turrets[i]->angle) - ((scan_arc * 1.4) / 2);
    arcs[i]->rot_x = 0;
    arcs[i]->rot_y = 0;
    arcs[i]->rot_z = 1;
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

  if (previous_shot_count > shot_count) {
    for (i = previous_shot_count - 1; i >= shot_count; i--) {
      remove_object(shots[i]);
    }
  }

  shots = realloc(shots, sizeof(object *) * shot_count);

  if (shot_count > previous_shot_count) {
    for (i = previous_shot_count; i < shot_count; i++) {
      shots[i] = load_obj(RENDERER, "bullet.obj");
    }
  }

  previous_shot_count = shot_count;

  for (i = 0; i < shot_count; i++) {
    shots[i]->location_x = ((float)g->shots[i]->x) / 200;
    shots[i]->location_y = ((float)g->shots[i]->y) / 200;

    shots[i]->angle = ((float)g->shots[i]->heading) * 1.4;
    shots[i]->rot_x = 0;
    shots[i]->rot_y = 0;
    shots[i]->rot_z = 1;
  }
}

void done() {
  terminate_renderers(RENDERER);
  free(bots);
  free(bot_turrets);
  free(arcs);
  bots_free_world(g);
}

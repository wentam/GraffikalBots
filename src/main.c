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
object *bot1;
object *bot2;
object *bot1_turret;
object *bot2_turret;
object *arc1;
object *arc2;
object **shots;
point_light *l;

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

  bot1 = load_obj(RENDERER, "bot.obj");
  bot2 = load_obj(RENDERER, "bot.obj");
  bot1_turret = load_obj(RENDERER, "bot_turret.obj");
  bot2_turret = load_obj(RENDERER, "bot_turret.obj");

  l = add_point_light();
  l->z = 5;

  arc1 = create_scan_arc(RENDERER, 16, 500);
  arc2 = create_scan_arc(RENDERER, 16, 500);
}

int previous_shot_count = 0;

void update(float time_step) {
  bots_tick(g);

  if (g->tanks[0]->health <= 0) {
    hide_object(bot1);
    hide_object(bot1_turret);
  }

  if (g->tanks[1]->health <= 0) {
    hide_object(bot2);
    hide_object(bot2_turret);
  }

  // location
  bot1->location_x = ((float)g->tanks[0]->x) / 200;
  bot1->location_y = ((float)g->tanks[0]->y) / 200;
  bot1_turret->location_x = (((float)g->tanks[0]->x) / 200);
  bot1_turret->location_y = (((float)g->tanks[0]->y) / 200);
  bot2->location_x = ((float)g->tanks[1]->x) / 200;
  bot2->location_y = ((float)g->tanks[1]->y) / 200;
  bot2_turret->location_x = ((float)g->tanks[1]->x) / 200;
  bot2_turret->location_y = ((float)g->tanks[1]->y) / 200;

  bot1->angle = ((float)g->tanks[0]->heading) * 1.4;
  bot1->rot_x = 0;
  bot1->rot_y = 0;
  bot1->rot_z = 1;

  // turret angle
  int scan_arc = g->cpus[0]->ports[14] << 8;
  scan_arc |= g->cpus[0]->ports[15];

  int scan_range = g->cpus[0]->ports[16];
  scan_range |= g->cpus[0]->ports[17];

  update_scan_arc(arc1, scan_arc, scan_range * 20);

  scan_arc = g->cpus[1]->ports[14] << 8;
  scan_arc |= g->cpus[1]->ports[15];

  scan_range = g->cpus[1]->ports[16];
  scan_range |= g->cpus[1]->ports[17];

  update_scan_arc(arc2, scan_arc, scan_range * 20);

  bot1_turret->angle =
      ((float)g->tanks[0]->heading + (float)g->tanks[0]->turret_offset) * 1.4;
  bot1->rot_x = 0;
  bot1->rot_y = 0;
  bot1->rot_z = 1;

  bot2->angle = ((float)g->tanks[1]->heading) * 1.4;
  bot2->rot_x = 0;
  bot2->rot_y = 0;
  bot2->rot_z = 1;

  bot2_turret->angle =
      ((float)g->tanks[1]->heading + (float)g->tanks[1]->turret_offset) * 1.4;
  bot2->rot_x = 0;
  bot2->rot_y = 0;
  bot2->rot_z = 1;

  // scan arc
  arc1->location_x = bot1->location_x;
  arc1->location_y = bot1->location_y;

  arc2->location_x = bot2->location_x;
  arc2->location_y = bot2->location_y;

  arc1->angle = (bot1_turret->angle) - ((scan_arc * 1.4) / 2);
  arc1->rot_x = 0;
  arc1->rot_y = 0;
  arc1->rot_z = 1;

  arc2->angle = (bot2_turret->angle) - ((scan_arc * 1.4) / 2);
  arc2->rot_x = 0;
  arc2->rot_y = 0;
  arc2->rot_z = 1;

  // shots
  int shot_count = 0;
  int i = 0;
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
  bots_free_world(g);
}

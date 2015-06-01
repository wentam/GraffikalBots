#include "main.h"
#include <graffiks/driver/driver-linux.h>
#include <graffiks/material/material.h>
#include <graffiks/mesh/cube_mesh.h>
#include <graffiks/object/obj_loader.h>
#include <graffiks/renderer/renderer.h>
#include <graffiks/object/object.h>
#include <graffiks/lights.h>
#include <bots/bots.h>

bots_world *g;
object *bot1;
object *bot2;
object *bot1_turret;
object *bot2_turret;
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
    return;
  }

  /* hack the bots into position
   *
   * We only do this because bots_add_bot doesn't position the bots at all
   * right now. In the future, it'll position them with some standard
   * algorithm.
   */
  g->tanks[0]->x = 200;
  g->tanks[0]->y = 200;

  init_graffiks_xorg(1024, 768, "GraffikalBots", init, update, done);
}

void init(int *width, int *height) {
  init_renderers(GRAFFIKS_RENDERER_FORWARD);

  bot1 = load_obj(GRAFFIKS_RENDERER_FORWARD, "bot.obj");
  bot2 = load_obj(GRAFFIKS_RENDERER_FORWARD, "bot.obj");
  bot1_turret = load_obj(GRAFFIKS_RENDERER_FORWARD, "bot_turret.obj");
  bot2_turret = load_obj(GRAFFIKS_RENDERER_FORWARD, "bot_turret.obj");

  l = add_point_light();
  l->z = 5;
}

void update(float time_step) {
  bots_tick(g);

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
      ((float)g->tanks[0]->heading + (float)g->tanks[0]->turret_offset) * 1.4;
  bot2->rot_x = 0;
  bot2->rot_y = 0;
  bot2->rot_z = 1;
}

void done() {
  terminate_renderers(GRAFFIKS_RENDERER_FORWARD);
  bots_free_world(g);
}

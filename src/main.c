#include "main.h"
#include <graffiks/driver/driver-linux.h>
#include <graffiks/material/material.h>
#include <graffiks/mesh/cube_mesh.h>
#include <graffiks/object/obj_loader.h>
#include <graffiks/renderer/renderer.h>
#include <graffiks/object/object.h>
#include <graffiks/lights.h>
#include <graffiks/gl_helper.h>

int main() {
  init_graffiks_xorg(1024, 768, "GraffikalBots", init, update, done);
}

void init(int *width, int *height) {
  init_renderers(GRAFFIKS_RENDERER_DEFERRED);
}

void update(float time_step) {}

void done() { terminate_renderers(GRAFFIKS_RENDERER_DEFERRED); }

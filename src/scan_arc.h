#ifndef SCAN_ARC_H
#define SCAN_ARC_H
#include <graffiks/object/object.h>
#include <graffiks/renderer/renderer.h>

// degrees is 0-256
gfks_object *create_scan_arc(gfks_renderer_flags flags, int degrees,
                             int scan_range);
void update_scan_arc(gfks_object *o, int degrees, int scan_range);

#endif

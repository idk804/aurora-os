/*
 * Aurora Compositor - View (Window) Management
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include "server.h"

/* Begin interactive move */
void aurora_begin_interactive_move(struct aurora_view *view) {
    struct aurora_server *server = view->server;
    if (view->xdg_toplevel->base->surface !=
            wlr_surface_get_root_surface(
                server->seat->pointer_state.focused_surface)) {
        return;
    }
    server->grabbed_view = view;
    server->cursor_mode = AURORA_CURSOR_MOVE;
    server->grab_x = server->cursor->x - view->scene_tree->node.x;
    server->grab_y = server->cursor->y - view->scene_tree->node.y;
}

/* Begin interactive resize */
void aurora_begin_interactive_resize(struct aurora_view *view, uint32_t edges) {
    struct aurora_server *server = view->server;
    if (view->xdg_toplevel->base->surface !=
            wlr_surface_get_root_surface(
                server->seat->pointer_state.focused_surface)) {
        return;
    }

    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);

    double border_x = (view->scene_tree->node.x + geo_box.x) +
        ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->scene_tree->node.y + geo_box.y) +
        ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);

    server->grabbed_view = view;
    server->cursor_mode = AURORA_CURSOR_RESIZE;
    server->resize_edges = edges;
    server->grab_x = server->cursor->x - border_x;
    server->grab_y = server->cursor->y - border_y;
    server->grab_geobox = geo_box;
    server->grab_geobox.x += view->scene_tree->node.x;
    server->grab_geobox.y += view->scene_tree->node.y;
}

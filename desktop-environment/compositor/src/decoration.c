/*
 * Aurora Compositor - Window Decoration
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include "server.h"

struct aurora_decoration {
    struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration;
    struct aurora_server *server;
    struct wl_listener destroy;
    struct wl_listener request_mode;
};

static void decoration_handle_destroy(struct wl_listener *listener, void *data) {
    struct aurora_decoration *deco =
        wl_container_of(listener, deco, destroy);
    wl_list_remove(&deco->destroy.link);
    wl_list_remove(&deco->request_mode.link);
    free(deco);
}

static void decoration_handle_request_mode(struct wl_listener *listener, void *data) {
    struct aurora_decoration *deco =
        wl_container_of(listener, deco, request_mode);
    /* Aurora prefers server-side decorations for a consistent look */
    wlr_xdg_toplevel_decoration_v1_set_mode(deco->wlr_decoration,
        WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

void aurora_new_xdg_decoration(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, new_xdg_decoration);
    struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration = data;

    struct aurora_decoration *deco = calloc(1, sizeof(*deco));
    deco->wlr_decoration = wlr_decoration;
    deco->server = server;

    deco->destroy.notify = decoration_handle_destroy;
    wl_signal_add(&wlr_decoration->events.destroy, &deco->destroy);
    deco->request_mode.notify = decoration_handle_request_mode;
    wl_signal_add(&wlr_decoration->events.request_mode, &deco->request_mode);

    /* Set initial mode to server-side */
    wlr_xdg_toplevel_decoration_v1_set_mode(wlr_decoration,
        WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
}

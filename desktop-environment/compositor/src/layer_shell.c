/*
 * Aurora Compositor - Layer Shell (Panels, Widgets, Overlays)
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_scene.h>

#include "server.h"

struct aurora_layer_surface {
    struct aurora_server *server;
    struct wlr_layer_surface_v1 *layer_surface;
    struct wlr_scene_layer_surface_v1 *scene_layer_surface;

    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
};

static void layer_surface_map(struct wl_listener *listener, void *data) {
    struct aurora_layer_surface *surface =
        wl_container_of(listener, surface, map);
    wlr_log(WLR_INFO, "Layer surface mapped: %s",
        surface->layer_surface->namespace ?
            surface->layer_surface->namespace : "(none)");
}

static void layer_surface_unmap(struct wl_listener *listener, void *data) {
    struct aurora_layer_surface *surface =
        wl_container_of(listener, surface, unmap);
    wlr_log(WLR_INFO, "Layer surface unmapped");
    (void)surface;
}

static void layer_surface_commit(struct wl_listener *listener, void *data) {
    struct aurora_layer_surface *surface =
        wl_container_of(listener, surface, commit);
    struct wlr_layer_surface_v1 *layer = surface->layer_surface;
    struct wlr_output *output = layer->output;

    if (!output) {
        /* Assign to first available output */
        struct aurora_output *aurora_output;
        if (!wl_list_empty(&surface->server->outputs)) {
            aurora_output = wl_container_of(
                surface->server->outputs.next, aurora_output, link);
            output = aurora_output->wlr_output;
            layer->output = output;
        }
    }

    if (layer->initial_commit) {
        /* Get output dimensions for exclusive zone calculation */
        uint32_t width = output ? output->width : 0;
        uint32_t height = output ? output->height : 0;

        if (layer->current.desired_width == 0) {
            layer->current.desired_width = width;
        }
        if (layer->current.desired_height == 0) {
            layer->current.desired_height = height;
        }

        wlr_layer_surface_v1_configure(layer,
            layer->current.desired_width,
            layer->current.desired_height);
    }
}

static void layer_surface_destroy(struct wl_listener *listener, void *data) {
    struct aurora_layer_surface *surface =
        wl_container_of(listener, surface, destroy);

    wl_list_remove(&surface->map.link);
    wl_list_remove(&surface->unmap.link);
    wl_list_remove(&surface->commit.link);
    wl_list_remove(&surface->destroy.link);
    free(surface);
}

void aurora_layer_new_surface(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, new_layer_surface);
    struct wlr_layer_surface_v1 *layer_surface = data;

    struct aurora_layer_surface *surface = calloc(1, sizeof(*surface));
    surface->server = server;
    surface->layer_surface = layer_surface;

    /* Create scene layer surface based on layer */
    enum zwlr_layer_shell_v1_layer layer = layer_surface->pending.layer;
    struct wlr_scene_tree *parent = &server->scene->tree;

    surface->scene_layer_surface =
        wlr_scene_layer_surface_v1_create(parent, layer_surface);

    surface->map.notify = layer_surface_map;
    wl_signal_add(&layer_surface->surface->events.map, &surface->map);
    surface->unmap.notify = layer_surface_unmap;
    wl_signal_add(&layer_surface->surface->events.unmap, &surface->unmap);
    surface->commit.notify = layer_surface_commit;
    wl_signal_add(&layer_surface->surface->events.commit, &surface->commit);
    surface->destroy.notify = layer_surface_destroy;
    wl_signal_add(&layer_surface->events.destroy, &surface->destroy);

    wlr_log(WLR_INFO, "New layer surface: namespace=%s layer=%d",
        layer_surface->namespace ? layer_surface->namespace : "(none)",
        layer);
}

/*
 * Aurora Compositor - XDG Shell (Window Protocol)
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include "server.h"

extern void aurora_begin_interactive_move(struct aurora_view *view);
extern void aurora_begin_interactive_resize(struct aurora_view *view, uint32_t edges);

static void xdg_toplevel_map(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, map);

    wl_list_insert(&view->server->views, &view->link);

    /* Start with fade-in animation */
    view->opacity = 0.0f;
    view->target_opacity = 1.0f;
    view->animating = true;

    aurora_focus_view(view, view->xdg_toplevel->base->surface);
    wlr_log(WLR_INFO, "Window mapped: %s",
        view->xdg_toplevel->title ? view->xdg_toplevel->title : "(untitled)");
}

static void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, unmap);
    wl_list_remove(&view->link);

    /* Reset grab if this view was grabbed */
    if (view == view->server->grabbed_view) {
        view->server->cursor_mode = AURORA_CURSOR_PASSTHROUGH;
        view->server->grabbed_view = NULL;
    }
}

static void xdg_toplevel_commit(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, commit);
    if (view->xdg_toplevel->base->initial_commit) {
        wlr_xdg_toplevel_set_size(view->xdg_toplevel, 0, 0);
    }
}

static void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, destroy);

    wl_list_remove(&view->map.link);
    wl_list_remove(&view->unmap.link);
    wl_list_remove(&view->commit.link);
    wl_list_remove(&view->destroy.link);
    wl_list_remove(&view->request_move.link);
    wl_list_remove(&view->request_resize.link);
    wl_list_remove(&view->request_maximize.link);
    wl_list_remove(&view->request_fullscreen.link);

    free(view);
}

static void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, request_move);
    aurora_begin_interactive_move(view);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, request_resize);
    struct wlr_xdg_toplevel_resize_event *event = data;
    aurora_begin_interactive_resize(view, event->edges);
}

static void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, request_maximize);
    if (view->xdg_toplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
    }
}

static void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data) {
    struct aurora_view *view = wl_container_of(listener, view, request_fullscreen);
    if (view->xdg_toplevel->base->initialized) {
        wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
    }
}

void aurora_xdg_new_toplevel(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, new_xdg_toplevel);
    struct wlr_xdg_toplevel *xdg_toplevel = data;

    struct aurora_view *view = calloc(1, sizeof(*view));
    view->server = server;
    view->xdg_toplevel = xdg_toplevel;
    view->opacity = 1.0f;
    view->target_opacity = 1.0f;
    view->animating = false;

    view->scene_tree = wlr_scene_xdg_surface_create(
        &server->scene->tree, xdg_toplevel->base);
    view->scene_tree->node.data = view;
    xdg_toplevel->base->data = view->scene_tree;

    /* Connect signals */
    view->map.notify = xdg_toplevel_map;
    wl_signal_add(&xdg_toplevel->base->surface->events.map, &view->map);
    view->unmap.notify = xdg_toplevel_unmap;
    wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &view->unmap);
    view->commit.notify = xdg_toplevel_commit;
    wl_signal_add(&xdg_toplevel->base->surface->events.commit, &view->commit);
    view->destroy.notify = xdg_toplevel_destroy;
    wl_signal_add(&xdg_toplevel->events.destroy, &view->destroy);

    view->request_move.notify = xdg_toplevel_request_move;
    wl_signal_add(&xdg_toplevel->events.request_move, &view->request_move);
    view->request_resize.notify = xdg_toplevel_request_resize;
    wl_signal_add(&xdg_toplevel->events.request_resize, &view->request_resize);
    view->request_maximize.notify = xdg_toplevel_request_maximize;
    wl_signal_add(&xdg_toplevel->events.request_maximize, &view->request_maximize);
    view->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
    wl_signal_add(&xdg_toplevel->events.request_fullscreen, &view->request_fullscreen);
}

void aurora_xdg_new_popup(struct wl_listener *listener, void *data) {
    struct wlr_xdg_popup *xdg_popup = data;

    struct wlr_xdg_surface *parent =
        wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
    if (!parent) {
        wlr_log(WLR_ERROR, "Popup has no parent surface");
        return;
    }

    struct wlr_scene_tree *parent_tree = parent->data;
    xdg_popup->base->data = wlr_scene_xdg_surface_create(
        parent_tree, xdg_popup->base);
}

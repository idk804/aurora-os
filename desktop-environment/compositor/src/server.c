/*
 * Aurora Compositor - Server Implementation
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <assert.h>
#include <stdlib.h>

#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_subcompositor.h>

#include "server.h"

/* Forward declarations for handlers defined in other files */
extern void aurora_output_new(struct wl_listener *listener, void *data);
extern void aurora_xdg_new_toplevel(struct wl_listener *listener, void *data);
extern void aurora_xdg_new_popup(struct wl_listener *listener, void *data);
extern void aurora_layer_new_surface(struct wl_listener *listener, void *data);
extern void aurora_new_xdg_decoration(struct wl_listener *listener, void *data);
extern void aurora_input_new(struct wl_listener *listener, void *data);
extern void aurora_cursor_motion(struct wl_listener *listener, void *data);
extern void aurora_cursor_motion_absolute(struct wl_listener *listener, void *data);
extern void aurora_cursor_button(struct wl_listener *listener, void *data);
extern void aurora_cursor_axis(struct wl_listener *listener, void *data);
extern void aurora_cursor_frame(struct wl_listener *listener, void *data);
extern void aurora_seat_request_cursor(struct wl_listener *listener, void *data);
extern void aurora_seat_request_selection(struct wl_listener *listener, void *data);

struct aurora_server *aurora_server_create(struct aurora_config *config) {
    struct aurora_server *server = calloc(1, sizeof(*server));
    if (!server) {
        wlr_log(WLR_ERROR, "Failed to allocate server");
        return NULL;
    }
    server->config = config;
    server->cursor_mode = AURORA_CURSOR_PASSTHROUGH;
    wl_list_init(&server->views);
    wl_list_init(&server->outputs);
    wl_list_init(&server->keyboards);
    return server;
}

bool aurora_server_init(struct aurora_server *server) {
    /* Create Wayland display */
    server->wl_display = wl_display_create();
    if (!server->wl_display) {
        wlr_log(WLR_ERROR, "Failed to create wl_display");
        return false;
    }
    server->event_loop = wl_display_get_event_loop(server->wl_display);

    /* Create backend */
    server->backend = wlr_backend_autocreate(
        wl_display_get_event_loop(server->wl_display), NULL);
    if (!server->backend) {
        wlr_log(WLR_ERROR, "Failed to create wlr_backend");
        return false;
    }

    /* Create renderer */
    server->renderer = wlr_renderer_autocreate(server->backend);
    if (!server->renderer) {
        wlr_log(WLR_ERROR, "Failed to create wlr_renderer");
        return false;
    }
    wlr_renderer_init_wl_display(server->renderer, server->wl_display);

    /* Create allocator */
    server->allocator = wlr_allocator_autocreate(
        server->backend, server->renderer);
    if (!server->allocator) {
        wlr_log(WLR_ERROR, "Failed to create wlr_allocator");
        return false;
    }

    /* Create compositing primitives */
    wlr_compositor_create(server->wl_display, 5, server->renderer);
    wlr_subcompositor_create(server->wl_display);
    wlr_data_device_manager_create(server->wl_display);

    /* Scene graph */
    server->scene = wlr_scene_create();
    server->output_layout = wlr_output_layout_create(server->wl_display);
    server->scene_layout = wlr_scene_attach_output_layout(
        server->scene, server->output_layout);

    /* XDG shell */
    server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
    server->new_xdg_toplevel.notify = aurora_xdg_new_toplevel;
    wl_signal_add(&server->xdg_shell->events.new_toplevel,
        &server->new_xdg_toplevel);
    server->new_xdg_popup.notify = aurora_xdg_new_popup;
    wl_signal_add(&server->xdg_shell->events.new_popup,
        &server->new_xdg_popup);

    /* Layer shell (for panels, widgets, etc.) */
    server->layer_shell = wlr_layer_shell_v1_create(server->wl_display, 4);
    server->new_layer_surface.notify = aurora_layer_new_surface;
    wl_signal_add(&server->layer_shell->events.new_surface,
        &server->new_layer_surface);

    /* XDG decoration */
    server->xdg_decoration_mgr =
        wlr_xdg_decoration_manager_v1_create(server->wl_display);
    server->new_xdg_decoration.notify = aurora_new_xdg_decoration;
    wl_signal_add(&server->xdg_decoration_mgr->events.new_toplevel_decoration,
        &server->new_xdg_decoration);

    /* Server-side decoration (prefer server-side for Aurora look) */
    server->server_decoration_mgr =
        wlr_server_decoration_manager_create(server->wl_display);
    wlr_server_decoration_manager_set_default_mode(
        server->server_decoration_mgr,
        WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);

    /* Output management */
    server->new_output.notify = aurora_output_new;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);

    /* Cursor */
    server->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
    server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);

    server->cursor_motion.notify = aurora_cursor_motion;
    wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);
    server->cursor_motion_absolute.notify = aurora_cursor_motion_absolute;
    wl_signal_add(&server->cursor->events.motion_absolute,
        &server->cursor_motion_absolute);
    server->cursor_button.notify = aurora_cursor_button;
    wl_signal_add(&server->cursor->events.button, &server->cursor_button);
    server->cursor_axis.notify = aurora_cursor_axis;
    wl_signal_add(&server->cursor->events.axis, &server->cursor_axis);
    server->cursor_frame.notify = aurora_cursor_frame;
    wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);

    /* Seat (input handling) */
    server->seat = wlr_seat_create(server->wl_display, "seat0");
    server->new_input.notify = aurora_input_new;
    wl_signal_add(&server->backend->events.new_input, &server->new_input);
    server->request_set_cursor.notify = aurora_seat_request_cursor;
    wl_signal_add(&server->seat->events.request_set_cursor,
        &server->request_set_cursor);
    server->request_set_selection.notify = aurora_seat_request_selection;
    wl_signal_add(&server->seat->events.request_set_selection,
        &server->request_set_selection);

    wlr_log(WLR_INFO, "Aurora server initialized successfully");
    return true;
}

bool aurora_server_start(struct aurora_server *server) {
    /* Add socket for Wayland clients */
    server->socket = wl_display_add_socket_auto(server->wl_display);
    if (!server->socket) {
        wlr_log(WLR_ERROR, "Failed to create Wayland socket");
        return false;
    }

    /* Start the backend */
    if (!wlr_backend_start(server->backend)) {
        wlr_log(WLR_ERROR, "Failed to start backend");
        return false;
    }

    /* Set WAYLAND_DISPLAY for child processes */
    setenv("WAYLAND_DISPLAY", server->socket, true);

    return true;
}

void aurora_server_run(struct aurora_server *server) {
    wl_display_run(server->wl_display);
}

void aurora_server_destroy(struct aurora_server *server) {
    if (!server) return;

    wl_display_destroy_clients(server->wl_display);

    if (server->cursor_mgr) wlr_xcursor_manager_destroy(server->cursor_mgr);
    if (server->cursor) wlr_cursor_destroy(server->cursor);
    if (server->allocator) wlr_allocator_destroy(server->allocator);
    if (server->renderer) wlr_renderer_destroy(server->renderer);
    if (server->backend) wlr_backend_destroy(server->backend);
    if (server->wl_display) wl_display_destroy(server->wl_display);

    free(server);
}

/* Find the view at a given layout coordinate */
struct aurora_view *aurora_view_at(struct aurora_server *server,
        double lx, double ly,
        struct wlr_surface **surface, double *sx, double *sy) {
    struct wlr_scene_node *node =
        wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);
    if (!node || node->type != WLR_SCENE_NODE_BUFFER) {
        return NULL;
    }

    struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
    struct wlr_scene_surface *scene_surface =
        wlr_scene_surface_try_from_buffer(scene_buffer);
    if (!scene_surface) {
        return NULL;
    }
    *surface = scene_surface->surface;

    /* Walk up the scene tree to find the view's tree */
    struct wlr_scene_tree *tree = node->parent;
    while (tree && !tree->node.data) {
        tree = tree->node.parent;
    }
    return tree ? tree->node.data : NULL;
}

/* Focus a view (bring to front and set keyboard focus) */
void aurora_focus_view(struct aurora_view *view, struct wlr_surface *surface) {
    if (!view) return;

    struct aurora_server *server = view->server;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;

    if (prev_surface == surface) {
        return; /* Already focused */
    }

    if (prev_surface) {
        /* Deactivate previous toplevel */
        struct wlr_xdg_toplevel *prev_toplevel =
            wlr_xdg_toplevel_try_from_wlr_surface(prev_surface);
        if (prev_toplevel) {
            wlr_xdg_toplevel_set_activated(prev_toplevel, false);
        }
    }

    /* Move view to front */
    wlr_scene_node_raise_to_top(&view->scene_tree->node);
    wl_list_remove(&view->link);
    wl_list_insert(&server->views, &view->link);

    /* Activate the new toplevel */
    wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);

    /* Set keyboard focus */
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    if (keyboard) {
        wlr_seat_keyboard_notify_enter(seat, view->xdg_toplevel->base->surface,
            keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
    }
}

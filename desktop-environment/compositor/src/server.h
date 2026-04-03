/*
 * Aurora Compositor - Server Header
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AURORA_SERVER_H
#define AURORA_SERVER_H

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/util/log.h>

#include "config.h"

/* Forward declarations */
struct aurora_view;
struct aurora_output;

/* Cursor mode for interactive operations */
enum aurora_cursor_mode {
    AURORA_CURSOR_PASSTHROUGH,
    AURORA_CURSOR_MOVE,
    AURORA_CURSOR_RESIZE,
};

/* Main server structure */
struct aurora_server {
    struct wl_display *wl_display;
    struct wl_event_loop *event_loop;
    const char *socket;

    /* Backend & rendering */
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_allocator *allocator;

    /* Scene graph */
    struct wlr_scene *scene;
    struct wlr_scene_output_layout *scene_layout;

    /* Shell protocols */
    struct wlr_xdg_shell *xdg_shell;
    struct wl_listener new_xdg_toplevel;
    struct wl_listener new_xdg_popup;

    struct wlr_layer_shell_v1 *layer_shell;
    struct wl_listener new_layer_surface;

    /* Decoration */
    struct wlr_xdg_decoration_manager_v1 *xdg_decoration_mgr;
    struct wl_listener new_xdg_decoration;
    struct wlr_server_decoration_manager *server_decoration_mgr;

    /* Output management */
    struct wlr_output_layout *output_layout;
    struct wl_list outputs;
    struct wl_listener new_output;

    /* Input / Seat */
    struct wlr_seat *seat;
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *cursor_mgr;
    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;
    struct wl_listener new_input;
    struct wl_listener request_set_cursor;
    struct wl_listener request_set_selection;
    struct wl_list keyboards;

    /* Views (windows) */
    struct wl_list views;

    /* Interactive state */
    enum aurora_cursor_mode cursor_mode;
    struct aurora_view *grabbed_view;
    double grab_x, grab_y;
    struct wlr_box grab_geobox;
    uint32_t resize_edges;

    /* Configuration */
    struct aurora_config *config;

    /* Animation timer */
    struct wl_event_source *animation_timer;
};

/* Output (monitor) */
struct aurora_output {
    struct wl_list link;
    struct aurora_server *server;
    struct wlr_output *wlr_output;
    struct wlr_scene_output *scene_output;
    struct wl_listener frame;
    struct wl_listener request_state;
    struct wl_listener destroy;
};

/* View (window) */
struct aurora_view {
    struct wl_list link;
    struct aurora_server *server;
    struct wlr_xdg_toplevel *xdg_toplevel;
    struct wlr_scene_tree *scene_tree;

    /* Animation state */
    float opacity;
    float target_opacity;
    bool animating;

    /* Listeners */
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener commit;
    struct wl_listener destroy;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
    struct wl_listener request_minimize;
};

/* Keyboard */
struct aurora_keyboard {
    struct wl_list link;
    struct aurora_server *server;
    struct wlr_keyboard *wlr_keyboard;
    struct wl_listener modifiers;
    struct wl_listener key;
    struct wl_listener destroy;
};

/* Server lifecycle */
struct aurora_server *aurora_server_create(struct aurora_config *config);
bool aurora_server_init(struct aurora_server *server);
bool aurora_server_start(struct aurora_server *server);
void aurora_server_run(struct aurora_server *server);
void aurora_server_destroy(struct aurora_server *server);

/* View helpers */
struct aurora_view *aurora_view_at(struct aurora_server *server,
    double lx, double ly,
    struct wlr_surface **surface, double *sx, double *sy);
void aurora_focus_view(struct aurora_view *view, struct wlr_surface *surface);

#endif /* AURORA_SERVER_H */

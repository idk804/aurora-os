/*
 * Aurora Compositor - Input Handling
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdlib.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <xkbcommon/xkbcommon.h>

#include "server.h"

/* Keyboard key handler */
static bool handle_keybinding(struct aurora_server *server, xkb_keysym_t sym,
        uint32_t modifiers) {
    /* Super + Return: open terminal */
    if ((modifiers & WLR_MODIFIER_LOGO) && sym == XKB_KEY_Return) {
        if (server->config->terminal_cmd) {
            if (fork() == 0) {
                execl("/bin/sh", "/bin/sh", "-c",
                    server->config->terminal_cmd, NULL);
                _exit(1);
            }
        }
        return true;
    }

    /* Super + D: open launcher */
    if ((modifiers & WLR_MODIFIER_LOGO) && sym == XKB_KEY_d) {
        if (server->config->launcher_cmd) {
            if (fork() == 0) {
                execl("/bin/sh", "/bin/sh", "-c",
                    server->config->launcher_cmd, NULL);
                _exit(1);
            }
        }
        return true;
    }

    /* Super + Q: close focused window */
    if ((modifiers & WLR_MODIFIER_LOGO) && sym == XKB_KEY_q) {
        struct aurora_view *view;
        if (!wl_list_empty(&server->views)) {
            view = wl_container_of(server->views.next, view, link);
            wlr_xdg_toplevel_send_close(view->xdg_toplevel);
        }
        return true;
    }

    /* Super + L: lock screen */
    if ((modifiers & WLR_MODIFIER_LOGO) && sym == XKB_KEY_l) {
        if (server->config->lock_cmd) {
            if (fork() == 0) {
                execl("/bin/sh", "/bin/sh", "-c",
                    server->config->lock_cmd, NULL);
                _exit(1);
            }
        }
        return true;
    }

    /* Super + Shift + E: exit compositor */
    if ((modifiers & (WLR_MODIFIER_LOGO | WLR_MODIFIER_SHIFT))
            == (WLR_MODIFIER_LOGO | WLR_MODIFIER_SHIFT)
            && sym == XKB_KEY_E) {
        wl_display_terminate(server->wl_display);
        return true;
    }

    /* Super + Tab: cycle windows */
    if ((modifiers & WLR_MODIFIER_LOGO) && sym == XKB_KEY_Tab) {
        if (wl_list_length(&server->views) >= 2) {
            struct aurora_view *next_view = wl_container_of(
                server->views.prev, next_view, link);
            aurora_focus_view(next_view, next_view->xdg_toplevel->base->surface);
        }
        return true;
    }

    return false;
}

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
    struct aurora_keyboard *keyboard =
        wl_container_of(listener, keyboard, key);
    struct aurora_server *server = keyboard->server;
    struct wlr_keyboard_key_event *event = data;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(
        keyboard->wlr_keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);

    if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        for (int i = 0; i < nsyms; i++) {
            handled = handle_keybinding(server, syms[i], modifiers);
        }
    }

    if (!handled) {
        wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);
        wlr_seat_keyboard_notify_key(server->seat,
            event->time_msec, event->keycode, event->state);
    }
}

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data) {
    struct aurora_keyboard *keyboard =
        wl_container_of(listener, keyboard, modifiers);
    wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
        &keyboard->wlr_keyboard->modifiers);
}

static void keyboard_handle_destroy(struct wl_listener *listener, void *data) {
    struct aurora_keyboard *keyboard =
        wl_container_of(listener, keyboard, destroy);
    wl_list_remove(&keyboard->modifiers.link);
    wl_list_remove(&keyboard->key.link);
    wl_list_remove(&keyboard->destroy.link);
    wl_list_remove(&keyboard->link);
    free(keyboard);
}

static void new_keyboard(struct aurora_server *server,
        struct wlr_input_device *device) {
    struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

    struct aurora_keyboard *keyboard = calloc(1, sizeof(*keyboard));
    keyboard->server = server;
    keyboard->wlr_keyboard = wlr_keyboard;

    /* Set up XKB keymap */
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_keymap_new_from_names(
        context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);

    wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
    keyboard->destroy.notify = keyboard_handle_destroy;
    wl_signal_add(&device->events.destroy, &keyboard->destroy);

    wlr_seat_set_keyboard(server->seat, wlr_keyboard);
    wl_list_insert(&server->keyboards, &keyboard->link);
}

static void new_pointer(struct aurora_server *server,
        struct wlr_input_device *device) {
    wlr_cursor_attach_input_device(server->cursor, device);
}

void aurora_input_new(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        new_keyboard(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        new_pointer(server, device);
        break;
    default:
        break;
    }

    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat, caps);
}

/* Cursor event handlers */
static void process_cursor_motion(struct aurora_server *server, uint32_t time) {
    double sx, sy;
    struct wlr_surface *surface = NULL;
    struct aurora_view *view = aurora_view_at(server,
        server->cursor->x, server->cursor->y, &surface, &sx, &sy);

    if (!view) {
        wlr_cursor_set_xcursor(server->cursor, server->cursor_mgr, "default");
    }
    if (surface) {
        wlr_seat_pointer_notify_enter(server->seat, surface, sx, sy);
        wlr_seat_pointer_notify_motion(server->seat, time, sx, sy);
    } else {
        wlr_seat_pointer_clear_focus(server->seat);
    }
}

void aurora_cursor_motion(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, cursor_motion);
    struct wlr_pointer_motion_event *event = data;
    wlr_cursor_move(server->cursor, &event->pointer->base,
        event->delta_x, event->delta_y);
    process_cursor_motion(server, event->time_msec);
}

void aurora_cursor_motion_absolute(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, cursor_motion_absolute);
    struct wlr_pointer_motion_absolute_event *event = data;
    wlr_cursor_warp_absolute(server->cursor, &event->pointer->base,
        event->x, event->y);
    process_cursor_motion(server, event->time_msec);
}

void aurora_cursor_button(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, cursor_button);
    struct wlr_pointer_button_event *event = data;
    wlr_seat_pointer_notify_button(server->seat,
        event->time_msec, event->button, event->state);

    if (event->state == WL_POINTER_BUTTON_STATE_PRESSED) {
        double sx, sy;
        struct wlr_surface *surface;
        struct aurora_view *view = aurora_view_at(server,
            server->cursor->x, server->cursor->y, &surface, &sx, &sy);
        if (view) {
            aurora_focus_view(view, surface);
        }
    }
}

void aurora_cursor_axis(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, cursor_axis);
    struct wlr_pointer_axis_event *event = data;
    wlr_seat_pointer_notify_axis(server->seat,
        event->time_msec, event->orientation, event->delta,
        event->delta_discrete, event->source, event->relative_direction);
}

void aurora_cursor_frame(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, cursor_frame);
    wlr_seat_pointer_notify_frame(server->seat);
}

void aurora_seat_request_cursor(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, request_set_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = data;
    struct wlr_seat_client *focused = server->seat->pointer_state.focused_client;
    if (focused == event->seat_client) {
        wlr_cursor_set_surface(server->cursor, event->surface,
            event->hotspot_x, event->hotspot_y);
    }
}

void aurora_seat_request_selection(struct wl_listener *listener, void *data) {
    struct aurora_server *server =
        wl_container_of(listener, server, request_set_selection);
    struct wlr_seat_request_set_selection_event *event = data;
    wlr_seat_set_selection(server->seat, event->source, event->serial);
}

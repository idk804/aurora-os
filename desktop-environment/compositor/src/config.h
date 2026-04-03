/*
 * Aurora Compositor - Configuration Header
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef AURORA_CONFIG_H
#define AURORA_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

struct aurora_config {
    /* General */
    bool natural_scroll;
    float mouse_sensitivity;

    /* Appearance */
    int border_width;
    uint32_t border_color_focused;    /* ARGB */
    uint32_t border_color_unfocused;  /* ARGB */
    float corner_radius;
    float opacity_unfocused;

    /* Animations */
    bool animations_enabled;
    int animation_duration_ms;
    float animation_curve;  /* bezier control point */

    /* Gaps */
    int inner_gap;
    int outer_gap;

    /* Wallpaper */
    char *wallpaper_path;

    /* Keybindings */
    char *terminal_cmd;
    char *launcher_cmd;
    char *lock_cmd;
};

struct aurora_config *aurora_config_create(void);
bool aurora_config_load(struct aurora_config *config, const char *path);
void aurora_config_destroy(struct aurora_config *config);

#endif /* AURORA_CONFIG_H */

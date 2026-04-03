/*
 * Aurora Compositor - Configuration Implementation
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

/* Aurora color palette (ARGB) */
#define AURORA_GREEN       0xFF00FF87
#define AURORA_PURPLE      0xFFB388FF
#define AURORA_CYAN        0xFF00E5FF
#define AURORA_BG_DARK     0xFF0A0A1A
#define AURORA_BG_MEDIUM   0xFF0D1117
#define AURORA_TEXT         0xFFE0E0E0

struct aurora_config *aurora_config_create(void) {
    struct aurora_config *config = calloc(1, sizeof(*config));
    if (!config) return NULL;

    /* Defaults - Aurora theme */
    config->natural_scroll = true;
    config->mouse_sensitivity = 1.0f;

    config->border_width = 2;
    config->border_color_focused = AURORA_GREEN;
    config->border_color_unfocused = 0xFF333344;
    config->corner_radius = 10.0f;
    config->opacity_unfocused = 0.95f;

    config->animations_enabled = true;
    config->animation_duration_ms = 250;
    config->animation_curve = 0.25f;

    config->inner_gap = 8;
    config->outer_gap = 12;

    config->wallpaper_path = strdup("/usr/share/backgrounds/aurora/default.png");
    config->terminal_cmd = strdup("foot");
    config->launcher_cmd = strdup("aurora-launcher");
    config->lock_cmd = strdup("aurora-lock");

    return config;
}

/* Simple key=value config parser */
bool aurora_config_load(struct aurora_config *config, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        /* Skip comments and empty lines */
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0')
            continue;

        char key[256], value[768];
        if (sscanf(line, "%255[^=]=%767[^\n]", key, value) != 2)
            continue;

        /* Trim whitespace */
        char *k = key;
        while (*k == ' ') k++;
        char *v = value;
        while (*v == ' ') v++;

        if (strcmp(k, "natural_scroll") == 0)
            config->natural_scroll = (strcmp(v, "true") == 0);
        else if (strcmp(k, "mouse_sensitivity") == 0)
            config->mouse_sensitivity = atof(v);
        else if (strcmp(k, "border_width") == 0)
            config->border_width = atoi(v);
        else if (strcmp(k, "corner_radius") == 0)
            config->corner_radius = atof(v);
        else if (strcmp(k, "animations") == 0)
            config->animations_enabled = (strcmp(v, "true") == 0);
        else if (strcmp(k, "animation_duration") == 0)
            config->animation_duration_ms = atoi(v);
        else if (strcmp(k, "inner_gap") == 0)
            config->inner_gap = atoi(v);
        else if (strcmp(k, "outer_gap") == 0)
            config->outer_gap = atoi(v);
        else if (strcmp(k, "wallpaper") == 0) {
            free(config->wallpaper_path);
            config->wallpaper_path = strdup(v);
        }
        else if (strcmp(k, "terminal") == 0) {
            free(config->terminal_cmd);
            config->terminal_cmd = strdup(v);
        }
        else if (strcmp(k, "launcher") == 0) {
            free(config->launcher_cmd);
            config->launcher_cmd = strdup(v);
        }
    }

    fclose(f);
    return true;
}

void aurora_config_destroy(struct aurora_config *config) {
    if (!config) return;
    free(config->wallpaper_path);
    free(config->terminal_cmd);
    free(config->launcher_cmd);
    free(config->lock_cmd);
    free(config);
}

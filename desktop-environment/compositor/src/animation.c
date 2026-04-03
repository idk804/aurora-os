/*
 * Aurora Compositor - Animation System
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <math.h>
#include "server.h"

/* Ease-out cubic for smooth Aurora-style animations */
static float ease_out_cubic(float t) {
    return 1.0f - powf(1.0f - t, 3.0f);
}

/* Ease-in-out for gentle transitions */
static float ease_in_out_quad(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - powf(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

/* Interpolate between values */
float aurora_lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

/* Color interpolation for aurora effects (ARGB) */
uint32_t aurora_color_lerp(uint32_t color_a, uint32_t color_b, float t) {
    uint8_t a_a = (color_a >> 24) & 0xFF;
    uint8_t r_a = (color_a >> 16) & 0xFF;
    uint8_t g_a = (color_a >> 8) & 0xFF;
    uint8_t b_a = color_a & 0xFF;

    uint8_t a_b = (color_b >> 24) & 0xFF;
    uint8_t r_b = (color_b >> 16) & 0xFF;
    uint8_t g_b = (color_b >> 8) & 0xFF;
    uint8_t b_b = color_b & 0xFF;

    uint8_t a = (uint8_t)aurora_lerp(a_a, a_b, t);
    uint8_t r = (uint8_t)aurora_lerp(r_a, r_b, t);
    uint8_t g = (uint8_t)aurora_lerp(g_a, g_b, t);
    uint8_t b = (uint8_t)aurora_lerp(b_a, b_b, t);

    return (a << 24) | (r << 16) | (g << 8) | b;
}

/* Aurora glow cycle colors */
static const uint32_t aurora_colors[] = {
    0xFF00FF87,  /* Green */
    0xFF00E5FF,  /* Cyan */
    0xFFB388FF,  /* Purple */
    0xFF7C4DFF,  /* Deep purple */
    0xFF18FFFF,  /* Bright cyan */
    0xFF00FF87,  /* Back to green */
};
#define AURORA_COLOR_COUNT (sizeof(aurora_colors) / sizeof(aurora_colors[0]))

/* Get the current aurora accent color based on time */
uint32_t aurora_get_accent_color(float time_seconds) {
    float cycle_duration = 30.0f; /* Full color cycle in seconds */
    float t = fmodf(time_seconds, cycle_duration) / cycle_duration;
    float index_f = t * (AURORA_COLOR_COUNT - 1);
    int index = (int)index_f;
    float frac = index_f - index;

    if (index >= (int)AURORA_COLOR_COUNT - 1) {
        return aurora_colors[AURORA_COLOR_COUNT - 1];
    }

    return aurora_color_lerp(aurora_colors[index], aurora_colors[index + 1],
        ease_in_out_quad(frac));
}

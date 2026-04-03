/*
 * Aurora Compositor - Main Entry Point
 * A wlroots-based Wayland compositor for Aurora OS
 *
 * Copyright (C) 2026 Aurora OS Project
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "server.h"
#include "config.h"

static void print_usage(const char *prog) {
    fprintf(stderr,
        "Aurora Compositor v0.1.0\n"
        "Usage: %s [options]\n"
        "\n"
        "Options:\n"
        "  -c, --config <path>   Configuration file path\n"
        "  -s, --startup <cmd>   Startup command to execute\n"
        "  -d, --debug           Enable debug logging\n"
        "  -h, --help            Show this help\n"
        "\n", prog);
}

int main(int argc, char *argv[]) {
    char *config_path = NULL;
    char *startup_cmd = NULL;
    bool debug = false;

    static struct option long_options[] = {
        {"config",  required_argument, 0, 'c'},
        {"startup", required_argument, 0, 's'},
        {"debug",   no_argument,       0, 'd'},
        {"help",    no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "c:s:dh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'c':
                config_path = optarg;
                break;
            case 's':
                startup_cmd = optarg;
                break;
            case 'd':
                debug = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /* Set log level */
    if (debug) {
        wlr_log_init(WLR_DEBUG, NULL);
    } else {
        wlr_log_init(WLR_INFO, NULL);
    }

    wlr_log(WLR_INFO, "🌌 Aurora Compositor starting...");

    /* Load configuration */
    struct aurora_config *config = aurora_config_create();
    if (config_path) {
        if (!aurora_config_load(config, config_path)) {
            wlr_log(WLR_ERROR, "Failed to load config: %s", config_path);
        }
    } else {
        /* Try default config paths */
        const char *home = getenv("HOME");
        if (home) {
            char default_path[512];
            snprintf(default_path, sizeof(default_path),
                "%s/.config/aurora/compositor.conf", home);
            aurora_config_load(config, default_path);
        }
    }

    /* Create and initialize server */
    struct aurora_server *server = aurora_server_create(config);
    if (!server) {
        wlr_log(WLR_ERROR, "Failed to create Aurora server");
        aurora_config_destroy(config);
        return 1;
    }

    if (!aurora_server_init(server)) {
        wlr_log(WLR_ERROR, "Failed to initialize Aurora server");
        aurora_server_destroy(server);
        aurora_config_destroy(config);
        return 1;
    }

    /* Start the server */
    if (!aurora_server_start(server)) {
        wlr_log(WLR_ERROR, "Failed to start Aurora server");
        aurora_server_destroy(server);
        aurora_config_destroy(config);
        return 1;
    }

    /* Run startup command if provided */
    if (startup_cmd) {
        wlr_log(WLR_INFO, "Running startup command: %s", startup_cmd);
        if (fork() == 0) {
            execl("/bin/sh", "/bin/sh", "-c", startup_cmd, NULL);
            _exit(1);
        }
    }

    wlr_log(WLR_INFO, "🌌 Aurora Compositor running on WAYLAND_DISPLAY=%s",
        server->socket);

    /* Run the event loop */
    aurora_server_run(server);

    /* Cleanup */
    wlr_log(WLR_INFO, "🌌 Aurora Compositor shutting down...");
    aurora_server_destroy(server);
    aurora_config_destroy(config);

    return 0;
}

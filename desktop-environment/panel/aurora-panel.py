#!/usr/bin/env python3
"""
Aurora Panel - Top Bar for Aurora Desktop Environment
A Wayland layer-shell panel with clock, system tray, and quick settings.

Uses gtk4-layer-shell for Wayland layer shell integration.
"""

import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gdk', '4.0')

from gi.repository import Gtk, Gdk, GLib, Gio, Pango
import subprocess
import datetime
import json
import os
import signal

# Try to import layer shell (optional, falls back gracefully)
try:
    gi.require_version('Gtk4LayerShell', '1.0')
    from gi.repository import Gtk4LayerShell as LayerShell
    HAS_LAYER_SHELL = True
except (ValueError, ImportError):
    HAS_LAYER_SHELL = False
    print("Warning: gtk4-layer-shell not available, running in normal window mode")


class AuroraPanel(Gtk.Application):
    """Main Aurora Panel application."""

    def __init__(self):
        super().__init__(application_id="org.aurora.panel")
        self.window = None
        self.clock_label = None
        self.date_label = None
        self.battery_label = None
        self.volume_icon = None
        self.wifi_icon = None

    def do_activate(self):
        self.window = Gtk.ApplicationWindow(application=self)
        self.window.set_title("Aurora Panel")
        self.window.set_default_size(-1, 36)

        # Configure as layer shell surface (top bar)
        if HAS_LAYER_SHELL:
            LayerShell.init_for_window(self.window)
            LayerShell.set_layer(self.window, LayerShell.Layer.TOP)
            LayerShell.set_anchor(self.window, LayerShell.Edge.TOP, True)
            LayerShell.set_anchor(self.window, LayerShell.Edge.LEFT, True)
            LayerShell.set_anchor(self.window, LayerShell.Edge.RIGHT, True)
            LayerShell.set_exclusive_zone(self.window, 36)
            LayerShell.set_margin(self.window, LayerShell.Edge.TOP, 0)
            LayerShell.set_namespace(self.window, "aurora-panel")

        # Apply CSS
        self._apply_css()

        # Build panel layout
        main_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        main_box.add_css_class("aurora-panel")

        # Left section: Activities button + workspaces
        left_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        left_box.set_margin_start(12)
        left_box.set_hexpand(True)
        left_box.set_halign(Gtk.Align.START)

        activities_btn = Gtk.Button(label="⟡ Aurora")
        activities_btn.add_css_class("activities-button")
        activities_btn.connect("clicked", self._on_activities_clicked)
        left_box.append(activities_btn)

        # Workspace indicators
        for i in range(1, 5):
            ws_btn = Gtk.Button(label=str(i))
            ws_btn.add_css_class("workspace-button")
            if i == 1:
                ws_btn.add_css_class("workspace-active")
            left_box.append(ws_btn)

        main_box.append(left_box)

        # Center section: Clock
        center_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        center_box.set_halign(Gtk.Align.CENTER)
        center_box.set_valign(Gtk.Align.CENTER)

        self.clock_label = Gtk.Label()
        self.clock_label.add_css_class("clock-label")
        center_box.append(self.clock_label)

        clock_button = Gtk.Button()
        clock_button.set_child(center_box)
        clock_button.add_css_class("clock-button")
        clock_button.connect("clicked", self._on_clock_clicked)
        main_box.append(clock_button)

        # Right section: System tray
        right_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=12)
        right_box.set_margin_end(12)
        right_box.set_hexpand(True)
        right_box.set_halign(Gtk.Align.END)

        # System indicators
        self.wifi_icon = Gtk.Label(label="󰤨")  # Nerd font wifi icon
        self.wifi_icon.add_css_class("tray-icon")
        right_box.append(self.wifi_icon)

        self.volume_icon = Gtk.Label(label="󰕾")  # Nerd font volume icon
        self.volume_icon.add_css_class("tray-icon")
        right_box.append(self.volume_icon)

        self.battery_label = Gtk.Label(label="󰁹 100%")  # Nerd font battery
        self.battery_label.add_css_class("tray-icon")
        right_box.append(self.battery_label)

        # Power button
        power_btn = Gtk.Button(label="⏻")
        power_btn.add_css_class("power-button")
        power_btn.connect("clicked", self._on_power_clicked)
        right_box.append(power_btn)

        main_box.append(right_box)

        self.window.set_child(main_box)
        self.window.present()

        # Start clock update
        self._update_clock()
        GLib.timeout_add_seconds(1, self._update_clock)

        # Update system info periodically
        self._update_system_info()
        GLib.timeout_add_seconds(30, self._update_system_info)

    def _apply_css(self):
        """Apply Aurora panel CSS styling."""
        css = b"""
        .aurora-panel {
            background-color: rgba(10, 10, 26, 0.85);
            border-bottom: 1px solid rgba(0, 255, 135, 0.2);
            min-height: 36px;
            padding: 0 4px;
        }

        .activities-button {
            background: none;
            border: none;
            color: #00ff87;
            font-weight: bold;
            font-size: 13px;
            padding: 4px 12px;
            border-radius: 6px;
        }
        .activities-button:hover {
            background-color: rgba(0, 255, 135, 0.15);
        }

        .workspace-button {
            background: none;
            border: none;
            color: rgba(224, 224, 224, 0.4);
            font-size: 11px;
            min-width: 24px;
            min-height: 24px;
            padding: 2px 6px;
            border-radius: 4px;
        }
        .workspace-button:hover {
            color: #e0e0e0;
            background-color: rgba(255, 255, 255, 0.08);
        }
        .workspace-active {
            color: #00ff87;
            background-color: rgba(0, 255, 135, 0.15);
        }

        .clock-button {
            background: none;
            border: none;
            padding: 4px 16px;
            border-radius: 6px;
        }
        .clock-button:hover {
            background-color: rgba(255, 255, 255, 0.08);
        }

        .clock-label {
            color: #e0e0e0;
            font-size: 13px;
            font-weight: 500;
        }

        .tray-icon {
            color: #e0e0e0;
            font-size: 14px;
        }

        .power-button {
            background: none;
            border: none;
            color: #e0e0e0;
            font-size: 14px;
            min-width: 28px;
            min-height: 28px;
            border-radius: 50%;
            padding: 2px;
        }
        .power-button:hover {
            background-color: rgba(255, 85, 85, 0.3);
            color: #ff5555;
        }
        """
        provider = Gtk.CssProvider()
        provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_display(
            Gdk.Display.get_default(),
            provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

    def _update_clock(self):
        """Update the clock display."""
        now = datetime.datetime.now()
        time_str = now.strftime("%a %b %-d   %H:%M")
        if self.clock_label:
            self.clock_label.set_text(time_str)
        return True  # Keep the timeout running

    def _update_system_info(self):
        """Update battery and other system indicators."""
        # Battery
        try:
            with open("/sys/class/power_supply/BAT0/capacity", "r") as f:
                capacity = f.read().strip()
            with open("/sys/class/power_supply/BAT0/status", "r") as f:
                status = f.read().strip()
            icon = "󰂄" if status == "Charging" else "󰁹"
            if self.battery_label:
                self.battery_label.set_text(f"{icon} {capacity}%")
        except FileNotFoundError:
            if self.battery_label:
                self.battery_label.set_text("󰚥 AC")

        return True

    def _on_activities_clicked(self, button):
        """Open the app launcher."""
        try:
            subprocess.Popen(["aurora-launcher"])
        except FileNotFoundError:
            print("aurora-launcher not found")

    def _on_clock_clicked(self, button):
        """Show calendar/notifications popup."""
        # TODO: Implement calendar popover
        pass

    def _on_power_clicked(self, button):
        """Show power menu."""
        # TODO: Implement power menu popover
        pass


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app = AuroraPanel()
    app.run()


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""
Aurora Widgets - Desktop Widgets for Aurora OS
Clock, weather, system monitor, and media player widgets.

Uses gtk4-layer-shell to render on the desktop background layer.
"""

import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gdk', '4.0')

from gi.repository import Gtk, Gdk, GLib
import datetime
import os
import subprocess
import json
import math
import signal

try:
    gi.require_version('Gtk4LayerShell', '1.0')
    from gi.repository import Gtk4LayerShell as LayerShell
    HAS_LAYER_SHELL = True
except (ValueError, ImportError):
    HAS_LAYER_SHELL = False


# ─── Aurora Colors ───
COLORS = {
    "green": "#00ff87",
    "purple": "#b388ff",
    "cyan": "#00e5ff",
    "bg_dark": "#0a0a1a",
    "bg_surface": "rgba(22, 27, 34, 0.75)",
    "text": "#e0e0e0",
    "text_dim": "#a0a0b0",
    "border": "rgba(0, 255, 135, 0.15)",
}


class ClockWidget(Gtk.Box):
    """Large clock with date, aurora gradient text."""

    def __init__(self):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        self.add_css_class("widget-card")
        self.set_size_request(320, 160)

        self.time_label = Gtk.Label()
        self.time_label.add_css_class("clock-time")
        self.append(self.time_label)

        self.date_label = Gtk.Label()
        self.date_label.add_css_class("clock-date")
        self.append(self.date_label)

        self.seconds_label = Gtk.Label()
        self.seconds_label.add_css_class("clock-seconds")
        self.append(self.seconds_label)

        self._update()
        GLib.timeout_add(1000, self._update)

    def _update(self):
        now = datetime.datetime.now()
        self.time_label.set_text(now.strftime("%H:%M"))
        self.date_label.set_text(now.strftime("%A, %B %-d"))
        self.seconds_label.set_text(now.strftime(":%S"))
        return True


class WeatherWidget(Gtk.Box):
    """Weather display with animated aurora background."""

    def __init__(self):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        self.add_css_class("widget-card")
        self.set_size_request(280, 140)

        # Header
        header = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        header_label = Gtk.Label(label="Weather")
        header_label.add_css_class("widget-title")
        header.append(header_label)
        self.append(header)

        # Weather content
        content = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=16)

        # Temperature and icon
        self.temp_label = Gtk.Label(label="--°")
        self.temp_label.add_css_class("weather-temp")
        content.append(self.temp_label)

        # Details
        details = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        self.condition_label = Gtk.Label(label="Loading...")
        self.condition_label.add_css_class("weather-condition")
        details.append(self.condition_label)

        self.details_label = Gtk.Label(label="")
        self.details_label.add_css_class("weather-details")
        details.append(self.details_label)

        content.append(details)
        self.append(content)

        # Fetch weather
        GLib.idle_add(self._fetch_weather)
        GLib.timeout_add_seconds(1800, self._fetch_weather)  # Every 30 min

    def _fetch_weather(self):
        """Fetch weather from wttr.in."""
        try:
            result = subprocess.run(
                ["curl", "-s", "wttr.in/?format=%t|%C|%h|%w"],
                capture_output=True, text=True, timeout=10
            )
            if result.returncode == 0 and result.stdout:
                parts = result.stdout.strip().split("|")
                if len(parts) >= 2:
                    self.temp_label.set_text(parts[0])
                    self.condition_label.set_text(parts[1])
                    if len(parts) >= 4:
                        self.details_label.set_text(
                            f"Humidity: {parts[2]}  Wind: {parts[3]}")
        except Exception:
            self.condition_label.set_text("Unavailable")
        return True


class SystemMonitorWidget(Gtk.Box):
    """CPU, RAM, and disk usage with aurora-colored bars."""

    def __init__(self):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        self.add_css_class("widget-card")
        self.set_size_request(280, 180)

        title = Gtk.Label(label="System")
        title.add_css_class("widget-title")
        title.set_halign(Gtk.Align.START)
        self.append(title)

        # CPU
        self.cpu_bar = self._create_stat_row("CPU", "cpu")
        self.append(self.cpu_bar["box"])

        # RAM
        self.ram_bar = self._create_stat_row("RAM", "ram")
        self.append(self.ram_bar["box"])

        # Disk
        self.disk_bar = self._create_stat_row("Disk", "disk")
        self.append(self.disk_bar["box"])

        self._update()
        GLib.timeout_add_seconds(3, self._update)

    def _create_stat_row(self, label, css_class):
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=2)

        header = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=0)
        name = Gtk.Label(label=label)
        name.add_css_class("stat-label")
        name.set_halign(Gtk.Align.START)
        name.set_hexpand(True)
        header.append(name)

        value = Gtk.Label(label="0%")
        value.add_css_class("stat-value")
        header.append(value)
        box.append(header)

        bar = Gtk.ProgressBar()
        bar.add_css_class(f"stat-bar-{css_class}")
        bar.set_fraction(0)
        box.append(bar)

        return {"box": box, "label": value, "bar": bar}

    def _update(self):
        # CPU usage
        try:
            load = os.getloadavg()[0]
            cpu_count = os.cpu_count() or 1
            cpu_pct = min(load / cpu_count, 1.0)
            self.cpu_bar["bar"].set_fraction(cpu_pct)
            self.cpu_bar["label"].set_text(f"{int(cpu_pct * 100)}%")
        except Exception:
            pass

        # RAM usage
        try:
            with open("/proc/meminfo") as f:
                lines = f.readlines()
            mem = {}
            for line in lines:
                parts = line.split()
                if len(parts) >= 2:
                    mem[parts[0].rstrip(":")] = int(parts[1])
            total = mem.get("MemTotal", 1)
            available = mem.get("MemAvailable", 0)
            used_pct = (total - available) / total
            self.ram_bar["bar"].set_fraction(used_pct)
            used_gb = (total - available) / 1048576
            total_gb = total / 1048576
            self.ram_bar["label"].set_text(
                f"{used_gb:.1f}/{total_gb:.1f} GB")
        except Exception:
            pass

        # Disk usage
        try:
            st = os.statvfs("/")
            total = st.f_blocks * st.f_frsize
            free = st.f_bfree * st.f_frsize
            used_pct = (total - free) / total
            self.disk_bar["bar"].set_fraction(used_pct)
            used_gb = (total - free) / (1024**3)
            total_gb = total / (1024**3)
            self.disk_bar["label"].set_text(
                f"{used_gb:.0f}/{total_gb:.0f} GB")
        except Exception:
            pass

        return True


class MediaWidget(Gtk.Box):
    """Media player controls (MPRIS integration)."""

    def __init__(self):
        super().__init__(orientation=Gtk.Orientation.VERTICAL, spacing=8)
        self.add_css_class("widget-card")
        self.set_size_request(320, 120)

        title = Gtk.Label(label="Now Playing")
        title.add_css_class("widget-title")
        title.set_halign(Gtk.Align.START)
        self.append(title)

        # Track info
        self.track_label = Gtk.Label(label="No media playing")
        self.track_label.add_css_class("media-track")
        self.track_label.set_ellipsize(3)
        self.append(self.track_label)

        self.artist_label = Gtk.Label(label="")
        self.artist_label.add_css_class("media-artist")
        self.artist_label.set_ellipsize(3)
        self.append(self.artist_label)

        # Controls
        controls = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=16)
        controls.set_halign(Gtk.Align.CENTER)

        for icon, action in [("⏮", "Previous"), ("⏯", "PlayPause"), ("⏭", "Next")]:
            btn = Gtk.Button(label=icon)
            btn.add_css_class("media-control")
            btn.connect("clicked", self._on_control, action)
            controls.append(btn)

        self.append(controls)

        self._update_media()
        GLib.timeout_add_seconds(5, self._update_media)

    def _on_control(self, button, action):
        try:
            subprocess.Popen(
                ["dbus-send", "--type=method_call", "--dest=org.mpris.MediaPlayer2.playerctld",
                 "/org/mpris/MediaPlayer2",
                 f"org.mpris.MediaPlayer2.Player.{action}"],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
            )
        except Exception:
            pass

    def _update_media(self):
        try:
            result = subprocess.run(
                ["playerctl", "metadata", "--format",
                 "{{artist}}|||{{title}}"],
                capture_output=True, text=True, timeout=2
            )
            if result.returncode == 0 and "|||" in result.stdout:
                artist, title = result.stdout.strip().split("|||", 1)
                self.track_label.set_text(title or "Unknown")
                self.artist_label.set_text(artist or "Unknown Artist")
            else:
                self.track_label.set_text("No media playing")
                self.artist_label.set_text("")
        except Exception:
            pass
        return True


class AuroraWidgets(Gtk.Application):
    """Main widgets application - renders all widgets on the desktop."""

    def __init__(self):
        super().__init__(application_id="org.aurora.widgets")

    def do_activate(self):
        window = Gtk.ApplicationWindow(application=self)
        window.set_title("Aurora Widgets")

        if HAS_LAYER_SHELL:
            LayerShell.init_for_window(window)
            LayerShell.set_layer(window, LayerShell.Layer.BOTTOM)
            LayerShell.set_anchor(window, LayerShell.Edge.RIGHT, True)
            LayerShell.set_anchor(window, LayerShell.Edge.TOP, True)
            LayerShell.set_margin(window, LayerShell.Edge.RIGHT, 20)
            LayerShell.set_margin(window, LayerShell.Edge.TOP, 52)
            LayerShell.set_namespace(window, "aurora-widgets")
        else:
            window.set_default_size(340, 700)

        self._apply_css()

        # Widget container
        container = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=16)
        container.set_margin_top(0)
        container.set_margin_bottom(20)

        container.append(ClockWidget())
        container.append(WeatherWidget())
        container.append(SystemMonitorWidget())
        container.append(MediaWidget())

        window.set_child(container)
        window.present()

    def _apply_css(self):
        css = b"""
        .widget-card {
            background-color: rgba(22, 27, 34, 0.75);
            border: 1px solid rgba(0, 255, 135, 0.15);
            border-radius: 16px;
            padding: 16px;
        }

        .widget-title {
            color: #00ff87;
            font-size: 11px;
            font-weight: bold;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        /* Clock */
        .clock-time {
            color: #e0e0e0;
            font-size: 52px;
            font-weight: 200;
            letter-spacing: -2px;
        }
        .clock-date {
            color: #a0a0b0;
            font-size: 14px;
        }
        .clock-seconds {
            color: #00ff87;
            font-size: 14px;
            font-weight: bold;
        }

        /* Weather */
        .weather-temp {
            color: #e0e0e0;
            font-size: 36px;
            font-weight: 300;
        }
        .weather-condition {
            color: #e0e0e0;
            font-size: 14px;
        }
        .weather-details {
            color: #a0a0b0;
            font-size: 11px;
        }

        /* System Monitor */
        .stat-label {
            color: #a0a0b0;
            font-size: 11px;
        }
        .stat-value {
            color: #e0e0e0;
            font-size: 11px;
            font-weight: bold;
        }

        progressbar trough {
            background-color: rgba(42, 42, 58, 0.6);
            border-radius: 999px;
            min-height: 4px;
        }
        .stat-bar-cpu progress {
            background: linear-gradient(to right, #00ff87, #00e5ff);
            border-radius: 999px;
        }
        .stat-bar-ram progress {
            background: linear-gradient(to right, #b388ff, #7c4dff);
            border-radius: 999px;
        }
        .stat-bar-disk progress {
            background: linear-gradient(to right, #00e5ff, #18ffff);
            border-radius: 999px;
        }

        /* Media */
        .media-track {
            color: #e0e0e0;
            font-size: 14px;
            font-weight: 500;
        }
        .media-artist {
            color: #a0a0b0;
            font-size: 12px;
        }
        .media-control {
            background: none;
            border: none;
            color: #e0e0e0;
            font-size: 18px;
            min-width: 36px;
            min-height: 36px;
            border-radius: 50%;
            padding: 4px;
        }
        .media-control:hover {
            background-color: rgba(0, 255, 135, 0.15);
            color: #00ff87;
        }
        """
        provider = Gtk.CssProvider()
        provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_display(
            Gdk.Display.get_default(),
            provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app = AuroraWidgets()
    app.run()


if __name__ == "__main__":
    main()

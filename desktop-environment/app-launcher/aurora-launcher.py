#!/usr/bin/env python3
"""
Aurora App Launcher
Full-screen application launcher with fuzzy search and aurora blur.

Uses gtk4-layer-shell to overlay the desktop.
"""

import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Gdk', '4.0')

from gi.repository import Gtk, Gdk, GLib, Gio
import subprocess
import os
import signal
import re
from pathlib import Path

try:
    gi.require_version('Gtk4LayerShell', '1.0')
    from gi.repository import Gtk4LayerShell as LayerShell
    HAS_LAYER_SHELL = True
except (ValueError, ImportError):
    HAS_LAYER_SHELL = False


class AppEntry:
    """Represents a .desktop application entry."""

    def __init__(self, name, icon, exec_cmd, comment="", categories=None):
        self.name = name
        self.icon = icon or "application-x-executable"
        self.exec_cmd = exec_cmd
        self.comment = comment
        self.categories = categories or []
        self.search_text = f"{name} {comment} {' '.join(self.categories)}".lower()


class AuroraLauncher(Gtk.Application):
    """Aurora full-screen application launcher."""

    def __init__(self):
        super().__init__(application_id="org.aurora.launcher")
        self.window = None
        self.search_entry = None
        self.app_grid = None
        self.apps = []
        self.filtered_apps = []

    def do_activate(self):
        self._load_applications()

        self.window = Gtk.ApplicationWindow(application=self)
        self.window.set_title("Aurora Launcher")

        if HAS_LAYER_SHELL:
            LayerShell.init_for_window(self.window)
            LayerShell.set_layer(self.window, LayerShell.Layer.OVERLAY)
            LayerShell.set_anchor(self.window, LayerShell.Edge.TOP, True)
            LayerShell.set_anchor(self.window, LayerShell.Edge.BOTTOM, True)
            LayerShell.set_anchor(self.window, LayerShell.Edge.LEFT, True)
            LayerShell.set_anchor(self.window, LayerShell.Edge.RIGHT, True)
            LayerShell.set_keyboard_mode(self.window, LayerShell.KeyboardMode.EXCLUSIVE)
            LayerShell.set_namespace(self.window, "aurora-launcher")
        else:
            self.window.set_default_size(1280, 720)

        self._apply_css()

        # Key controller for Escape to close
        key_controller = Gtk.EventControllerKey()
        key_controller.connect("key-pressed", self._on_key_pressed)
        self.window.add_controller(key_controller)

        # Main vertical layout
        main_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        main_box.add_css_class("launcher-background")
        main_box.set_valign(Gtk.Align.CENTER)
        main_box.set_halign(Gtk.Align.CENTER)

        # Content container (centered, max width)
        content = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=24)
        content.set_margin_top(60)
        content.set_margin_bottom(60)
        content.set_margin_start(80)
        content.set_margin_end(80)
        content.set_size_request(800, -1)
        content.set_halign(Gtk.Align.CENTER)

        # Search bar
        search_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=12)
        search_box.add_css_class("search-container")

        search_icon = Gtk.Label(label="🔍")
        search_icon.add_css_class("search-icon")
        search_box.append(search_icon)

        self.search_entry = Gtk.Entry()
        self.search_entry.set_placeholder_text("Search applications...")
        self.search_entry.add_css_class("search-entry")
        self.search_entry.set_hexpand(True)
        self.search_entry.connect("changed", self._on_search_changed)
        self.search_entry.connect("activate", self._on_search_activate)
        search_box.append(self.search_entry)

        content.append(search_box)

        # Category filters
        category_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=8)
        category_box.set_halign(Gtk.Align.CENTER)
        categories = ["All", "Internet", "Development", "Graphics",
                       "Media", "Office", "System", "Utilities"]
        for cat in categories:
            btn = Gtk.Button(label=cat)
            btn.add_css_class("category-button")
            if cat == "All":
                btn.add_css_class("category-active")
            btn.connect("clicked", self._on_category_clicked, cat)
            category_box.append(btn)
        content.append(category_box)

        # App grid (scrollable)
        scroll = Gtk.ScrolledWindow()
        scroll.set_vexpand(True)
        scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)

        self.app_grid = Gtk.FlowBox()
        self.app_grid.set_max_children_per_line(8)
        self.app_grid.set_min_children_per_line(4)
        self.app_grid.set_column_spacing(16)
        self.app_grid.set_row_spacing(16)
        self.app_grid.set_selection_mode(Gtk.SelectionMode.NONE)
        self.app_grid.set_homogeneous(True)

        scroll.set_child(self.app_grid)
        content.append(scroll)

        main_box.append(content)
        self.window.set_child(main_box)

        # Populate grid
        self.filtered_apps = self.apps
        self._populate_grid()

        self.window.present()

        # Focus search on open
        GLib.idle_add(self.search_entry.grab_focus)

    def _apply_css(self):
        css = b"""
        .launcher-background {
            background-color: rgba(10, 10, 26, 0.92);
        }

        .search-container {
            background-color: rgba(22, 27, 34, 0.9);
            border: 1px solid rgba(0, 255, 135, 0.3);
            border-radius: 16px;
            padding: 8px 20px;
            min-height: 40px;
        }

        .search-entry {
            background: none;
            border: none;
            color: #e0e0e0;
            font-size: 18px;
            caret-color: #00ff87;
        }
        .search-entry:focus {
            box-shadow: none;
            border: none;
        }

        .search-icon {
            font-size: 18px;
            color: rgba(224, 224, 224, 0.5);
        }

        .category-button {
            background: none;
            border: 1px solid transparent;
            color: rgba(224, 224, 224, 0.5);
            font-size: 12px;
            padding: 4px 14px;
            border-radius: 999px;
        }
        .category-button:hover {
            color: #e0e0e0;
            background-color: rgba(255, 255, 255, 0.08);
        }
        .category-active {
            color: #00ff87;
            border-color: rgba(0, 255, 135, 0.3);
            background-color: rgba(0, 255, 135, 0.1);
        }

        .app-button {
            background-color: rgba(22, 27, 34, 0.6);
            border: 1px solid transparent;
            border-radius: 16px;
            padding: 16px;
            min-width: 100px;
            min-height: 100px;
            transition: all 200ms ease;
        }
        .app-button:hover {
            background-color: rgba(0, 255, 135, 0.1);
            border-color: rgba(0, 255, 135, 0.3);
        }

        .app-icon {
            font-size: 36px;
            margin-bottom: 8px;
        }

        .app-name {
            color: #e0e0e0;
            font-size: 12px;
        }

        .app-comment {
            color: rgba(160, 160, 176, 0.7);
            font-size: 10px;
        }
        """
        provider = Gtk.CssProvider()
        provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_display(
            Gdk.Display.get_default(),
            provider,
            Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

    def _load_applications(self):
        """Scan .desktop files for installed applications."""
        search_dirs = [
            "/usr/share/applications",
            "/usr/local/share/applications",
            os.path.expanduser("~/.local/share/applications"),
        ]

        seen = set()
        for dir_path in search_dirs:
            if not os.path.isdir(dir_path):
                continue
            for entry in os.scandir(dir_path):
                if not entry.name.endswith(".desktop"):
                    continue
                if entry.name in seen:
                    continue
                seen.add(entry.name)

                app = self._parse_desktop_file(entry.path)
                if app:
                    self.apps.append(app)

        self.apps.sort(key=lambda a: a.name.lower())

    def _parse_desktop_file(self, path):
        """Parse a .desktop file into an AppEntry."""
        try:
            kf = GLib.KeyFile()
            kf.load_from_file(path, GLib.KeyFileFlags.NONE)

            # Skip hidden and NoDisplay entries
            try:
                if kf.get_boolean("Desktop Entry", "NoDisplay"):
                    return None
            except GLib.Error:
                pass
            try:
                if kf.get_boolean("Desktop Entry", "Hidden"):
                    return None
            except GLib.Error:
                pass

            name = kf.get_locale_string("Desktop Entry", "Name", None)
            icon = None
            try:
                icon = kf.get_string("Desktop Entry", "Icon")
            except GLib.Error:
                pass
            exec_cmd = kf.get_string("Desktop Entry", "Exec")
            # Strip field codes from Exec
            exec_cmd = re.sub(r'%[fFuUdDnNickvm]', '', exec_cmd).strip()

            comment = ""
            try:
                comment = kf.get_locale_string("Desktop Entry", "Comment", None)
            except GLib.Error:
                pass

            categories = []
            try:
                cats = kf.get_string("Desktop Entry", "Categories")
                categories = [c for c in cats.split(";") if c]
            except GLib.Error:
                pass

            return AppEntry(name, icon, exec_cmd, comment, categories)
        except GLib.Error:
            return None

    def _populate_grid(self):
        """Populate the app grid with current filtered apps."""
        # Remove existing children
        child = self.app_grid.get_first_child()
        while child:
            next_child = child.get_next_sibling()
            self.app_grid.remove(child)
            child = next_child

        for app in self.filtered_apps[:64]:  # Limit for performance
            btn = self._create_app_button(app)
            self.app_grid.append(btn)

    def _create_app_button(self, app):
        """Create a button widget for an application."""
        box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=4)
        box.set_halign(Gtk.Align.CENTER)

        # Icon
        icon_widget = Gtk.Image()
        icon_widget.set_pixel_size(48)
        icon_theme = Gtk.IconTheme.get_for_display(Gdk.Display.get_default())
        if app.icon and icon_theme.has_icon(app.icon):
            icon_widget.set_from_icon_name(app.icon)
        else:
            icon_widget.set_from_icon_name("application-x-executable")
        box.append(icon_widget)

        # Name
        name_label = Gtk.Label(label=app.name)
        name_label.add_css_class("app-name")
        name_label.set_ellipsize(3)  # PANGO_ELLIPSIZE_END
        name_label.set_max_width_chars(14)
        box.append(name_label)

        button = Gtk.Button()
        button.set_child(box)
        button.add_css_class("app-button")
        button.connect("clicked", self._on_app_clicked, app)

        return button

    def _on_search_changed(self, entry):
        """Filter apps based on search text."""
        query = entry.get_text().lower().strip()
        if not query:
            self.filtered_apps = self.apps
        else:
            self.filtered_apps = [
                app for app in self.apps
                if self._fuzzy_match(query, app.search_text)
            ]
        self._populate_grid()

    def _fuzzy_match(self, query, text):
        """Simple fuzzy matching."""
        qi = 0
        for char in text:
            if qi < len(query) and char == query[qi]:
                qi += 1
        return qi == len(query)

    def _on_search_activate(self, entry):
        """Launch first result on Enter."""
        if self.filtered_apps:
            self._launch_app(self.filtered_apps[0])

    def _on_app_clicked(self, button, app):
        """Launch clicked application."""
        self._launch_app(app)

    def _launch_app(self, app):
        """Launch an application and close the launcher."""
        try:
            subprocess.Popen(
                app.exec_cmd,
                shell=True,
                start_new_session=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL
            )
        except Exception as e:
            print(f"Failed to launch {app.name}: {e}")
        self.quit()

    def _on_category_clicked(self, button, category):
        """Filter by category."""
        if category == "All":
            self.filtered_apps = self.apps
        else:
            self.filtered_apps = [
                app for app in self.apps
                if category in app.categories
            ]
        self._populate_grid()

    def _on_key_pressed(self, controller, keyval, keycode, state):
        """Handle key presses (Escape to close)."""
        if keyval == Gdk.KEY_Escape:
            self.quit()
            return True
        return False


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app = AuroraLauncher()
    app.run()


if __name__ == "__main__":
    main()

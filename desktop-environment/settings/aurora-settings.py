#!/usr/bin/env python3
"""
Aurora Settings - System Settings Application
Unified settings for Aurora OS.
"""

import gi
gi.require_version('Gtk', '4.0')
gi.require_version('Adw', '1')

from gi.repository import Gtk, Adw, Gio
import signal


class AuroraSettings(Adw.Application):
    """Aurora OS Settings Application."""

    def __init__(self):
        super().__init__(application_id="org.aurora.settings")

    def do_activate(self):
        window = Adw.ApplicationWindow(application=self)
        window.set_title("Settings")
        window.set_default_size(900, 650)

        # Navigation split view
        split = Adw.NavigationSplitView()

        # Sidebar
        sidebar_page = Adw.NavigationPage(title="Settings")
        sidebar_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)

        # Sidebar header
        sidebar_header = Adw.HeaderBar()
        sidebar_header.set_title_widget(Gtk.Label(label="Settings"))
        sidebar_box.append(sidebar_header)

        # Settings categories
        categories = Gtk.ListBox()
        categories.set_selection_mode(Gtk.SelectionMode.SINGLE)
        categories.add_css_class("navigation-sidebar")
        categories.connect("row-selected", self._on_category_selected, split)

        settings_categories = [
            ("🌐", "Network", "Wi-Fi, Ethernet, VPN, Proxy"),
            ("🔊", "Sound", "Output, Input, Volume, Effects"),
            ("🖥️", "Display", "Resolution, Refresh Rate, Night Light"),
            ("🎨", "Appearance", "Theme, Wallpaper, Fonts, Colors"),
            ("🔔", "Notifications", "App Notifications, Do Not Disturb"),
            ("🔋", "Power", "Battery, Sleep, Performance"),
            ("⌨️", "Keyboard", "Layouts, Shortcuts, Input Methods"),
            ("🖱️", "Mouse & Touchpad", "Speed, Gestures, Natural Scroll"),
            ("👤", "Users", "Accounts, Login Screen"),
            ("📦", "Apps", "Default Apps, Installed Apps"),
            ("🔒", "Privacy & Security", "Lock, Encryption, Permissions"),
            ("🕐", "Date & Time", "Timezone, Clock, NTP"),
            ("ℹ️", "About", "System Info, Aurora OS Version"),
        ]

        for icon, name, desc in settings_categories:
            row = Adw.ActionRow(title=f"{icon}  {name}", subtitle=desc)
            row.set_activatable(True)
            row.category_name = name
            categories.append(row)

        scroll = Gtk.ScrolledWindow()
        scroll.set_child(categories)
        scroll.set_vexpand(True)
        sidebar_box.append(scroll)
        sidebar_page.set_child(sidebar_box)

        # Content area (default: Appearance)
        self.content_page = Adw.NavigationPage(title="Appearance")
        self.content_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)

        content_header = Adw.HeaderBar()
        self.content_box.append(content_header)

        self.content_scroll = Gtk.ScrolledWindow()
        self.content_scroll.set_vexpand(True)
        self.content_box.append(self.content_scroll)

        self._show_appearance_page()
        self.content_page.set_child(self.content_box)

        split.set_sidebar(sidebar_page)
        split.set_content(self.content_page)

        window.set_content(split)
        window.present()

    def _on_category_selected(self, listbox, row, split):
        if not row:
            return
        name = row.category_name
        self.content_page.set_title(name)

        page_methods = {
            "Appearance": self._show_appearance_page,
            "Display": self._show_display_page,
            "Sound": self._show_sound_page,
            "About": self._show_about_page,
        }

        method = page_methods.get(name, lambda: self._show_placeholder(name))
        method()

    def _show_appearance_page(self):
        page = Adw.PreferencesPage()

        # Theme group
        theme_group = Adw.PreferencesGroup(title="Theme",
            description="Customize the look and feel")

        # Dark mode toggle
        dark_row = Adw.SwitchRow(title="Dark Mode",
            subtitle="Use dark color scheme (Aurora default)")
        dark_row.set_active(True)
        theme_group.add(dark_row)

        # Accent color
        accent_row = Adw.ComboRow(title="Accent Color")
        accent_model = Gtk.StringList.new([
            "Aurora Green", "Aurora Purple", "Aurora Cyan",
            "Aurora Pink", "Custom"
        ])
        accent_row.set_model(accent_model)
        theme_group.add(accent_row)

        page.add(theme_group)

        # Wallpaper group
        wall_group = Adw.PreferencesGroup(title="Wallpaper")

        wall_row = Adw.ActionRow(title="Current Wallpaper",
            subtitle="/usr/share/backgrounds/aurora/default.png")
        wall_btn = Gtk.Button(label="Browse", valign=Gtk.Align.CENTER)
        wall_row.add_suffix(wall_btn)
        wall_group.add(wall_row)

        page.add(wall_group)

        # Font group
        font_group = Adw.PreferencesGroup(title="Fonts")

        interface_font = Adw.ActionRow(title="Interface Font",
            subtitle="Inter 11")
        font_group.add(interface_font)

        mono_font = Adw.ActionRow(title="Monospace Font",
            subtitle="FiraCode Nerd Font 12")
        font_group.add(mono_font)

        page.add(font_group)

        # Animations group
        anim_group = Adw.PreferencesGroup(title="Animations")

        anim_row = Adw.SwitchRow(title="Enable Animations",
            subtitle="Smooth window transitions and effects")
        anim_row.set_active(True)
        anim_group.add(anim_row)

        speed_row = Adw.ComboRow(title="Animation Speed")
        speed_model = Gtk.StringList.new(["Slow", "Normal", "Fast"])
        speed_row.set_model(speed_model)
        speed_row.set_selected(1)
        anim_group.add(speed_row)

        page.add(anim_group)

        self.content_scroll.set_child(page)

    def _show_display_page(self):
        page = Adw.PreferencesPage()

        display_group = Adw.PreferencesGroup(title="Displays",
            description="Configure connected monitors")

        res_row = Adw.ComboRow(title="Resolution")
        res_model = Gtk.StringList.new([
            "3840×2160", "2560×1440", "1920×1080", "1366×768"
        ])
        res_row.set_model(res_model)
        display_group.add(res_row)

        refresh_row = Adw.ComboRow(title="Refresh Rate")
        refresh_model = Gtk.StringList.new(["144 Hz", "120 Hz", "60 Hz"])
        refresh_row.set_model(refresh_model)
        display_group.add(refresh_row)

        scale_row = Adw.ComboRow(title="Scale")
        scale_model = Gtk.StringList.new(["100%", "125%", "150%", "200%"])
        scale_row.set_model(scale_model)
        display_group.add(scale_row)

        page.add(display_group)

        night_group = Adw.PreferencesGroup(title="Night Light")
        night_row = Adw.SwitchRow(title="Night Light",
            subtitle="Reduce blue light in the evening")
        night_group.add(night_row)
        page.add(night_group)

        self.content_scroll.set_child(page)

    def _show_sound_page(self):
        page = Adw.PreferencesPage()

        output_group = Adw.PreferencesGroup(title="Output")
        vol_row = Adw.ActionRow(title="Volume")
        vol_scale = Gtk.Scale.new_with_range(
            Gtk.Orientation.HORIZONTAL, 0, 100, 1)
        vol_scale.set_value(75)
        vol_scale.set_size_request(200, -1)
        vol_scale.set_valign(Gtk.Align.CENTER)
        vol_row.add_suffix(vol_scale)
        output_group.add(vol_row)

        device_row = Adw.ComboRow(title="Output Device")
        device_model = Gtk.StringList.new(["Speakers", "Headphones", "HDMI"])
        device_row.set_model(device_model)
        output_group.add(device_row)

        page.add(output_group)
        self.content_scroll.set_child(page)

    def _show_about_page(self):
        page = Adw.PreferencesPage()

        about_group = Adw.PreferencesGroup(title="Aurora OS")

        version_row = Adw.ActionRow(title="Version", subtitle="0.1.0 (Alpha)")
        about_group.add(version_row)

        kernel_row = Adw.ActionRow(title="Kernel", subtitle="Linux (Arch)")
        about_group.add(kernel_row)

        de_row = Adw.ActionRow(title="Desktop", subtitle="Aurora Desktop Environment")
        about_group.add(de_row)

        wm_row = Adw.ActionRow(title="Compositor", subtitle="Aurora Compositor (wlroots)")
        about_group.add(wm_row)

        page.add(about_group)

        hw_group = Adw.PreferencesGroup(title="Hardware")
        cpu_row = Adw.ActionRow(title="Processor", subtitle="Detecting...")
        hw_group.add(cpu_row)
        mem_row = Adw.ActionRow(title="Memory", subtitle="Detecting...")
        hw_group.add(mem_row)
        gpu_row = Adw.ActionRow(title="Graphics", subtitle="Detecting...")
        hw_group.add(gpu_row)
        page.add(hw_group)

        self.content_scroll.set_child(page)

    def _show_placeholder(self, name):
        page = Adw.PreferencesPage()
        group = Adw.PreferencesGroup(title=name,
            description="This settings page is under construction.")
        page.add(group)
        self.content_scroll.set_child(page)


def main():
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    app = AuroraSettings()
    app.run()


if __name__ == "__main__":
    main()

project(
  'aurora-compositor',
  'c',
  version: '0.1.0',
  license: 'GPL-3.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
  ],
)

# Dependencies
wayland_server = dependency('wayland-server')
wayland_protocols = dependency('wayland-protocols')
wlroots = dependency('wlroots', version: '>= 0.17')
xkbcommon = dependency('xkbcommon')
pixman = dependency('pixman-1')
math = meson.get_compiler('c').find_library('m')

aurora_deps = [
  wayland_server,
  wayland_protocols,
  wlroots,
  xkbcommon,
  pixman,
  math,
]

aurora_sources = files(
  'src/main.c',
  'src/server.c',
  'src/output.c',
  'src/input.c',
  'src/view.c',
  'src/xdg_shell.c',
  'src/layer_shell.c',
  'src/decoration.c',
  'src/animation.c',
  'src/config.c',
)

executable(
  'aurora-compositor',
  aurora_sources,
  dependencies: aurora_deps,
  install: true,
)

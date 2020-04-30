project('mish', 'c', version: '0.1.0', meson_version: '>= 0.40.0')

rl_d = dependency('readline')
glib_d = dependency('glib-2.0')

source_c = [
  'main.c'
]

mish = executable('mish',
  source_c,
  dependencies : [ rl_d, glib_d ],
  install : true)

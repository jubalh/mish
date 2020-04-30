project('mish', 'c', version: '0.1.0', meson_version: '>= 0.40.0')

rl_d = dependency('readline')
glib_d = dependency('glib-2.0')

conf = configuration_data()
conf.set('VERSION', '"' + meson.project_version() + '"', description : 'mish shell version')

config_h = configure_file(output : 'config.h', configuration : conf)

source_c = [
  'main.c',
  config_h
]

mish = executable('mish',
  source_c,
  dependencies : [ rl_d, glib_d ],
  install : true)

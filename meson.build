project('SRAL', ['c', 'cpp'],
  version : '1.0.0',
  default_options : [
    'cpp_std=c++20',
    'default_library=both',
    'warning_level=2'
  ]
)

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')
host_os = host_machine.system()

inc_dir = include_directories('Include')

build_test = get_option('build_sral_test')
disable_uia = get_option('sral_disable_uia')

sral_sources = [
  'SRC/Encoding.cpp',
  'SRC/SRAL.cpp',
  'SRC/Engine.cpp'
]

sral_deps = []
sral_args = ['-D_CRT_SECURE_NO_WARNINGS']

if host_os == 'windows'
  sral_sources += [
    'SRC/NVDA.cpp', 'SRC/SAPI.cpp',
    'Dep/blastspeak.c', 'Dep/fsapi.c', 'Dep/nvda_control.c',
    'SRC/Jaws.cpp', 'Dep/wasapi.cpp', 'SRC/ZDSR.cpp'
  ]
  
  if not disable_uia
    sral_sources += ['SRC/UIA.cpp', 'Dep/UIAProvider.cpp']
    sral_deps += cpp.find_library('uiautomationcore')
  else
    sral_args += '-DSRAL_NO_UIA'
  endif

elif host_os == 'darwin'
  add_languages('objcpp')
  sral_sources += ['SRC/AVSpeech.mm', 'SRC/VoiceOver.mm']
  sral_deps += dependency('appleframeworks', modules : ['AppKit', 'Foundation', 'AVFoundation'])

else
  sral_sources += ['Dep/utf-8.c', 'SRC/SpeechDispatcher.cpp']
  sral_deps += dependency('speech-dispatcher')
  sral_deps += cpp.find_library('brlapi')
endif

sral_lib = library('SRAL',
  sral_sources,
  include_directories : inc_dir,
  dependencies : sral_deps,
  cpp_args : sral_args,
  c_args : sral_args,
  install : true
)

install_headers('Include/SRAL.h')

if build_test
  executable('SRAL_test',
    'Examples/C/SRALExample.c',
    include_directories : inc_dir,
    link_with : sral_lib,
    dependencies : sral_deps
  )

  if host_os == 'windows'
    executable('SRAL_NVDAControleExConsole',
      ['Examples/C/NVDAControlExConsole.c', 'Dep/nvda_control.c'],
      include_directories : inc_dir
    )
  endif
endif

summary({
  'Build tests': build_test,
  'UIA support disabled': disable_uia,
  'Library type': get_option('default_library'),
  'C++ Standard': get_option('cpp_std')
}, bool_yn: true, section: 'Configuration')

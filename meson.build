project('uiu', ['c', 'cpp'], version : '0.1', default_options : ['cpp_std=c++23'])

gnu_efi_dep = dependency('gnu-efi')

gnu_efi_libdir = gnu_efi_dep.get_pkgconfig_variable('libdir')

# The correct solution would be to use `includes : true`, but ...
# https://github.com/mesonbuild/meson/issues/7516
gnu_efi_part_dep = gnu_efi_dep.partial_dependency(compile_args : true)

efi_c_args = ['-fpic', '-ffreestanding', '-fno-stack-protector', '-fno-stack-check', '-fshort-wchar', '-mno-red-zone', '-maccumulate-outgoing-args']
efi_cpp_args = efi_c_args

efi_link_args = ['-shared', '-Wl,-Bsymbolic']

objcopy_cmd = ['objcopy', '-j', '.text', '-j', '.sdata', '-j', '.data', '-j', '.rodata', '-j', '.dynamic', '-j', '.dynsym',  '-j', '.rel', '-j', '.rela', '-j', '.rel.*', '-j', '.rela.*', '-j', '.reloc', '--target', 'efi-app-x86_64', '--subsystem=10', '@INPUT@', '@OUTPUT@']

hello_exe = executable(
  'hello',
  [
    'hello.c',
  ],
  dependencies : [
    gnu_efi_dep,
    # gnu efi's pkgconfig file doesn't add -lgnuefi
    meson.get_compiler('c').find_library('libgnuefi'),
  ],
  c_args : efi_c_args,
  link_args : efi_link_args + ['-Wl,-T'+gnu_efi_libdir+'/elf_x86_64_efi.lds'],
  objects : [gnu_efi_libdir+'/crt0-efi-x86_64.o'],
)

custom_target(
  'hello_efi',
  command : objcopy_cmd,
  input : hello_exe,
  output : 'hello.efi',
  build_always : true,
)

start_exe = executable(
  'start',
  [
    'start.cpp',
  ],
  dependencies : [
    gnu_efi_part_dep,
  ],
  cpp_args : efi_cpp_args + ['-mno-mmx', '-mno-sse'],
  link_args: efi_link_args,
)

custom_target(
  'start_efi',
  command : objcopy_cmd,
  input : start_exe,
  output : 'start.efi',
  build_always : true,
)

uiu_exe = executable(
  'uiu',
  [
    'uiu.cpp',
  ],
  dependencies : [
    dependency('fmt'),
    gnu_efi_part_dep,
  ],
  cpp_args : ['-fshort-wchar'],
  install : true,
)

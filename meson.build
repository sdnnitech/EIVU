
project('SIVU', 'c',
        default_options: [
            'buildtype=release',
            'c_std=c11',
            'default_library=static',
            'warning_level=2',
        ],
)

cc = meson.get_compiler('c')

add_global_arguments('-D_XOPEN_SOURCE=600', language : 'c')
add_global_arguments('-D_GNU_SOURCE', language : 'c')

if get_option('developer_mode')
    add_global_arguments('-DDEBUG', language : 'c')
endif

incdir = include_directories('lib/common')

subdir('test')

executable('nf', 'src/nf.c', include_directories : incdir)
executable('rx', 'src/rx.c', include_directories : incdir)
executable('tx', 'src/tx.c', include_directories : incdir)


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

# add_global_arguments('-DDEBUG', language : 'c')

# incdir = include_directories('lib')
# executable('nf', 'src/nf.c', include_directories : incdir)

subdir('test')

executable('nf', 'src/nf.c')
executable('rx', 'src/rx.c')
executable('tx', 'src/tx.c')

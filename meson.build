
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

md = get_option('metadata_size')
add_global_arguments(f'-DMETADATA_SIZE=@md@', language : 'c')

dataroom = get_option('dataroom_size')
add_global_arguments(f'-DMBUF_DATAROOM_SIZE=@dataroom@', language : 'c')

headroom = get_option('headroom_size')
add_global_arguments(f'-DMBUF_HEADROOM_SIZE=@headroom@', language : 'c')

if get_option('zero_copy')
    if get_option('developer_mode')
        error('Not support the combination of options')
    endif

    run_command('zero_copy.sh', check : true)
endif

if get_option('vio_header')
    add_global_arguments('-DVIO_HEADER', language : 'c')
endif

if get_option('developer_mode')
    add_global_arguments('-DDEBUG', language : 'c')
endif

incdir_host = []
incdir_guest = []
args_host = []
args_guest = []

subdir('lib')

subdir('test')

executable('nf', 'src/nf.c', include_directories : incdir_guest, c_args : args_guest)
executable('rx', 'src/rx.c', include_directories : incdir_host, c_args : args_host)
executable('tx', 'src/tx.c', include_directories : incdir_host, c_args : args_host)

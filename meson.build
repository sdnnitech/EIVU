
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
headroom = get_option('headroom_size')

CACHELINE_SIZE = 64

mbuf_alignment = get_option('mbuf_alignment')
mdbuf = md
if mbuf_alignment == 'mbuf'
    if mdbuf % 64 != 0
        mdbuf += CACHELINE_SIZE - mdbuf % CACHELINE_SIZE
    endif
    if (dataroom + headroom) % 64 != 0
        dataroom += CACHELINE_SIZE - (dataroom + headroom) % CACHELINE_SIZE
    endif
endif
if mbuf_alignment == 'pkt'
    if  get_option('separated_md') or get_option('aggregated_md')
        if (dataroom + headroom) % 64 != 0
            dataroom += CACHELINE_SIZE - (dataroom + headroom) % CACHELINE_SIZE
        endif
    else
        if (mdbuf + dataroom + headroom) % 64 != 0
            dataroom += CACHELINE_SIZE - (mdbuf + dataroom + headroom) % CACHELINE_SIZE
        endif
    endif
endif

add_global_arguments(f'-DMBUF_DATAROOM_SIZE=@dataroom@', language : 'c')
add_global_arguments(f'-DMBUF_HEADROOM_SIZE=@headroom@', language : 'c')

zerocpy = get_option('zero_copy')
foreach zc : zerocpy
    if get_option('developer_mode')
        error('Not support the combination of options')
    endif

    if zc == 'rx'
        add_global_arguments('-DZEROCOPY_RX', language : 'c')
    elif zc == 'tx'
        add_global_arguments('-DZEROCOPY_TX', language : 'c')
    endif
endforeach

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

if not get_option('aggregated_md')
    args_guest += f'-DMDBUF_SIZE=@mdbuf@'
endif
if not get_option('host_aggregated_md')
    args_host += f'-DMDBUF_SIZE=@mdbuf@'
endif

subdir('lib')

subdir('test')

executable('nf', 'src/nf.c', include_directories : incdir_guest, c_args : args_guest)
executable('rx', 'src/rx.c', include_directories : incdir_host, c_args : args_host)
executable('tx', 'src/tx.c', include_directories : incdir_host, c_args : args_host)

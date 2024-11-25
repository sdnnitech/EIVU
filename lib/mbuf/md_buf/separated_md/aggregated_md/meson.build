
agsz = get_option('aggregation_num')
md = get_option('metadata_size')
mdbufsz = md * agsz * 2 - md * agsz * 2 % 64
mdbufnum = 36864 / agsz - 36864 / agsz % 64
add_global_arguments(f'-DAGGREGATED_MD_NUM=@agsz@', language : 'c')
add_global_arguments(f'-DMDBUF_SIZE=@mdbufsz@', language : 'c')
add_global_arguments(f'-DMDBUF_NUM=@mdbufnum@', language : 'c')

if get_option('aggregated_md')
    if get_option('reuse')
        args_guest += '-DMD_MEMOBJ_CACHE_STACK'
        incdir_guest += include_directories('reuse')
    else
        args_guest += '-DMD_MEMOBJ_CACHE_STACK'
        incdir_guest += include_directories('mpool')
    endif
endif

if get_option('host_aggregated_md')
    if get_option('reuse')
        args_host += '-DMD_MEMOBJ_CACHE_STACK'
        incdir_host += include_directories('reuse')
    else
        args_host += '-DMD_MEMOBJ_CACHE_STACK'
        incdir_host += include_directories('mpool')
    endif
endif

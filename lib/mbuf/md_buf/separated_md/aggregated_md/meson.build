
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

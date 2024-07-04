
incdir_host += include_directories('common')
incdir_guest += include_directories('common')

if not get_option('host_aggregated_md')
    args_host += '-DMD_MEMOBJ_CACHE_STACK'
    incdir_host += include_directories('integrated_md')
endif

if not get_option('md_queue')
    if get_option('separated_md') or get_option('reuse') or get_option('aggregated_md')
        subdir('separated_md')
    else
        args_guest += '-DMD_MEMOBJ_CACHE_STACK'
        incdir_guest += include_directories('integrated_md')
        args_host += '-DGUEST_INTEGRATED_MD'
    endif
endif

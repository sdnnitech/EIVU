
incdir_host += include_directories('common')
incdir_guest += include_directories('common')

if get_option('md_queue')
    if get_option('separated_md') or get_option('aggregated_md')
        error('Not support the combination of options')
    endif
else
    if not get_option('separated_md') and not get_option('aggregated_md')
        args_guest += '-DMD_MEMOBJ_CACHE_STACK'
        args_guest += '-DINTEGRATED_MD'
        incdir_guest += include_directories('integrated_md')
        args_host += '-DGUEST_INTEGRATED_MD'
    endif
endif

if not get_option('host_aggregated_md') and not get_option('separated_md')
    args_host += '-DMD_MEMOBJ_CACHE_STACK'
    args_host += '-DINTEGRATED_MD'
    incdir_host += include_directories('integrated_md')
endif

if get_option('separated_md') or get_option('aggregated_md') or get_option('host_aggregated_md')
    subdir('separated_md')
endif

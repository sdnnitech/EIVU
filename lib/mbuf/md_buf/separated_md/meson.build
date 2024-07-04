# is related with md-area and pktbuf

if get_option('host_aggregated_md')
    args_host += '-DMD_MEMOBJ_CACHE_STACK'
    incdir_host += include_directories('aggregated_md')
endif

if get_option('aggregated_md')
    args_guest += '-DMD_MEMOBJ_CACHE_STACK'
    incdir_guest += include_directories('aggregated_md')
else
    subdir('individual_md')
endif

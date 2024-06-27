# is related with md-area and pktbuf

if get_option('aggregated_md')
    args_guest += '-DMD_MEMOBJ_CACHE_STACK'
    incdir_guest += include_directories('aggregated_md')
else
    subdir('individual_md')
endif

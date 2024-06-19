# is related with md-area and pktbuf
incdir += include_directories('common')

if get_option('aggregated_md')
    add_global_arguments('-DMD_MEMOBJ_CACHE_STACK', language : 'c')
    incdir += include_directories('aggregated_md')
else
    subdir('individual_md')
endif

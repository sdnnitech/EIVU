
add_global_arguments('-DPKTBUF_MEMOBJ_CACHE_STACK', language : 'c')

incdir += include_directories('common')

if get_option('separated_md')
    subdir('separated_md')
else
    incdir += include_directories('integrated_md')
endif

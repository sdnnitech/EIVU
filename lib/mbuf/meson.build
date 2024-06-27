
add_global_arguments('-DPKTBUF_MEMOBJ_CACHE_STACK', language : 'c')

incdir_host += include_directories('common')
incdir_guest += include_directories('common')

if get_option('md_queue')
    incdir_host += include_directories('host_led')
    incdir_guest += include_directories('host_led')
    args_guest += '-DHOST_LED'
else
    incdir_host += include_directories('guest_led')
    incdir_guest += include_directories('guest_led')
    args_guest += '-DGUEST_LED'
endif

if get_option('md_queue')
    incdir_guest += include_directories('md_que')
else
    incdir_guest += include_directories('md_buf')
endif

# subdir('md_buf')

args_host += '-DMD_MEMOBJ_CACHE_STACK'
incdir_host += include_directories('integrated_md')

if get_option('separated_md')
    subdir('separated_md')
else
    args_guest += '-DMD_MEMOBJ_CACHE_STACK'
    incdir_guest += include_directories('integrated_md')
endif


add_global_arguments('-DPKTBUF_MEMOBJ_CACHE_STACK', language : 'c')

incdir_host += include_directories('common')
incdir_guest += include_directories('common')

if get_option('memcpy_md')
    if get_option('aggregated_md')
        error('Not support the combination of options')
    endif

    if get_option ('md_queue')
        error('Not support the combination of options')
    endif
endif

if get_option('md_queue') or get_option('memcpy_md')
    incdir_host += include_directories('host_led')
    incdir_guest += include_directories('host_led')
    args_guest += '-DHOST_LED'
    args_host += '-DHOST_LED'
else
    incdir_host += include_directories('guest_led')
    incdir_guest += include_directories('guest_led')
    args_guest += '-DGUEST_LED'
    args_host += '-DGUEST_LED'
endif

if get_option('md_queue')
    args_guest += '-DMDQUE'
    args_host += '-DMDQUE'
    incdir_guest += include_directories('md_que')
endif

subdir('md_buf')

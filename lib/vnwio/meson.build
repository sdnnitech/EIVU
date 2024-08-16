# if DESC_SIZE == 4 || get_option('fastest')
    # if BUF_NUM >= 32768 || get_option('separated_md')
    # "Invalid"
    # else
    # add_global_arguments('-DDESC_SIZE=4', language : 'c')
    # endif
# else if DESC_SIZE == 8
    # if BUF_NUM >= 32768
    # 
    # else
    #
    # endif
# endif

if get_option('fastest_mode')
    if get_option('developer_mode')
        error('Not support the combination of options')
    endif

    if get_option('separated_md')
        # error('Not support the combination of options')
    endif

    add_global_arguments('-DDESC_SIZE=8', language : 'c')
endif

incdir_guest += include_directories('.')
incdir_host += include_directories('.')

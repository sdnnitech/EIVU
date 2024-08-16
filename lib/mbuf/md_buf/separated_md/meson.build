# is related with md-area and pktbuf

if get_option('host_aggregated_md') or get_option('aggregated_md')
    subdir('aggregated_md')
endif

if not get_option('host_aggregated_md') or not get_option('aggregated_md')
    subdir('individual_md')
endif

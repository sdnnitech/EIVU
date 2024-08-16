# is related with md-area and pktbuf
incdir_guest += include_directories('common')
incdir_host += include_directories('common')

if get_option('reuse')
    incdir_guest += include_directories('reuse')
    incdir_host += include_directories('reuse')
else
    subdir('mpool')
endif

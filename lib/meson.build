# deal with switching implementation of Mbuf patterns

incdir += include_directories('common')

if get_option('separated_md')
    subdir('separated_md')
else
    incdir += include_directories('integrated_md')
endif

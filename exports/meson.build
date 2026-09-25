# Modified library code

motion_vectors_export = executable('motion_vectors_export',
    'motion_vectors.c',
    link_with: [libdav1d, dav1d_input_objs],
    include_directories: [dav1d_inc_dirs, include_directories('../tools')],
    dependencies: [thread_dependency, rt_dependency, getopt_dependency],
    install: false,
)

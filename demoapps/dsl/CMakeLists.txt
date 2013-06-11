project(GraphLab)

add_graphlab_executable(gl_server gl_server.cpp)

add_library(gen_impl SHARED gen_impl.cpp)
add_library(gen_test SHARED gen_test.cpp)
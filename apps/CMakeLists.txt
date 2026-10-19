add_executable(compute_node compute_node.cpp ../include/benchmark/ycsb/distribution/constant_generator.h)

target_compile_features(compute_node PRIVATE cxx_std_17)

target_link_libraries(compute_node PUBLIC processor_lib)

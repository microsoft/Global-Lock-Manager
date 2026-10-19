add_executable(common_tests
        "test_typedefs.cpp"
        "test_thread_pin.cpp")

target_link_libraries(common_tests GTest::gtest_main common_lib)

include(GoogleTest)
gtest_discover_tests(common_tests)

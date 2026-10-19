add_executable(lock_manager_tests
        "lock_manager/test_lock_mode.cpp"
        "lock_manager/test_transaction_locks_queue.cpp"
        "lock_manager/test_node_lock_state.cpp"
        buffer_manager/test_buffer_manager.cpp)

target_link_libraries(lock_manager_tests GTest::gtest_main storage_lib)

include(GoogleTest)
gtest_discover_tests(lock_manager_tests)

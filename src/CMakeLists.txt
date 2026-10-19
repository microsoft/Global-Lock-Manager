include(FetchContent)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.10.0
)
FetchContent_MakeAvailable(spdlog)

add_subdirectory(common)
add_subdirectory(transport)
add_subdirectory(storage)
add_subdirectory(query)
add_subdirectory(benchmark)
add_subdirectory(processor)

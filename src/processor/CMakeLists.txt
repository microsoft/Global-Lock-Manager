# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB PROC_HEADER_LIST CONFIGURE_DEPENDS "${GlobalLocalManager_SOURCE_DIR}/include/processor/*.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(processor_lib STATIC processor.cpp ${PROC_HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(processor_lib PUBLIC ../../include)

# All users of this library will need at least C++17
target_compile_features(processor_lib PUBLIC cxx_std_17)

target_link_libraries(processor_lib PUBLIC benchmark_lib)

# IDEs should put the headers in a nice place
source_group(
	TREE "${PROJECT_SOURCE_DIR}/include"
	PREFIX "Header Files"
	FILES ${PROC_HEADER_LIST}
)

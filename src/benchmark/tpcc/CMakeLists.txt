# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB QUERY_TPCC_HEADER_LIST CONFIGURE_DEPENDS "${GlobalLocalManager_SOURCE_DIR}/include/query/*.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(tpcc_lib STATIC
    tpcc_database_state.cpp
    ${QUERY_TPCC_HEADER_LIST}
)

# We need this directory, and users of our library will need it too
target_include_directories(tpcc_lib PUBLIC ../../../include)

# All users of this library will need at least C++17
target_compile_features(tpcc_lib PUBLIC cxx_std_17)

target_link_libraries(tpcc_lib PUBLIC query_lib)

# IDEs should put the headers in a nice place
source_group(
	TREE "../../../include"
	PREFIX "Header Files"
	FILES ${QUERY_TPCC_HEADER_LIST}
)

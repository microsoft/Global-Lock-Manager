# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
file(GLOB BUFFER_MANAGER_HEADER_LIST CONFIGURE_DEPENDS
		"${GlobalLocalManager_SOURCE_DIR}/include/storage/buffer_manager/*.h")
file(GLOB LOCK_MANAGER_HEADER_LIST CONFIGURE_DEPENDS
		"${GlobalLocalManager_SOURCE_DIR}/include/storage/lock_manager/*.h"
		"${GlobalLocalManager_SOURCE_DIR}/include/storage/lock_manager/locks/*.h")
file(GLOB LOG_MANAGER_HEADER_LIST CONFIGURE_DEPENDS
		"${GlobalLocalManager_SOURCE_DIR}/include/storage/log_manager/*.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(storage_lib STATIC
	buffer_manager/buffer_manager.cpp
	buffer_manager/frame.cpp
	lock_manager/locks/lock_mode.cpp
	lock_manager/locks/node_lock.cpp
	lock_manager/locks/transaction_lock_pool.cpp
	lock_manager/locks/transaction_locks_queue.cpp
	lock_manager/locks/transaction_locks_queue_state.cpp
	lock_manager/lock_manager.cpp
	lock_manager/obj_id_to_obj_locks_map.cpp
	log_manager/log_manager.cpp
	${BUFFER_MANAGER_HEADER_LIST}
	${LOCK_MANAGER_HEADER_LIST}
	${LOG_MANAGER_HEADER_LIST}
)

# We need this directory, and users of our library will need it too
target_include_directories(storage_lib PUBLIC ../../include)

# All users of this library will need at least C++17
target_compile_features(storage_lib PUBLIC cxx_std_17)

target_link_libraries(storage_lib PUBLIC transport_lib)

# IDEs should put the headers in a nice place
source_group(
	TREE "${PROJECT_SOURCE_DIR}/include"
	PREFIX "Header Files"
	FILES ${BUFFER_MANAGER_HEADER_LIST}
		  ${LOCK_MANAGER_HEADER_LIST}
		  ${LOG_MANAGER_HEADER_LIST}
)

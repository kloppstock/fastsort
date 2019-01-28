cmake_minimum_required ( VERSION 3.7 )

# download and unpack googletest when configuring
configure_file ( CMakeLists.txt.in googletest-download/CMakeLists.txt )
execute_process ( COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
				RESULT_VARIABLE result
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if ( result )
   message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ( )
execute_process ( COMMAND ${CMAKE_COMMAND} --build .
				RESULT_VARIABLE result
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if ( result )
   message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif ( )

set ( gtest_force_shared_crt ON CACHE BOOL "" FORCE )

add_subdirectory ( ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
				 ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
				 EXCLUDE_FROM_ALL )



set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address" )


set ( CMAKE_CXX_STANDARD 11 )
add_executable ( test testmain.cpp )
add_test ( NAME SortTests COMMAND test )
target_link_libraries ( test gtest_main )

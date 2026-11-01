# The following CMakeLists.txt file is responsible for building and testing jdm-array.

cmake_minimum_required(VERSION 3.10)
project(jdm-array VERSION 1.0 LANGUAGES C)

# Set the C standard to C11
set(CMAKE_C_STANDARD 11)

set(CMAKE_REQUIRED_LIBRARIES jdm-alloc jdm-log)

option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

if(ENABLE_ASAN)
    message(STATUS "Address sanitizer enabled")
    
    # Add sanitizer flags for both compiling and linking
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

option(ENABLE_UBSAN "Enable UB Sanitizer" OFF)

if(ENABLE_UBSAN)
    message(STATUS "UB sanitizer enabled")
    
    # Add sanitizer flags for both compiling and linking
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)

if(ENABLE_TSAN)
    message(STATUS "Thread sanitizer enabled")
    
    # Add sanitizer flags for both compiling and linking
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread)
endif()

# Add the source files for the jdm-array library
set(SOURCES
    src/jdm-array.c
)
# Create the jdm-array library
add_library(jdm-array STATIC ${SOURCES})

# Include the header files for the jdm-array library
target_include_directories(jdm-array PUBLIC include)

# Link the jdm-log library to the jdm-alloc library
target_link_libraries(jdm-array PRIVATE jdm-alloc jdm-log)

# Add the test executables
add_executable(test-jdm-array tests/test-jdm-array.c)
target_link_libraries(test-jdm-array PRIVATE jdm-array)
add_test(NAME test-jdm-array COMMAND test-jdm-array)

# Enable testing
enable_testing()

# Set the CMake build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set the output directories for the build artifacts
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set the compiler flags for the Release build type
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set the compiler flags for the Debug build type
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")

# Set the compiler flags for the Sanitized build type
set(CMAKE_C_FLAGS_SANITIZED "-fsanitize=address -g -O1 -fno-omit-frame-pointer -DDEBUG")

# Set the compiler flags for the RelWithDebInfo build type
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# Set the compiler flags for the MinSizeRel build type
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

# Build the jdm-array library and the test executable
add_custom_target(build-all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target jdm-array
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test-jdm-array
    COMMENT "Building jdm-array library and test executable"
)

# set the default target to build-all
set_property(DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/bin)

# Add a custom target to run the tests
add_custom_target(run-tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test-jdm-array
    COMMENT "Running tests for jdm-array"
)

# build release and run tests
add_custom_target(build-release-and-test
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config Release
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    COMMENT "Building jdm-array library in Release mode and running tests"
)

# set install target for the jdm-array library and header files
install(TARGETS jdm-array
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)    
install(DIRECTORY include/ DESTINATION include)
install(TARGETS 
    test-jdm-array
    RUNTIME DESTINATION bin
)

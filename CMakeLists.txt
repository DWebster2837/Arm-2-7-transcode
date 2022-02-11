#Top-level CMake project file

cmake_minimum_required (VERSION 3.8)

project ("transcoder")

# Include sub-projects.
add_subdirectory ("src")


#Force MSVC to compile C++20 or higher. Will need changing once MSVC properly supports std::format. 
if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_23)
else ()
    target_compile_features("${PROJECT_NAME}" PUBLIC cxx_std_20)
endif ()
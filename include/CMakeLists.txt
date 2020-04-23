# list include header files in Visual Studio Solution Explorer

file(GLOB_RECURSE IncludeHeaders "${CMAKE_SOURCE_DIR}/include/*.h*")
add_library(Include ${IncludeHeaders})
set_target_properties(Include PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(Include PRIVATE "${CMAKE_SOURCE_DIR}/include")

set(UPNPC_BUILD_STATIC ON CACHE BOOL "Build static library")
set(UPNPC_BUILD_SHARED OFF CACHE BOOL "Build shared library")
set(UPNPC_BUILD_TESTS OFF CACHE BOOL "Build test executables")

add_subdirectory(miniupnpc)
add_subdirectory(gtest)

set_property(TARGET upnpc-static gtest gtest_main PROPERTY FOLDER "external")

if(MSVC)
  set_property(TARGET upnpc-static APPEND_STRING PROPERTY COMPILE_FLAGS " -wd4244 -wd4267")
elseif(FREEBSD)
  set_property(TARGET upnpc-static APPEND_STRING PROPERTY COMPILE_FLAGS " -D_XOPEN_SOURCE=600 -Wno-undef -Wno-unused-result -Wno-unused-value")
else()
  set_property(TARGET upnpc-static APPEND_STRING PROPERTY COMPILE_FLAGS " -Wno-undef -Wno-unused-result -Wno-unused-value")
endif()

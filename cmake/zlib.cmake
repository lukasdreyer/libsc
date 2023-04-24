# build Zlib to ensure compatibility.
# We use Zlib 2.x for speed and robustness.
include(GNUInstallDirs)
include(ExternalProject)

# option to chose if we want to download zlib sources, or use a local archive file
option(LIBSC_USE_ZLIB_ARCHIVE "Turn ON if you want to use a local archive of zlib sources (default: OFF)." OFF)

# select which version of zlib will be build
if (NOT DEFINED LIBS_BUILD_ZLIB_VERSION)
  set(LIBSC_BUILD_ZLIB_VERSION 2.0.6)
endif()

# default zlib source archive
if (NOT DEFINED LIBSC_BUILD_ZLIB_ARCHIVE_FILE)
  set(LIBSC_BUILD_ZLIB_ARCHIVE_FILE https://github.com/zlib-ng/zlib-ng/archive/refs/tags/${LIBSC_BUILD_ZLIB_VERSION}.tar.gz CACHE STRING "zlib source archive (URL or local filepath).")
endif()

set(ZLIB_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/include)

if(BUILD_SHARED_LIBS)
  if(WIN32)
    set(ZLIB_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}zlib1${CMAKE_SHARED_LIBRARY_SUFFIX})
  else()
    set(ZLIB_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}z${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
else()
  if(MSVC)
    set(ZLIB_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX})
  else()
    set(ZLIB_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}z${CMAKE_STATIC_LIBRARY_SUFFIX})
  endif()
endif()

set(zlib_cmake_args
-DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
-DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
-DCMAKE_BUILD_TYPE=Release
-DZLIB_COMPAT:BOOL=on
-DZLIB_ENABLE_TESTS:BOOL=off
-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
)

if(LIBSC_USE_ZLIB_ARCHIVE)
  ExternalProject_Add(ZLIB
    URL ${LIBSC_BUILD_ZLIB_ARCHIVE_FILE}
    CMAKE_ARGS ${zlib_cmake_args}
    BUILD_BYPRODUCTS ${ZLIB_LIBRARIES}
    TLS_VERIFY true
    CONFIGURE_HANDLED_BY_BUILD ON
    INACTIVITY_TIMEOUT 60
    )
else()
  ExternalProject_Add(ZLIB
    GIT_REPOSITORY https://github.com/zlib-ng/zlib-ng.git
    GIT_TAG ${LIBSC_BUILD_ZLIB_VERSION}
    GIT_SHALLOW true
    CMAKE_ARGS ${zlib_cmake_args}
    BUILD_BYPRODUCTS ${ZLIB_LIBRARIES}
    TLS_VERIFY true
    CONFIGURE_HANDLED_BY_BUILD ON
    INACTIVITY_TIMEOUT 60
    )
endif()

# --- imported target

file(MAKE_DIRECTORY ${ZLIB_INCLUDE_DIRS})
# avoid race condition

add_library(ZLIB::ZLIB INTERFACE IMPORTED GLOBAL)
add_dependencies(ZLIB::ZLIB ZLIB)  # to avoid include directory race condition
target_link_libraries(ZLIB::ZLIB INTERFACE ${ZLIB_LIBRARIES})
target_include_directories(ZLIB::ZLIB INTERFACE ${ZLIB_INCLUDE_DIRS})

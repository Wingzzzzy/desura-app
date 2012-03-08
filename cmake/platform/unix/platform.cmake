###############################################################################
# some global defines
###############################################################################

add_definitions(-DBOOST_FILESYSTEM_VERSION=2 -D_LARGE_FILES
    -D_FILE_OFFSET_BITS=64)

###############################################################################
# some compiler flags
###############################################################################

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
else()
    add_definitions(-DNDEBUG)
endif()

add_compiler_flags(-fPIC -pipe -fvisibility=hidden -Wl,-Bsymbolic-functions -lpthread -g3)
add_compiler_flags(DEBUG -rdynamic -fno-omit-frame-pointer)
add_compiler_flags(RELEASE -O2)

###############################################################################
# important C/C++ flags
###############################################################################

# Desura uses the awesome new C++ Standard
add_compiler_flags(CXX -std=c++0x)

# Karol Herbst: I get an error while building without that
add_definitions(-fpermissive)

# wxWidgets config
add_definitions(-D__WXGTK__)
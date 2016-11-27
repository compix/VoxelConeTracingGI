# Tries to find GLEW
# Set the environment variable "GLEW" if it's not installed in the common places.
# If GLEW was found then the following variables will be set:
# GLEW_FOUND
# GLEW_LIBRARY/GLEW_LIBRARIES
# GLEW_INCLUDE_DIR/GLEW_INCLUDE_DIRS

set(GLEW_PATHS	${GLEW}
				$ENV{GLEW}
				~/Library/Frameworks
				/Library/Frameworks
				/usr/local
				/usr
				/sw
				/opt/local
				/opt/csw
				/opt)

find_path(GLEW_INCLUDE_DIR GL/glew.h PATH_SUFFIXES include PATHS ${GLEW_PATHS})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	# 64bit architecture
	find_library(GLEW_LIBRARY NAMES glew32 glew GLEW PATH_SUFFIXES lib lib/x64 PATHS ${GLEW_PATHS})
else()
	# 32bit architecture
	find_library(GLEW_LIBRARY NAMES glew32 glew PATH_SUFFIXES lib lib/Win32 PATHS ${GLEW_PATHS})
endif()

set(GLEW_LIBRARIES ${GLEW_LIBRARY})
set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})

mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLEW REQUIRED_VARS GLEW_LIBRARY GLEW_INCLUDE_DIR)

if(${GLEW_LIBRARY} AND ${GLEW_INCLUDE_DIR})
	set(GLEW_FOUND TRUE)
endif()
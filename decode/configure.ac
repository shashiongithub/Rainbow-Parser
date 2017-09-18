# this is example-file: configure.ac

# initial information about the project
AC_INIT([lpcfgparser],[0.1],[scohen@inf.ed.ac.uk])

# check if the source folder is available
AC_CONFIG_SRCDIR([src/main.cpp])

# check for C++ preprocessor and compiler
CXXFLAGS="$CXXFLAGS -std=c++0x"
CXXFLAGS="$CXXFLAGS -O3"

#AX_CHECK_COMPILE_FLAG([-std=c++0x], [CXXFLAGS="$CXXFLAGS -std=c++0x"], AC_MSG_ERROR([Need C++11 support]))

AC_PROG_CXXCPP
AC_PROG_CXX

AC_PROG_CC

# automake initialization (mandatory) including a check for automake API version >= 1.9
AM_INIT_AUTOMAKE([1.9])


# use the C++ compiler for the following checks
AC_LANG([C++])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([assert.h])
AC_CHECK_HEADERS([cmath])
AC_CHECK_HEADERS([ctime])
AC_CHECK_HEADERS([fstream])
AC_CHECK_HEADERS([functional])
AC_CHECK_HEADERS([iostream])
AC_CHECK_HEADERS([limits])
AC_CHECK_HEADERS([malloc.h])
AC_CHECK_HEADERS([math.h])
AC_CHECK_HEADERS([regex])
AC_CHECK_HEADERS([set])
AC_CHECK_HEADERS([sstream])
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([stdio.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([tr1/unordered_map])
AC_CHECK_HEADERS([tr1/unordered_set])
AC_CHECK_HEADERS([tuple])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([vector])


# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# distribute additional compiler and linker flags among Makefiles
# --> set and change these variables instead of CXXFLAGS or LDFLAGS (for user only)
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([AM_LDFLAGS])

# files to generate via autotools (prepare .am or .in source files)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])

# finally this generates the Makefiles etc. for the build
AC_OUTPUT

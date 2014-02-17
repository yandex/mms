AC_PREREQ([2.60])
AC_INIT([mms], [0.18.0], [dprokoptsev@yandex-team.ru])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([include/mms/writer.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# TODO: find out how to make automake accept header-only libraries
AM_CONDITIONAL([am__EXEEXT], [true])

AC_LANG_CPLUSPLUS

AC_PROG_CXX
AM_PROG_AR
AC_PROG_LIBTOOL

AX_BOOST_BASE([1.40], [mms_have_boost=yes], [
    AC_MSG_WARN([no boost found on your system; unit tests will be unavailable.])
])
AX_BOOST_UNIT_TEST_FRAMEWORK

AC_CACHE_CHECK([whether compiler supports C++11], [mms_cv_cxx11_supported],[
    mms_saved_CXXFLAGS="$CXXFLAGS"
    CXXFLAGS="$CXXFLAGS -std=c++0x"
    AC_TRY_COMPILE([

        #include <type_traits>

        // Variable templates
        template<class... Args> class Foo {};
        Foo<int, char, double> g_foo;

        struct Nontrivial {
            Nontrivial(const Nontrivial&);
        };

        static_assert( std::is_trivial<int>::value, "int should be trivial" );
        static_assert( !std::is_trivial<Nontrivial>::value, "Nontrivial should not be trivial" );

    ], [], [mms_cv_cxx11_supported=yes], [ mms_cv_cxx11_supported=no ])
    CXXFLAGS="$mms_saved_CXXFLAGS"
])

if test "x$mms_cv_cxx11_supported" != xyes; then
    AC_MSG_WARN([your compiler does not support C++11; unit tests will be unavailable.])
fi

AM_CONDITIONAL([ENABLE_UNITTESTS], [\
    test "x$mms_have_boost" = xyes \
    && test "x$ax_cv_boost_unit_test_framework" = xyes \
    && test "x$mms_cv_cxx11_supported" = xyes \
])

AC_CONFIG_FILES([
    Makefile
    tests/Makefile
    include/Makefile
])
AC_OUTPUT

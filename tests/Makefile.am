if ENABLE_UNITTESTS

test_sources = \
	mms_cast_test.cpp \
	mms_diff_test.cpp \
	mms_hash_test.cpp \
	mms_map_test.cpp \
	mms_move_cr_test.cpp \
	mms_optional_test.cpp \
	mms_ptr_test.cpp \
	mms_set_test.cpp \
	mms_string_comparator_test.cpp \
	mms_string_test.cpp \
	mms_struct_test.cpp \
	mms_test.cpp \
	mms_transient_test.cpp \
	mms_vector_test.cpp \
	mms_version_test.cpp \
	mms_test_main.cpp \
	\
	align_tools.h \
	ptr_recursive.h	\
	test_config.h \
	tools.h


check_PROGRAMS = mms_test_cxx11 mms_test_boost

mms_test_cxx11_CXXFLAGS = -std=c++0x -DMMS_TEST_CXX11 -I$(top_srcdir)/include
mms_test_cxx11_SOURCES = $(test_sources)
mms_test_cxx11_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

mms_test_boost_CXXFLAGS = -DMMS_TEST_BOOST -I$(top_srcdir)/include
mms_test_boost_SOURCES = $(test_sources)
mms_test_boost_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

TESTS = mms_test_cxx11 mms_test_boost

else

TESTS = stub_test.sh

endif

EXTRA_DIST = stub_test.sh

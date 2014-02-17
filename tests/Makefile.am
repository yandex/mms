if ENABLE_UNITTESTS

check_PROGRAMS = mms_test_cxx11 mms_test_boost

mms_test_cxx11_CXXFLAGS = -std=c++0x -DMMS_TEST_CXX11
mms_test_cxx11_SOURCES = mms_test.cpp
mms_test_cxx11_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

mms_test_boost_CXXFLAGS = -DMMS_TEST_BOOST
mms_test_boost_SOURCES = mms_test.cpp
mms_test_boost_LDADD = $(BOOST_UNIT_TEST_FRAMEWORK_LIB)

TESTS = mms_test_cxx11 mms_test_boost

else

TESTS = stub_test.sh

endif

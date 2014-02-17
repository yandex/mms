/*
 * Copyright (c) 2011-2014 Dmitry Prokoptsev <dprokoptsev@yandex-team.ru>
 *
 * This file is part of mms, the memory-mapped storage library.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "test_config.h"

#include <mms/cast.h>
#include <mms/writer.h>
#include <mms/vector.h>
#include <mms/map.h>
#include <mms/set.h>
#include <mms/string.h>
#include <mms/type_traits.h>


#include <boost/test/unit_test.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/assign/std/vector.hpp>

#include <iostream>
#include <sstream>
#include <fstream>

using boost::assign::operator +=;

template<class P>
struct UnversionedClass {
    typedef UnversionedClass<mms::Mmapped> MmappedType;

    UnversionedClass()
    {}

    template<class A>
    void traverseFields(A a) const
    {
        a(i)(v)(b);
    }


    int i;
    mms::vector<P, int> v;
    bool b;
};

template<class P, size_t Version>
struct VersionedClass {
    //Version is calculated by MmappedType
    typedef VersionedClass<mms::Mmapped, Version> MmappedType;

    VersionedClass()
    {}

    //so format version is actual here
    static mms::FormatVersion formatVersion() { return Version; }

    template<class A>
    void traverseFields(A a) const
    {
        a(i)(v)(b);
    }

    int i;
    mms::vector<P, int> v;
    bool b;
};

//Derived class, methods of this class has another type, than VersionedClass
template <size_t V>
struct MutableVersionedClass : public VersionedClass<mms::Standalone, V>
{
};

template<class T>
std::string makeTest(T* object)
{
    object->i = 7;
    object->v += 1, 5, 4, 3;
    object->b = true;

    std::ostringstream oss;
    mms::write(oss, *object);
    return oss.str();
}

template<class S, class M>
void checkMmapped(const S& standalone, const M& mmapped)
{
    BOOST_CHECK_EQUAL(standalone.i, mmapped.i);
    BOOST_CHECK_EQUAL_COLLECTIONS(standalone.v.begin(), standalone.v.end(),
        mmapped.v.begin(), mmapped.v.end());
    BOOST_CHECK_EQUAL(standalone.b, mmapped.b);
}

BOOST_AUTO_TEST_CASE( unversioned_test )
{
    UnversionedClass<mms::Standalone> object;
    const std::string string =
        makeTest< UnversionedClass<mms::Standalone> >(&object);

    UnversionedClass<mms::Mmapped> mmappedObject =
        mms::cast< UnversionedClass<mms::Mmapped> >(
            string.c_str(), string.size());

    checkMmapped(object, mmappedObject);
}


BOOST_AUTO_TEST_CASE( versioned_test )
{
    VersionedClass<mms::Standalone, 1> object;
    const std::string string = makeTest(&object);

    BOOST_CHECK_THROW((mms::safeCast< VersionedClass<mms::Mmapped, 1> >(
        string.c_str(),
        sizeof(VersionedClass<mms::Mmapped, 1>) - 1)),
        std::length_error);

    BOOST_CHECK_NO_THROW((mms::safeCast< VersionedClass<mms::Mmapped, 1> >(
        string.c_str(), string.size())));

    BOOST_CHECK_THROW((mms::safeCast< VersionedClass<mms::Mmapped, 2> >(
        string.c_str(), string.size())),
        std::exception);
    BOOST_CHECK_THROW((mms::safeCast< VersionedClass<mms::Mmapped, 43> >(
        string.c_str(), string.size())),
        std::exception);

    const VersionedClass<mms::Mmapped, 1> mmappedObject =
        mms::safeCast< VersionedClass<mms::Mmapped, 1> >(
            string.c_str(), string.size());

    checkMmapped(object, mmappedObject);
    checkMmapped(object,
            mms::cast<VersionedClass<mms::Mmapped, 1> >(
                string.c_str(), string.size()));
}

BOOST_AUTO_TEST_CASE( derived_versioned_test )
{
    MutableVersionedClass<1> object;
    const std::string string =  makeTest(&object);

    BOOST_CHECK_NO_THROW((mms::safeCast< VersionedClass<mms::Mmapped, 1> >(
        string.c_str(), string.size())));
}

BOOST_AUTO_TEST_CASE( primitives_version_test )
{
    {
        std::ostringstream oss;
        int x = 15;
        mms::safeWrite(oss, x);
        BOOST_CHECK_EQUAL(mms::safeCast<int>(oss.str().c_str(),
                    oss.str().size()), 15);
        BOOST_CHECK_THROW(mms::safeCast<double>(oss.str().c_str(),
                    oss.str().size()), std::exception);
    }
    {
        std::ostringstream oss;
        double x = 15.0;
        mms::safeWrite(oss, x);
        BOOST_CHECK_EQUAL(mms::safeCast<double>(oss.str().c_str(),
                    oss.str().size()), 15.0);
        BOOST_CHECK_THROW(mms::safeCast<int>(oss.str().c_str(),
                    oss.str().size()), std::exception);
    }
}

BOOST_AUTO_TEST_CASE( container_version_test )
{
    {
        std::ostringstream oss;
        typedef mms::vector<mms::Standalone, int> Vector;
        Vector x;
        mms::safeWrite(oss, x);
        BOOST_CHECK_NO_THROW(mms::safeCast<mms::MmappedType<Vector>::type>(
                    oss.str().c_str(), oss.str().size()));

        BOOST_CHECK_THROW(
                (mms::safeCast<mms::vector<mms::Mmapped, double> >(
                    oss.str().c_str(), oss.str().size())), std::exception);
        BOOST_CHECK_THROW(
                (mms::safeCast<mms::string<mms::Mmapped> >(
                    oss.str().c_str(), oss.str().size())), std::exception);
    }
    {
        std::ostringstream oss;
        typedef mms::vector<mms::Standalone,
                VersionedClass<mms::Standalone, 15> > Vector;
        Vector x;
        mms::safeWrite(oss, x);
        BOOST_CHECK_NO_THROW(mms::safeCast<mms::MmappedType<Vector>::type>(
                    oss.str().c_str(), oss.str().size()));

        //Inner class in vector change its version, but typeid of vector
        //is not changed
        typedef mms::vector<mms::Standalone,
                VersionedClass<mms::Standalone, 12> > VectorAnotherVersion;
        BOOST_CHECK_THROW(
                (mms::safeCast<mms::MmappedType<VectorAnotherVersion>::type >(
                    oss.str().c_str(), oss.str().size())), std::exception);
    }
    {
        std::ostringstream oss;
        typedef mms::set<mms::Standalone,
                VersionedClass<mms::Standalone, 15> > Vector;
        Vector x;
        mms::safeWrite(oss, x);
        BOOST_CHECK_NO_THROW(mms::safeCast<mms::MmappedType<Vector>::type>(
                    oss.str().c_str(), oss.str().size()));

        //Inner class in vector change its version, but typeid of vector
        //is not changed
        typedef mms::set<mms::Standalone,
                VersionedClass<mms::Standalone, 12> > VectorAnotherVersion;
        BOOST_CHECK_THROW(
                (mms::safeCast<mms::MmappedType<VectorAnotherVersion>::type >(
                    oss.str().c_str(), oss.str().size())), std::exception);
    }
    {
        std::ostringstream oss;
        typedef mms::map<mms::Standalone, int,
                VersionedClass<mms::Standalone, 15> > Vector;
        Vector x;
        mms::safeWrite(oss, x);
        BOOST_CHECK_NO_THROW(mms::safeCast<mms::MmappedType<Vector>::type>(
                    oss.str().c_str(), oss.str().size()));

        //Inner class in vector change its version, but typeid of vector
        //is not changed
        typedef mms::map<mms::Standalone, int,
                VersionedClass<mms::Standalone, 12> > VectorAnotherVersion;
        BOOST_CHECK_THROW(
                (mms::safeCast<mms::MmappedType<VectorAnotherVersion>::type >(
                    oss.str().c_str(), oss.str().size())), std::exception);
    }
}



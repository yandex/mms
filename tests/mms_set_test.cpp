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

#include <boost/test/unit_test.hpp>


#include "tools.h"
#include <mms/set.h>

#include <iostream>

namespace mms {
template <class T1, class T2, class Cmp1, template<class> class Cmp2>
bool operator == (const std::set<T1, Cmp1>& l,
        const mms::set<mms::Mmapped, T2, Cmp2>& r)
{
    if (r.size() != l.size()) return false;
    return std::equal(l.begin(), l.end(), r.begin());
}
}//namespace mms

template <class T>
void checkPrimitiveSet(const std::set<T>& v)
{
    BOOST_CHECK(v == mmappedSet(v));
}

BOOST_AUTO_TEST_CASE( set_primitive_test  )
{
    size_t MAX_SIZE = 1000;
    for(size_t size = 0; size <= MAX_SIZE; ++size) {
        checkPrimitiveSet(genSet<int>(12, size));
        checkPrimitiveSet(genSet<char>(12, size));
        checkPrimitiveSet(genSet<bool>(12, size));
        checkPrimitiveSet(genSet<size_t>(12, size));
        checkPrimitiveSet(genSet<uint16_t>(12, size));
        checkPrimitiveSet(genSet<size_t>(12, size));
    }
    BOOST_CHECK((std::set<int>()) == (mms::set<mms::Mmapped, int>()));
}

template <class T>
void checkPrimitiveSetAlign(const std::set<T>& v)
{
    std::stringstream out;
    mms::Writer w(out);
    mms::set<mms::Standalone, T> mmsVector(v);
    size_t ofs = mms::unsafeWrite(w, mmsVector);
    std::string buf = out.str();
    mms::set<mms::Mmapped, T> mappedSet =
        *reinterpret_cast<const mms::set<mms::Mmapped, T> *>(
                buf.c_str() + ofs);
    BOOST_CHECK(ofs % sizeof(void*) == 0);
    BOOST_CHECK(isAligned(mappedSet));
    BOOST_CHECK_EQUAL(w.pos(),
            alignedSize(sizeof(T) * v.size())
            + 2 * sizeof(size_t) //reference size
    );
}

BOOST_AUTO_TEST_CASE( align_set_test  )
{
    size_t MAX_SIZE = 1000;
    for(size_t size = 0; size <= MAX_SIZE; ++size) {
        checkPrimitiveSetAlign(genSet<int>(12, size));
        checkPrimitiveSetAlign(genSet<char>(12, size));
        checkPrimitiveSetAlign(genSet<bool>(12, size));
        checkPrimitiveSetAlign(genSet<size_t>(12, size));
        checkPrimitiveSetAlign(genSet<uint16_t>(12, size));
        checkPrimitiveSetAlign(genSet<size_t>(12, size));
    }
}

namespace {

std::string buildElement(size_t i, size_t j, size_t size)
{
    std::string result;
    result.reserve(size);
    for(size_t t = 0; t < size; ++t) {
        result.push_back('a' + i + j + t);
    }
    return result;
}

template<class Matrix, class Vector>
Matrix buildMatrix(size_t size)
{
    Matrix matrix;
    for(size_t i = 0; i < size; ++i) {
        Vector vector;
        for(size_t j = 0; j < size; ++j) {
            vector.insert(buildElement(i, j, size));
        }
        matrix.insert(vector);
    }
    return matrix;
}

} // namespace

BOOST_AUTO_TEST_CASE( align_inner_set_test  )
{
    typedef std::set<std::string> StdVector;
    typedef std::set<StdVector,
            LexicographicalCompare<StdVector> > StdMatrix;

    typedef mms::string<mms::Standalone> StandaloneString;
    typedef mms::set<mms::Standalone, StandaloneString>
        StandaloneVector;
    typedef mms::set<mms::Standalone, StandaloneVector,
            LexicographicalCompare> StandaloneMatrix;

    typedef mms::string<mms::Mmapped> MmappedString;
    typedef mms::set<mms::Mmapped, MmappedString>
        MmappedVector;
    typedef mms::set<mms::Mmapped, MmappedVector,
            LexicographicalCompare> MmappedMatrix;

    const size_t size = 26;

    StdMatrix stdMatrix = buildMatrix<StdMatrix, StdVector>(size);
    StandaloneMatrix standaloneMatrix =
        buildMatrix<StandaloneMatrix, StandaloneVector>(size);

    std::stringstream out;
    mms::Writer w(out);
    size_t ofs = mms::unsafeWrite(w, standaloneMatrix);
    std::string buf = out.str();
    MmappedMatrix mmappedMatrix =
        *reinterpret_cast<const MmappedMatrix*>(buf.c_str() + ofs);
    BOOST_CHECK(ofs % sizeof(void*) == 0);
    BOOST_CHECK(isAligned(mmappedMatrix));

    MmappedMatrix::const_iterator it = mmappedMatrix.begin();
    for(size_t i = 0; i < size; ++i, ++it) {
        BOOST_CHECK(it != mmappedMatrix.end());
        MmappedVector::const_iterator innerIt = it->begin();
        for(size_t j = 0; j < size; ++j, ++innerIt) {
            BOOST_CHECK(innerIt != it->end());
            BOOST_CHECK(it->find(*innerIt) == innerIt);
            BOOST_CHECK_EQUAL(*innerIt, buildElement(i, j, size) );
        }
        BOOST_CHECK(innerIt == it->end());
    }
    BOOST_CHECK(it == mmappedMatrix.end());
    BOOST_CHECK(stdMatrix == mmappedMatrix);

}

#ifdef MMS_USE_CXX11
BOOST_AUTO_TEST_CASE( set_initializer  )
{
    mms::set<mms::Standalone, int> m{1, 2, 3};
    BOOST_CHECK_EQUAL(m.size(), 3);
    BOOST_CHECK_EQUAL(m.count(1) + m.count(2) + m.count(3), 3);
}
#endif

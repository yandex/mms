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
#include <mms/vector.h>

#include <iostream>

namespace mms {
template <class T1, class T2>
bool operator == (const mms::vector<mms::Mmapped, T2>& lhs,
        const std::vector<T1>& rhs) {
    return rhs == lhs;
}

template <class T1, class T2>
bool operator == (const std::vector<T1>& lhs,
        const mms::vector<mms::Mmapped, T2>& rhs)
{
    if (rhs.size() != lhs.size()) return false;
    for(size_t i = 0; i < lhs.size(); ++i) {
        //test operator []
        if (!(lhs[i] == rhs[i])) return false;
    }
    return
        std::equal(lhs.begin(), lhs.end(), rhs.begin()) &&
        std::equal(rhs.rbegin(), rhs.rend(), lhs.rbegin());
}
}//namespace mms

template <class T>
void checkPrimitiveVector(const std::vector<T>& v)
{
    BOOST_CHECK(v == mmappedVector(v));
}

BOOST_AUTO_TEST_CASE( vector_primitive_test )
{
    size_t MAX_SIZE = 1000;
    for(size_t size = 0; size <= MAX_SIZE; ++size) {
        checkPrimitiveVector(genVector<int>(12, size));
        checkPrimitiveVector(genVector<char>(12, size));
        checkPrimitiveVector(genVector<bool>(12, size));
        checkPrimitiveVector(genVector<size_t>(12, size));
        checkPrimitiveVector(genVector<uint16_t>(12, size));
        checkPrimitiveVector(genVector<size_t>(12, size));
    }
    BOOST_CHECK((std::vector<int>()) == (mms::vector<mms::Mmapped, int>()));

    mms::vector<mms::Mmapped, int> v = mmappedVector(genVector<int>(12, 10));

    BOOST_CHECK_EQUAL(std::distance(v.begin(), v.end()), v.size());
    BOOST_CHECK_EQUAL(std::distance(v.rbegin(), v.rend()), v.size());
}

template <class T>
void checkPrimitiveVectorAlign(const std::vector<T>& v)
{
    std::stringstream out;
    mms::Writer w(out);
    mms::vector<mms::Standalone, T> mmsVector(v);
    size_t ofs = mms::unsafeWrite(w, mmsVector);
    std::string buf = out.str();
    mms::vector<mms::Mmapped, T> mmappedMatrix =
        *reinterpret_cast<const mms::vector<mms::Mmapped, T> *>(
                buf.c_str() + ofs);
    BOOST_CHECK(ofs % sizeof(void*) == 0);
    BOOST_CHECK(isAligned(mmappedMatrix));
    BOOST_CHECK_EQUAL(w.pos(),
            alignedSize(sizeof(T) * v.size())
            + 2 * sizeof(size_t) //reference size
    );
}

BOOST_AUTO_TEST_CASE( align_vector_test  )
{
    size_t MAX_SIZE = 1000;
    for(size_t size = 0; size <= MAX_SIZE; ++size) {
        checkPrimitiveVectorAlign(genVector<int>(12, size));
        checkPrimitiveVectorAlign(genVector<char>(12, size));
        checkPrimitiveVectorAlign(genVector<bool>(12, size));
        checkPrimitiveVectorAlign(genVector<size_t>(12, size));
        checkPrimitiveVectorAlign(genVector<uint16_t>(12, size));
        checkPrimitiveVectorAlign(genVector<size_t>(12, size));
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

template<class Matrix>
Matrix buildMatrix(size_t size)
{
    Matrix matrix;
    matrix.resize(size);
    for(size_t i = 0; i < size; ++i) {
        matrix[i].resize(size);
        for(size_t j = 0; j < size; ++j) {
            matrix[i][j] = buildElement(i, j, size);
        }
    }
    return matrix;
}

} // namespace

BOOST_AUTO_TEST_CASE( align_inner_vector_test  )
{
    typedef std::vector<std::string> StdVector;
    typedef std::vector<StdVector> StdMatrix;

    typedef mms::vector<mms::Standalone,
        mms::string<mms::Standalone> > StandaloneVector;
    typedef mms::vector<mms::Standalone, StandaloneVector> StandaloneMatrix;

    typedef mms::vector<mms::Mmapped,
        mms::string<mms::Mmapped> > MmappedVector;
    typedef mms::vector<mms::Mmapped, MmappedVector> MmappedMatrix;

    const size_t size = 26;
    StdMatrix stdMatrix = buildMatrix<StdMatrix>(size);
    StandaloneMatrix standaloneMatrix = buildMatrix<StandaloneMatrix>(size);

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
            BOOST_CHECK_EQUAL(mmappedMatrix[i][j], buildElement(i, j, size) );
            BOOST_CHECK_EQUAL(it->at(j), buildElement(i, j, size) );
            BOOST_CHECK_EQUAL(*innerIt, buildElement(i, j, size) );
        }
        BOOST_CHECK(innerIt == it->end());
    }
    BOOST_CHECK(it == mmappedMatrix.end());
    BOOST_CHECK(stdMatrix == mmappedMatrix);
}

struct TripleNotPod
{
    TripleNotPod() { }
    TripleNotPod(double) { }
    double x, y;
    char z, t;
    uint64_t u, v;

    template <class A> void traverseFields(A a) const
    {
        a(x)(y)(z)(t)(u)(v);
    }

    typedef TripleNotPod MmappedType;
};

BOOST_AUTO_TEST_CASE( vector_pair_size_test  )
{
    mms::vector<mms::Standalone, TripleNotPod> a;
    const size_t COUNT = 128;
    a.resize(COUNT);
    std::stringstream out;
    mms::write(out, a);
    size_t dataSize = sizeof(TripleNotPod) * COUNT;
    dataSize += (sizeof(size_t) - dataSize % sizeof(size_t)) % sizeof(size_t);
    dataSize += 2 * sizeof(size_t); //Reference
    dataSize += sizeof(size_t); //Version
    BOOST_CHECK_EQUAL(dataSize, out.str().size());
}

#ifdef MMS_USE_CXX11
BOOST_AUTO_TEST_CASE( vector_initializer  )
{
    mms::vector<mms::Standalone, int> m{1, 2, 3};
    BOOST_CHECK_EQUAL(m.size(), 3);
    BOOST_CHECK_EQUAL(m[0], 1);
    BOOST_CHECK_EQUAL(m[1], 2);
    BOOST_CHECK_EQUAL(m[2], 3);
}
#endif

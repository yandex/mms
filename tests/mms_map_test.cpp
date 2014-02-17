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
#include <mms/map.h>

#include <boost/foreach.hpp>

#include <iostream>

namespace mms {

template <class F1, class S1, class F2, class S2>
bool operator == (const std::pair<F1, S1>& l, const std::pair<F2, S2>& r)
{
    return l.first == r.first && l.second == r.second;
}

template <class K1, class V1, class K2, class V2, class Cmp1, template<class> class Cmp2>
bool operator == (
    const std::map<K1, V1, Cmp1>& lhs,
    const mms::map<mms::Mmapped, K2, V2, Cmp2>& rhs
)
{
    if (rhs.size() != lhs.size()) return false;
    typedef std::pair<K2, V2> MmapedValueType;
    typedef Cmp2<K2> MCmp;
    //Check mmaped container
    BOOST_FOREACH(const MmapedValueType& kv, rhs) {
        typedef typename mms::map<mms::Mmapped, K2, V2, Cmp2>::const_iterator
                MmappedIterator;

        BOOST_CHECK_EQUAL(rhs.count(kv.first), 1);
        BOOST_CHECK_EQUAL(rhs[kv.first], kv.second);
        MmappedIterator it = rhs.find(kv.first);
        BOOST_CHECK(!MCmp()(it->first, kv.first));
        BOOST_CHECK(!MCmp()(kv.first, it->first));
        BOOST_CHECK(it->second == kv.second);
        MmappedIterator lb = rhs.lower_bound(kv.first);
        MmappedIterator ub = rhs.upper_bound(kv.first);
        BOOST_CHECK(lb == it);
        BOOST_CHECK(ub == (it + 1));
        BOOST_CHECK(std::make_pair(lb, ub) == rhs.equal_range(kv.first));
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

}//namespace mms

template <class K, class V, class Cmp >
void checkPrimitiveMap(const std::map<K, V, Cmp>& m)
{
    BOOST_CHECK(m == mmappedMap(m));
}

BOOST_AUTO_TEST_CASE( map_primitive_test  )
{
    size_t MAX_SIZE = 1000;
    for(size_t size = 0; size <= MAX_SIZE; ++size) {
        checkPrimitiveMap(genMap<int, char>(12, size));
        checkPrimitiveMap(genMap<char, char>(12, size));
        checkPrimitiveMap(genMap<bool, char>(12, size));
        checkPrimitiveMap(genMap<size_t, bool>(12, size));
        checkPrimitiveMap(genMap<uint16_t, uint16_t>(12, size));
        checkPrimitiveMap(genMap<size_t, uint16_t>(12, size));
        checkPrimitiveMap(genMap<size_t, size_t>(12, size));
        checkPrimitiveMap(genMap<int, int>(12, size));
        checkPrimitiveMap(genMap<bool, int>(12, size));
        checkPrimitiveMap(genMap<char, int>(12, size));
    }
    BOOST_CHECK((std::map<int, int>()) ==
            (mms::map<mms::Mmapped, int, int>()));
}

template <class K, class V, class Cmp >
void checkPrimitiveMapAlign(const std::map<K, V, Cmp>& m)
{
    std::stringstream out;
    mms::Writer w(out);
    mms::map<mms::Standalone, K, V> mmsMap(m);
    size_t ofs = mms::unsafeWrite(w, mmsMap);
    std::string buf = out.str();
    mms::map<mms::Mmapped, K, V> mappedMap =
        *reinterpret_cast<const mms::map<mms::Mmapped, K, V> *>(
                buf.c_str() + ofs);
    BOOST_CHECK(ofs % sizeof(void*) == 0);
    BOOST_CHECK(isAligned(mappedMap));
    BOOST_CHECK_EQUAL(w.pos(),
            alignedSize(sizeof(std::pair<K, V>) * m.size())
            + 2 * sizeof(size_t) //reference size
    );
}

BOOST_AUTO_TEST_CASE( align_map_test  )
{
    size_t MAX_SIZE = 1000;
    for(size_t size = 0; size <= MAX_SIZE; ++size) {
        checkPrimitiveMapAlign(genMap<int, char>(12, size));
        checkPrimitiveMapAlign(genMap<char, char>(12, size));
        checkPrimitiveMapAlign(genMap<bool, char>(12, size));
        checkPrimitiveMapAlign(genMap<size_t, bool>(12, size));
        checkPrimitiveMapAlign(genMap<uint16_t, uint16_t>(12, size));
        checkPrimitiveMapAlign(genMap<size_t, uint16_t>(12, size));
        checkPrimitiveMapAlign(genMap<size_t, size_t>(12, size));
        checkPrimitiveMapAlign(genMap<int, int>(12, size));
        checkPrimitiveMapAlign(genMap<bool, int>(12, size));
        checkPrimitiveMapAlign(genMap<char, int>(12, size));
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
            vector[buildElement(i, j, size)] = buildElement(i, j, size);
        }
        matrix[vector] = buildElement(i, i, size);
    }
    return matrix;
}

} // namespace

BOOST_AUTO_TEST_CASE( align_inner_map_test  )
{
    typedef std::map<std::string, std::string> StdVector;
    typedef std::map<StdVector, std::string,
            LexicographicalCompare<StdVector> > StdMatrix;

    typedef mms::string<mms::Standalone> StandaloneString;
    typedef mms::map<mms::Standalone, StandaloneString, StandaloneString>
        StandaloneVector;
    typedef mms::map<mms::Standalone, StandaloneVector, StandaloneString,
            LexicographicalCompare> StandaloneMatrix;

    typedef mms::string<mms::Mmapped> MmappedString;
    typedef mms::map<mms::Mmapped, MmappedString, MmappedString>
        MmappedVector;
    typedef mms::map<mms::Mmapped, MmappedVector, MmappedString,
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
        BOOST_CHECK_EQUAL(it->second, buildElement(i, i, size) );
        MmappedVector::const_iterator innerIt = it->first.begin();
        for(size_t j = 0; j < size; ++j, ++innerIt) {
            BOOST_CHECK(innerIt != it->first.end());
            BOOST_CHECK_EQUAL(it->first[innerIt->first], innerIt->second);
            BOOST_CHECK_EQUAL(innerIt->first, buildElement(i, j, size) );
            BOOST_CHECK_EQUAL(innerIt->second, buildElement(i, j, size) );
        }
        BOOST_CHECK(innerIt == it->first.end());
    }
    BOOST_CHECK(it == mmappedMatrix.end());
    BOOST_CHECK(stdMatrix == mmappedMatrix);
}

#ifdef MMS_USE_CXX11
BOOST_AUTO_TEST_CASE( map_initializer  )
{
    mms::map<mms::Standalone, int, int> m{{1, 1}, {2, 2}};
    BOOST_CHECK_EQUAL(m[1], 1);
    BOOST_CHECK_EQUAL(m[2], 2);
}
#endif

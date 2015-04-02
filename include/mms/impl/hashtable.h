
/*
 * impl/hashtable.h -- a mmapped version of unordered containers
 *
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


#pragma once

#include "defs.h"
#include "container.h"
#include "../writer.h"

#include <functional>
#include <cstdlib>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mms {
namespace impl {

template<
    class Key,
    class Value,
    template<class> class Hash,      // conforming to 'size_t(const Key&)'
    template<class> class Eq,        // conforming to 'bool(const Key&, const Key&)'
    template<class> class SelectKey  // conforming to 'const Key&(const Value&)'
>
class Hashtable {
public:
    typedef Key key_type;
    typedef Value value_type;

    size_t bucket_count() const { return buckets_.size() - 1; }

    typedef const value_type* const_local_iterator;
    const_local_iterator begin(size_t bucket) const { return buckets_[bucket].template ptr<Value>(); }
    const_local_iterator end  (size_t bucket) const { return buckets_[bucket+1].template ptr<Value>(); }

    typedef const value_type* const_iterator;
    const_iterator begin() const
        { return buckets_.size() ? begin(0) : nullIterator(); }
    const_iterator end()   const
        { return buckets_.size() ? end(bucket_count()-1) : nullIterator(); }

    // Make stupid BOOST_FOREACH happy
    typedef const value_type* iterator;
    iterator begin() { return static_cast<const Hashtable*>(this)->begin(); }
    iterator end()   { return static_cast<const Hashtable*>(this)->end(); }

    size_t size() const { return end() - begin(); }
    bool empty() const { return end() == begin(); }

    const_iterator find(const Key& k) const
    {
        if (buckets_.size() == 0)
            return nullIterator();

        size_t h = Hash<Key>()(k) % bucket_count();
        for (const_local_iterator i = begin(h), ie = end(h); i != ie; ++i)
            if (Eq<Key>()(k, SelectKey<Value>()(*i)))
                return i;
        return end();
    }

    size_t count(const Key& k) const { return (find(k) != end()) ? 1 : 0; }

    std::pair<const_iterator, const_iterator> equal_range(const Key& k) const
    {
        const_iterator i = find(k);
        return (i != end()) ? std::make_pair(i, i+1) : std::make_pair(i, i);
    }

    template<class Writer, class Container>
    static size_t writeData(Writer& w, const Container& c)
    {
        if (c.empty()) {
            return w.pos();
        }

        const size_t bucketCount = c.bucket_count();
        std::vector<size_t> bucketSizes(bucketCount, 0);

        // Compute sizes of buckets.
        for (typename Container::const_iterator i = c.begin(), ie = c.end(); i != ie; ++i) {
            ++bucketSizes[hashVal(*i) % bucketCount];
        }

        // Compute starting position of each bucket.
        std::vector<size_t> bucketPositions(bucketCount);
        bucketPositions[0] = 0;
        std::partial_sum(
                bucketSizes.begin(), --bucketSizes.end(), ++bucketPositions.begin());

        // Store pointers to items in grouped by buckets.
        std::vector<const typename Container::value_type *> items(c.size(), 0);

        for (typename Container::const_iterator i = c.begin(), ie = c.end(); i != ie; ++i) {
            items[bucketPositions[hashVal(*i) % bucketCount]++] = &*i;
        }

        Offsets ofs;

        // Write data for items.
        for (size_t i = 0, ie = items.size(); i != ie; ++i) {
            impl::writeData(w, *items[i], OfsPopulateIter(ofs));
        }

        // Reset bucket positions and add a guard at the end.
        bucketPositions[0] = 0;
        std::partial_sum(
                bucketSizes.begin(), --bucketSizes.end(), ++bucketPositions.begin());
        bucketPositions.push_back(c.size());

        // Write fields for items and store offsets at the beginnin of each bucket.
        std::vector<size_t> bucketOffsets;
        bucketOffsets.reserve(bucketCount + 1);

        align(w);

        for (size_t i = 0, ie = items.size(), bucket = 0; i != ie; ++i) {
            while (i == bucketPositions[bucket]) {
                bucketOffsets.push_back(w.pos());
                ++bucket;
            }
            impl::writeField(w, *items[i], OfsConsumeIter(ofs));
        }
        // Write offsets for empty buckets at the end.
        while (bucketOffsets.size() != bucketCount + 1) {
            bucketOffsets.push_back(w.pos());
        }
        align(w);

        // Write offsets of buckets
        size_t pos = w.pos();
        for (size_t i = 0, ie = bucketOffsets.size(); i != ie; ++i) {
            writeOffset(w, bucketOffsets[i]);
        }
        return pos;
    }

    template<class Writer, class Container>
    static size_t writeField(Writer& w, const Container& c, size_t dataPos)
    {
        return writeRef(w, dataPos, c.empty() ? 0 : c.bucket_count() + 1);
    }

    size_t parasiteLoad() const {
        return sizeof(Offset)*(buckets_.end() - buckets_.begin());
    }

private:
    Sequence<Offset> buckets_;

    template<class K>
    static size_t hashKey(const K& k) { return Hash<K>()(k); }

    template<class V>
    static size_t hashVal(const V& v) { return hashKey(SelectKey<V>()(v)); }

    static iterator nullIterator() { return static_cast<iterator>(0); }
};

} // namespace impl
} // namespace mms

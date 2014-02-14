// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

#include "defs.h"
#include "container.h"
#include "../writer.h"

#include <functional>
#include <cstdlib>
#include <stdexcept>
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
    const_local_iterator begin(size_t bucket) const { return buckets_[bucket].ptr<Value>(); }
    const_local_iterator end  (size_t bucket) const { return buckets_[bucket+1].ptr<Value>(); }

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
        size_t bktCount = c.bucket_count();
        Offsets ofs;
        for (size_t bkt = 0; bkt != bktCount; ++bkt)
            for (typename Container::const_local_iterator i = c.begin(bkt), ie = c.end(bkt); i != ie; ++i) {
                if (hashVal(*i) % bktCount != bkt)
                    throw std::logic_error("Bucket index mismatch while building mms::hashtable");
                impl::writeData(w, *i, OfsPopulateIter(ofs));
            }

        std::vector<size_t> buckets;
        align(w);
        for (size_t bkt = 0; bkt != bktCount; ++bkt) {
            buckets.push_back(w.pos());
            for (typename Container::const_local_iterator i = c.begin(bkt), ie = c.end(bkt); i != ie; ++i)
                impl::writeField(w, *i, OfsConsumeIter(ofs));
        }
        buckets.push_back(w.pos());
        align(w);

        size_t pos = w.pos();
        for (std::vector<size_t>::iterator i = buckets.begin(), ie = buckets.end(); i != ie; ++i)
            writeOffset(w, *i);
        return pos;
    }

    template<class Writer, class Container>
    static size_t writeField(Writer& w, const Container& c, size_t dataPos)
    {
        return writeRef(w, dataPos, c.bucket_count() + 1);
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


/*
 * impl/container.h -- a base for (almost) all mms containers
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

#include <stdlib.h>

#include <algorithm>
#include <iterator>
#include <stdexcept>

struct TestAccess;

namespace mms { namespace impl {

/// A class providing support for relative pointers.
class Offset
{
public:
    Offset(): offset_(0) {}

    explicit Offset(const void* ptr)
        :offset_(offset(ptr)) {}

    Offset(const Offset& c)
        :offset_(c.isNull() ? 0 : offset(c.ptr<void>())) {}

    Offset& operator = (const Offset& c) {
        offset_ = c.isNull() ? 0 : offset(c.ptr<void>());
        return *this;
    }

    template<class T>
    const T* ptr() const { return ptr<T>(offset_); }

    bool isNull() const { return offset_ == 0; }

protected:
    ssize_t offset_;

    template<class T>
    const T* ptr(const ssize_t& offset) const
    {
        return reinterpret_cast<const T*>(
            reinterpret_cast<const char*>(&offset) + offset
        );
    }

    ssize_t offset(const void* ptr) const
    {
        return reinterpret_cast<const char*>(ptr) -
               reinterpret_cast<const char*>(this);
    }

    friend struct ::TestAccess;
};

/**
 * A base class for all mmapped containers which stores their elements
 * in a contiguous region of file.
 *
 * Provides support for relative pointers and more convinient access
 * to stored size and beginning ot elements.
 */
class Container
{
public:
    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

protected:
    Container(): ofs_(), size_(0) {}

    Container(const void* ptr, size_t size)
        :ofs_(ptr), size_(size) {}

    template<class T>
    const T* ptr() const { return ofs_.ptr<T>(); }

private:
    Offset ofs_;
    size_t size_;

    friend struct ::TestAccess;
};

/**
 * A base class for sequence-based containers.
 *
 * Provides basic accessors (begin(), end(), operator[], etc...)
 * which are to be provided both by vector and string.
 */
template <class T>
class Sequence: public Container
{
public:
    typedef T value_type;
    typedef const value_type* const_iterator;
    typedef const value_type* iterator; // make stupid BOOST_FOREACH happy
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    Sequence(): Container() {}
    Sequence(const void* ptr, size_t size): Container(ptr, size) {}

    const_iterator begin() const { return ptr<value_type>(); }
    const_iterator end() const   { return begin() + size(); }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }

    const value_type& front() const { return *begin(); }
    const value_type& back() const { return *(end() - 1); }

    const value_type& operator[] (size_t n) const { return *(begin() + n); }

    const value_type& at(size_t n) const
    {
        if (n >= size()) {
            throw std::out_of_range("mms::Sequence::at");
        }
        return (*this)[n];
    }
};

template < class Key, class Value, class Cmp, class SelectKey >
struct VersatileCmp {
    bool operator()(const Key& a, const Key& b) const
        { return Cmp()(a, b); }
    bool operator()(const Key& a, const Value& b) const
        { return Cmp()(a, SelectKey()(b)); }
    bool operator()(const Value& a, const Key& b) const
        { return Cmp()(SelectKey()(a), b); }
    bool operator()(const Value& a, const Value& b) const
        { return Cmp()(SelectKey()(a), SelectKey()(b)); }
};

template < class Key, class Cmp, class SelectKey >
struct VersatileCmp<Key, Key, Cmp, SelectKey> {
    bool operator()(const Key& a, const Key& b) const
        { return Cmp()(a, b); }
};

/**
 * A base class for associative containers (map, set...)
 *
 * Stores elements in a sorted continuous region, utilizing
 * binary search for finding elements.
 *
 * NB: since functors are not stored, those must be stateless.
 */
template <
    class Key,
    class Value,
    class Cmp, // conforming to "bool less(const key_type&, const key_type&)"
    class SelectKey   // conforming to "key_type key(const value_type&)"
>
class SortedSequence : public Container
{
    typedef impl::VersatileCmp<Key, Value, Cmp, SelectKey> VersatileCmp;
public:
    typedef Key key_type;
    typedef Value value_type;
    // note: no 'mapped_type' here
    typedef const value_type* const_iterator;
    typedef const_iterator iterator; // make stupid BOOST_FOREACH happy

    SortedSequence(): Container() {}
    SortedSequence(const void* ptr, size_t size): Container(ptr, size) {}

    const_iterator begin() const { return ptr<value_type>(); }
    const_iterator end() const   { return begin() + size(); }

    const_iterator find(const key_type& key) const
    {
        std::pair<const_iterator, const_iterator> r = equal_range(key);
        return r.first != r.second ? r.first : end();
    }

    const_iterator lower_bound(const key_type& key) const
    {
        return std::lower_bound(begin(), end(), key, VersatileCmp());
    }

    const_iterator upper_bound(const key_type& key) const
    {
        return std::upper_bound(begin(), end(), key, VersatileCmp());
    }

    std::pair<const_iterator, const_iterator>
    equal_range(const key_type& key) const
    {
        return std::equal_range(begin(), end(), key, VersatileCmp());
    }

    size_t count(const key_type& key) const
    {
        std::pair<const_iterator, const_iterator> range = equal_range(key);
        return std::distance(range.first, range.second);
    }
};

} // namespace impl

template<class T>
bool operator == (const impl::Sequence<T>& a, const impl::Sequence<T>& b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template<class T>
bool operator != (const impl::Sequence<T>& a, const impl::Sequence<T>& b)
{
    return !(a == b);;
}

template<class K, class V, class C, class S>
bool operator == (const impl::SortedSequence<K,V,C,S>& a, const impl::SortedSequence<K,V,C,S>& b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}


template<class K, class V, class C, class S>
bool operator != (const impl::SortedSequence<K,V,C,S>& a, const impl::SortedSequence<K,V,C,S>& b)
{
    return !(a == b);
}

} // namespace mms

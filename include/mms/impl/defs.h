// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

#include "config.h"
#include "front_remove_iterator.h"
#include <deque>
#include <utility>

namespace mms {

namespace impl {

template <class T>
struct FieldDescriptor {
    FieldDescriptor(const char* name, T* object) :
        name(name), object(object)
    {}

    const char* const name;
    T* const object;
};

template <class T>
inline FieldDescriptor<T> makeFieldDescriptor(const char *name, T* object) {
    return FieldDescriptor<T>(name, object);
}

typedef std::pair<size_t, const void*> WriterID;

class WriterBase {
public:
    explicit WriterBase(size_t pos = 0, size_t transientPos = 0):
        pos_(pos), transientPos_(transientPos), id_(nextId())
    {}

    size_t pos() const { return pos_; }

    void putTransient(size_t size) { transientPos_ += size; }
    size_t transientPos() const { return transientPos_; }

    // Returns a unique ID for writer.
    // NB: using of both sequential number and address seems to eliminate
    //     multithreading problems even without any atomics.
    WriterID id() const { return WriterID(id_, this); }

protected:
    void advance(size_t size) { pos_ += size; }

private:
    size_t pos_;
    size_t transientPos_;
    size_t id_;

    static size_t nextId()
    {
        static size_t id = 0;
        return ++id;
    }

    WriterBase(const WriterBase&) /* = delete */;
    WriterBase& operator = (const WriterBase&) /* = delete */;
};

typedef std::deque<size_t> Offsets;

typedef std::back_insert_iterator <Offsets> OfsPopulateIter;
typedef      front_remove_iterator<Offsets> OfsConsumeIter;


template<class T>
struct Identity: public std::unary_function<T, T> {
    const T& operator()(const T& t) const { return t; }
};

template<class T> struct Select1st;

template <class K, class V>
struct Select1st< std::pair<K, V> >: public std::unary_function<std::pair<K, V>, K> {
    const K& operator ()(const std::pair<K, V>& p) const
        { return p.first; }
};

}} // namespace mms::impl

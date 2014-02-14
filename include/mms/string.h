// mms -- a memory-mapped storage.
//
// See https://github.com/dprokoptsev/mms/wiki/Library-usage
// for short instruction about its usage.

#pragma once

#include "writer.h"
#include "version.h"
#include "impl/tags.h"
#include "impl/container.h"
#include "impl/fwd.h"

#include <iostream>
#include <string>

#include <cstring>

namespace mms {


template<>
class string<Mmapped>: public impl::Sequence<char>
{
public:
    static FormatVersion formatVersion(Versions& vs)
        { return vs.hash("string"); }

    // Sequence<char>::size() stores length of string
    // (not including stored trailing zero)

    string(): impl::Sequence<char>(zero(), 0) {}
    /*implicit*/ string(const std::string& s)
        :impl::Sequence<char>(s.c_str(), s.size()) {}
    /*implicit*/ string(const char* str)
        :impl::Sequence<char>(str, strlen(str)) {}
    
    string(const char* str, size_t size): impl::Sequence<char>(str, size)
    {
        if (str[size] != 0)
            throw std::logic_error("the string has no trailing zero");
    }

    const char* c_str() const { return ptr<char>(); }
    size_t length() const { return size(); }

    typedef string<Mmapped> MmappedType;

private:
    static const char* zero() { static const char z = 0; return &z; }
};


template<>
class string<Standalone>: public std::string {
public:
    typedef string<Mmapped> MmappedType;

    string() {}
    string(const std::string& s): std::string(s) {}
    string(const string& s): std::string(s) {}

    string(const char* s): std::string(s) {}

    explicit string(const string<Mmapped>& s): std::string(s.begin(), s.end()) {}
    string<Standalone>& operator = (const string<Mmapped>& s)
    {
        assign(s.begin(), s.end());
        return *this;
    }

    string& operator = (const string& s)
        { std::string::operator=(s); return *this; }
    string& operator = (const std::string& s)
        { std::string::operator=(s); return *this; }
    string& operator = (const char* s)
        { std::string::operator=(s); return *this; }
    
#if MMS_USE_CXX11
    string(std::string&& s) { swap(s); }
    string(string&& s) { swap(s); }
    string& operator = (string&& s)
        { std::string::operator=(std::move(s)); return *this; }
#endif

    template<class Writer>
    size_t writeData(Writer& w) const
    {
        impl::align(w);
        size_t res = w.pos();
        w.write(c_str(), size() + 1); // +1 for trailing zero
        return res;
    }

    template<class Writer>
    size_t writeField(Writer& w, size_t pos) const
    {
        return impl::writeRef(w, pos, size());
    }
};


inline std::ostream& operator << (
        std::ostream& out,
        const string<Mmapped>& s)
{
    out.write(s.c_str(), s.size());
    return out;
}

// size of string is just size and offset
// base empty class optimization must work in this case
MMS_STATIC_ASSERT(sizeof(string<Mmapped>) == 2 * sizeof(size_t), "mms::string size mismatch");

namespace impl {

// A helper class which simplifies mms::string comparison with std::string,
// const char* and with each other.
class StringRef {
public:
    /*implicit*/ StringRef(const std::string& s)
        :ptr_(s.c_str()), len_(s.size()) {}
    /*implicit*/ StringRef(const mms::string<mms::Mmapped>& s)
        :ptr_(s.c_str()), len_(s.size()) {}
    /*implicit*/ StringRef(const char* s): ptr_(s), len_(strlen(s)) {}

    template<template<class> class Rel>
    static bool strcmp(const StringRef& a, const StringRef& b)
    {
        int memCmp = memcmp(a.ptr_, b.ptr_, std::min(a.len_, b.len_) + 1);
        return memCmp ? Rel<int>()(memCmp, 0) : Rel<size_t>()(a.len_, b.len_);
    }

private:
    const char* ptr_;
    size_t len_;
};

} // namespace impl;

// These operators must be in mms:: namespace
// (neither in mms::impl:: nor in ::)
// so that the argument-dependent lookup can find those
inline bool operator < (const impl::StringRef& a, const impl::StringRef& b)
    { return impl::StringRef::strcmp<std::less>(a, b); }
inline bool operator > (const impl::StringRef& a, const impl::StringRef& b)
    { return impl::StringRef::strcmp<std::greater>(a, b); }
inline bool operator <=(const impl::StringRef& a, const impl::StringRef& b)
    { return impl::StringRef::strcmp<std::less_equal>(a, b); }
inline bool operator >=(const impl::StringRef& a, const impl::StringRef& b)
    { return impl::StringRef::strcmp<std::greater_equal>(a, b); }
inline bool operator ==(const impl::StringRef& a, const impl::StringRef& b)
    { return impl::StringRef::strcmp<std::equal_to>(a, b); }
inline bool operator !=(const impl::StringRef& a, const impl::StringRef& b)
    { return impl::StringRef::strcmp<std::not_equal_to>(a, b); }


inline size_t hash_value(const string<Mmapped>& s)
    { return impl::hash_range(s.begin(), s.end()); }

} // namespace mms

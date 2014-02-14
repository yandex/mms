#pragma once

#include "writer.h"

#include <cstddef>//for size_t
#include <stdexcept>

namespace mms {

template <class T>
const T& unsafeCast(const char* buffer, size_t size) {
    return *reinterpret_cast<const T*>(buffer + size - sizeof(T));
}

template <class T>
const T& safeCast(const char* buffer, size_t size) {
    if (size < sizeof(T) + sizeof(FormatVersion)) {
        throw std::length_error(
            "Size of buffer is less than needed for format version and class"
            );
    }
    const T& object = unsafeCast<T>(buffer, size);
    impl::checkFormatVersion(
            object,
            unsafeCast<size_t>(buffer, size - sizeof(T))
        );
    return object;
}

template <class T>
const T& cast(const char* buffer, size_t size) {
    return unsafeCast<T>(buffer, size);
}

}//namespace mms

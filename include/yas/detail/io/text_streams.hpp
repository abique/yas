
// Copyright (c) 2010-2021 niXman (github dot nixman at pm dot me). All
// rights reserved.
//
// This file is part of YAS(https://github.com/niXman/yas) project.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef __yas__detail__io__text_streams_hpp
#define __yas__detail__io__text_streams_hpp

#include <yas/detail/config/config.hpp>

#include <yas/detail/io/io_exceptions.hpp>
#include <yas/detail/io/serialization_exceptions.hpp>
#include <yas/detail/type_traits/type_traits.hpp>
#include <yas/detail/tools/cast.hpp>
#include <yas/tools/wrap_asis.hpp>

#include <limits>

namespace yas {
namespace detail {

/***************************************************************************/

template<typename OS, std::size_t F, typename Trait>
struct text_ostream {
    text_ostream(OS &os)
        :os(os)
    {}

    template<typename T>
    void write_seq_size(T v) {
        write(__YAS_SCAST(std::uint64_t, v));
    }

    // for arrays
    template<typename T>
    void write(const T *ptr, std::size_t size) {
        __YAS_THROW_WRITE_ERROR(size != os.write(ptr, size));
    }

    template<typename T>
    void write(const asis_wrapper<T> &v) {
        write(v.val);
    }

    // for chars only
    template<typename T>
    void write(const T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, char, signed char, unsigned char)) {
        __YAS_THROW_WRITE_ERROR(sizeof(v) != os.write(&v, sizeof(v)));
    }

    // for bools only
    template<typename T>
    void write(const T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, bool)) {
        const char c = __YAS_SCAST(char, '0' + v);
        __YAS_THROW_WRITE_ERROR(sizeof(c) != os.write(&c, sizeof(c)));
    }

    // for signed 16/32/64 bits
    template<typename T>
    void write(const T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, std::int16_t, std::int32_t, std::int64_t)) {
        char buf[sizeof(v) * 4];
        std::size_t len = Trait::itoa(buf + 1, sizeof(buf) - 1, v);

        buf[0] = __YAS_SCAST(char, '0' + len);

        __YAS_THROW_WRITE_ERROR(len + 1 != os.write(buf, len + 1));
    }

    // for unsigned 16/32/64 bits
    template<typename T>
    void write(const T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, std::uint16_t, std::uint32_t, std::uint64_t)) {
        char buf[sizeof(v) * 4];
        std::size_t len = Trait::utoa(buf + 1, sizeof(buf), v);

        buf[0] = __YAS_SCAST(char, '0' + len);

        __YAS_THROW_WRITE_ERROR(len + 1 != os.write(buf, len + 1));
    }

    // for floats
    template<typename T>
    void write(const T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, float)) {
        char buf[std::numeric_limits<T>::max_exponent10 + 20];
        std::size_t len = Trait::ftoa(buf + 1, sizeof(buf) - 1, v);

        buf[0] = __YAS_SCAST(char, '0' + len);

        __YAS_THROW_WRITE_ERROR(len + 1 != os.write(buf, len + 1));
    }

    // for doubles
    template<typename T>
    void write(const T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, double)) {
        char buf[std::numeric_limits<T>::max_exponent10 + 20];
        std::size_t len = Trait::dtoa(buf + 1, sizeof(buf) - 1, v);

        buf[0] = __YAS_SCAST(char, '0' + len);

        __YAS_THROW_WRITE_ERROR(len + 1 != os.write(buf, len + 1));
    }

private:
    OS &os;
};

/***************************************************************************/

template<typename IS, std::size_t F, typename Trait>
struct text_istream {
    text_istream(IS &is)
        :is(is)
    {}

    std::size_t read_seq_size() {
        std::uint64_t size{};
        read(size);

        return __YAS_SCAST(std::size_t, size);
    }

    bool empty() const { return is.empty(); }
    char peekch() const { return is.peekch(); }
    char getch() { return is.getch(); }
    void ungetch(char ch) { is.ungetch(ch); }

    // for arrays
    std::size_t read(void *ptr, std::size_t size) {
        __YAS_THROW_READ_ERROR(size != is.read(ptr, size));

        return size;
    }

    template<typename T>
    void read(asis_wrapper<T> &v) {
        read(v.val);
    }

    // for chars only
    template<typename T>
    void read(T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, char, signed char, unsigned char)) {
        char buf;

        __YAS_THROW_READ_ERROR(sizeof(buf) != is.read(&buf, sizeof(buf)));

        v = __YAS_SCAST(T, buf);
    }

    // for bools only
    template<typename T>
    void read(T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, bool)) {
        char buf;

        __YAS_THROW_READ_ERROR(sizeof(buf) != is.read(&buf, sizeof(buf)));
        v = (buf - '0') != 0;
    }

    // for signed 16/32/64 bits
    template<typename T>
    void read(T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, std::int16_t, std::int32_t, std::int64_t)) {
        char buf[sizeof(T) * 4];
        std::uint8_t n;

        __YAS_THROW_READ_ERROR(sizeof(n) != is.read(&n, sizeof(n)));
        n = __YAS_SCAST(std::uint8_t, n - '0');

        buf[n] = 0;
        __YAS_THROW_READ_ERROR(n != is.read(buf, sizeof(buf) < n ? sizeof(buf) : n));

        v = Trait::template atoi<T>(buf, n);
    }

    // for unsigned 16/32/64 bits
    template<typename T>
    void read(T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, std::uint16_t, std::uint32_t, std::uint64_t)) {
        char buf[sizeof(T) * 4];
        std::uint8_t n;

        __YAS_THROW_READ_ERROR(sizeof(n) != is.read(&n, sizeof(n)));
        n = __YAS_SCAST(std::uint8_t, n - '0');

        buf[n] = 0;
        __YAS_THROW_READ_ERROR(n != is.read(buf, sizeof(buf) < n ? sizeof(buf) : n));

        v = Trait::template atou<T>(buf, n);
    }

    // for floats
    template<typename T>
    void read(T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, float)) {
        char buf[std::numeric_limits<T>::max_exponent10 + 20];
        std::uint8_t n;

        __YAS_THROW_READ_ERROR(sizeof(n) != is.read(&n, sizeof(n)));
        n = __YAS_SCAST(std::uint8_t, n - '0');

        buf[n] = 0;
        __YAS_THROW_READ_ERROR(n != is.read(buf, sizeof(buf) < n ? sizeof(buf) : n));

        v = Trait::template atof<T>(buf, n);
    }

    // for doubles
    template<typename T>
    void read(T &v, __YAS_ENABLE_IF_IS_ANY_OF(T, double)) {
        char buf[std::numeric_limits<T>::max_exponent10 + 20];
        std::uint8_t n;

        __YAS_THROW_READ_ERROR(sizeof(n) != is.read(&n, sizeof(n)));
        n = __YAS_SCAST(std::uint8_t, n - '0');

        buf[n] = 0;
        __YAS_THROW_READ_ERROR(n != is.read(buf, sizeof(buf) < n ? sizeof(buf) : n));

        v = Trait::template atod<T>(buf, n);
    }

private:
    IS &is;
};

/***************************************************************************/

} // ns detail
} // ns yas

#endif // __yas__detail__io__text_streams_hpp

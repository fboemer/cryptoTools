///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef GSL_UTIL_H
#define GSL_UTIL_H

#include "gsl_assert.h" // Ensures/Expects
#include <array>
#include <exception>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER

// No MSVC does constexpr fully yet
#pragma push_macro("constexpr")
#define constexpr /*constexpr*/

#pragma warning(push)
#pragma warning(disable : 4127) // conditional expression is constant

// MSVC 2013 workarounds
#if _MSC_VER <= 1800
// noexcept is not understood
#pragma push_macro("noexcept")
#define noexcept /*noexcept*/

// turn off some misguided warnings
#pragma warning(push)
#pragma warning(disable : 4351) // warns about newly introduced aggregate initializer behavior

#endif // _MSC_VER <= 1800

#endif // _MSC_VER

namespace gsl
{
//
// GSL.util: utilities
//

// final_act allows you to ensure something gets run at the end of a scope
template <class F>
class final_act
{
public:
    explicit final_act(F f) noexcept : f_(std::move(f)), invoke_(true) {}

    final_act(final_act&& other) noexcept : f_(std::move(other.f_)), invoke_(other.invoke_)
    {
        other.invoke_ = false;
    }

    final_act(const final_act&) = delete;
    final_act& operator=(const final_act&) = delete;

    ~final_act() noexcept
    {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_;
};

// finally() - convenience function to generate a final_act
template <class F>
inline final_act<F> finally(const F& f) noexcept
{
    return final_act<F>(f);
}

template <class F>
inline final_act<F> finally(F&& f) noexcept
{
    return final_act<F>(std::forward<F>(f));
}

// narrow_cast(): a searchable way to do narrowing casts of values
#if defined(_MSC_VER) && _MSC_VER <= 1800
template <class T, class U>
inline constexpr T narrow_cast(U u) noexcept
{
    return static_cast<T>(u);
}
#else
template <class T, class U>
inline constexpr T narrow_cast(U&& u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}
#endif

struct narrowing_error : public std::exception
{
};

namespace details
{
    template <class T, class U>
    struct is_same_signedness
        : public std::integral_constant<bool, std::is_signed<T>::value == std::is_signed<U>::value>
    {
    };
}

// narrow() : a checked version of narrow_cast() that throws if the cast changed the value
template <class T, class U>
inline T narrow(U u)
{
    T t = narrow_cast<T>(u);
    if (static_cast<U>(t) != u) throw narrowing_error();
    if (!details::is_same_signedness<T, U>::value && ((t < T{}) != (u < U{})))
        throw narrowing_error();
    return t;
}

//
// at() - Bounds-checked way of accessing static arrays, std::array, std::vector
//
template <class T, size_t N>
constexpr T& at(T (&arr)[N], std::ptrdiff_t index)
{
    Expects(index >= 0 && index < narrow_cast<std::ptrdiff_t>(N));
    return arr[static_cast<size_t>(index)];
}

template <class T, size_t N>
constexpr T& at(std::array<T, N>& arr, std::ptrdiff_t index)
{
    Expects(index >= 0 && index < narrow_cast<std::ptrdiff_t>(N));
    return arr[static_cast<size_t>(index)];
}

template <class Cont>
constexpr typename Cont::value_type& at(Cont& cont, std::ptrdiff_t index)
{
    Expects(index >= 0 && index < narrow_cast<std::ptrdiff_t>(cont.size()));
    return cont[static_cast<typename Cont::size_type>(index)];
}

template <class T>
constexpr const T& at(std::initializer_list<T> cont, std::ptrdiff_t index)
{
    Expects(index >= 0 && index < narrow_cast<std::ptrdiff_t>(cont.size()));
    return *(cont.begin() + index);
}

} // namespace gsl

#ifdef _MSC_VER

#pragma warning(pop)

#undef constexpr
#pragma pop_macro("constexpr")

#if _MSC_VER <= 1800

#undef noexcept
#pragma pop_macro("noexcept")

#pragma warning(pop)

#endif // _MSC_VER <= 1800

#endif // _MSC_VER

#endif // GSL_UTIL_H

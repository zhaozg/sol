// The MIT License (MIT)

// Copyright (c) 2013-2015 Danny Y., Rapptz

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_RESOLVE_HPP
#define SOL_RESOLVE_HPP

#include "traits.hpp"
#include "tuple.hpp"

namespace sol {
namespace detail {
template<typename R, typename... Args, typename F, typename = typename std::result_of<Unqualified<F>(Args...)>::type>
inline auto resolve_i(types<R(Args...)>, F&&) -> R(Unqualified<F>::*)(Args...) {
    using Sig = R(Args...);
    typedef Unqualified<F> Fu;
    return static_cast<Sig Fu::*>(&Fu::operator());
}

template<typename F, typename U = Unqualified<F>>
inline auto resolve_f(std::true_type, F&& f)
-> decltype(resolve_i(types<function_signature_t<decltype(&U::operator())>>(), std::forward<F>(f))) {
    return resolve_i(types<function_signature_t<decltype(&U::operator())>>(), std::forward<F>(f));
}

template<typename F>
inline void resolve_f(std::false_type, F&&) {
    static_assert(has_deducible_signature<F>::value,
                    "Cannot use no-template-parameter call with an overloaded functor: specify the signature");
}

template<typename F, typename U = Unqualified<F>>
inline auto resolve_i(types<>, F&& f) -> decltype(resolve_f(has_deducible_signature<U> {}, std::forward<F>(f))) {
    return resolve_f(has_deducible_signature<U> {}, std::forward<F>(f));
}

template<typename... Args, typename F, typename R = typename std::result_of<F&(Args...)>::type>
inline auto resolve_i(types<Args...>, F&& f) -> decltype( resolve_i(types<R(Args...)>(), std::forward<F>(f))) {
    return resolve_i(types<R(Args...)>(), std::forward<F>(f));
}

template<typename Sig, typename C>
inline Sig C::* resolve_v(std::false_type, Sig C::* mem_func_ptr) {
    return mem_func_ptr;
}

template<typename Sig, typename C>
inline Sig C::* resolve_v(std::true_type, Sig C::* mem_variable_ptr) {
    return mem_variable_ptr;
}
} // detail

template<typename... Args, typename R>
inline auto resolve(R fun_ptr(Args...)) -> R(*)(Args...) {
    return fun_ptr;
}

template<typename Sig>
inline Sig* resolve(Sig* fun_ptr) {
    return fun_ptr;
}

template<typename... Args, typename R, typename C>
inline auto resolve(R(C::*mem_ptr)(Args...)) -> R(C::*)(Args...) {
    return mem_ptr;
}

template<typename Sig, typename C>
inline Sig C::* resolve(Sig C::* mem_ptr) {
    return detail::resolve_v(std::is_member_object_pointer<Sig C::*>(), mem_ptr);
}

template<typename... Sig, typename F>
inline auto resolve(F&& f) -> decltype(detail::resolve_i(types<Sig...>(), std::forward<F>(f))) {
    return detail::resolve_i(types<Sig...>(), std::forward<F>(f));
}
} // sol

#endif // SOL_RESOLVE_HPP

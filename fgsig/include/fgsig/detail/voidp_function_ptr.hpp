//Copyright Florian Goujeon 2018 - 2019.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGSIG_DETAIL_VOIDP_FUNCTION_PTR_HPP
#define FGSIG_DETAIL_VOIDP_FUNCTION_PTR_HPP

namespace fgsig::detail
{

template<typename Signature>
struct voidp_function_ptr_helper;

template<typename R, typename... Params>
struct voidp_function_ptr_helper<R(Params...)>
{
    using type = R(*)(void*, Params...);
};

/*
Metafunction voidp_function_ptr returns a function pointer type whose signature
corresponds to the one given as template parameter, prepended with a void*.
*/
template<typename Signature>
using voidp_function_ptr = typename voidp_function_ptr_helper<Signature>::type;

} //namespace

#endif

//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGSIG_DETAIL_RAW_CLOSURE_HPP
#define FGSIG_DETAIL_RAW_CLOSURE_HPP

#include "voidp_function_ptr.hpp"
#include <list>

namespace fgsig::detail
{

template<typename Signature>
struct raw_closure
{
    raw_closure(const voidp_function_ptr<Signature> pf, void* const pvslot):
        pf(pf),
        pvslot(pvslot)
    {
    }

    voidp_function_ptr<Signature> pf;
    void* pvslot;
};

template<typename Signature>
using raw_closure_list = std::list<raw_closure<Signature>>;

template<typename Signature>
using raw_closure_id = typename raw_closure_list<Signature>::iterator;

} //namespace

#endif

//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGL_SIGNALS_OWNING_CONNECTION_HPP
#define FGL_SIGNALS_OWNING_CONNECTION_HPP

#include "connection.hpp"
#include <utility>

namespace fgl::signals
{

/*
owning_connection is a connection that owns the slot it connects to the signal.
*/
template<typename Signal, typename Slot>
struct owning_connection
{
    public:
        owning_connection(Signal& sig, Slot&& slot):
            slot_(std::move(slot)),
            connection_(sig, slot_)
        {
        }

        void close()
        {
            connection_.close();
        }

    private:
        Slot slot_;
        connection<Signal, Slot> connection_;
};

} //namespace

#endif

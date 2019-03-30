//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGSIG_OWNING_CONNECTION_HPP
#define FGSIG_OWNING_CONNECTION_HPP

#include "connection.hpp"
#include <utility>

namespace fgsig
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

        owning_connection(const owning_connection&) = delete;

        /*
        Move constructor:
        - Move slot
        - Connect signal to new slot
        - Disconnect signal from moved-from slot
        */
        owning_connection(owning_connection&& r):
            slot_(std::move(r.slot_)),
            connection_(*r.connection_.psignal_, slot_)
        {
            r.connection_.close();
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

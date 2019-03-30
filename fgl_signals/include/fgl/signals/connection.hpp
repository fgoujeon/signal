//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGL_SIGNALS_CONNECTION_HPP
#define FGL_SIGNALS_CONNECTION_HPP

#include "detail/raw_closure.hpp"
#include <tuple>
#include <utility>

namespace fgl::signals
{

namespace detail
{
    template<typename Slot, typename Signature>
    struct slot_caller;

    template<typename Slot, typename... Args>
    struct slot_caller<Slot, void(Args...)>
    {
        static void call(void* pvslot, Args... args)
        {
            auto& slot = *reinterpret_cast<Slot*>(pvslot);
            slot(std::forward<Args>(args)...);
        }
    };
}

/*
connection establishes a connection between the given signal and slot.
It doesn't own the given slot.
Its destructor closes the connection.
*/
template<typename Signal, typename Slot>
struct connection;

template<template<typename...> typename SignalTpl, typename Slot, typename... Signatures>
struct connection<SignalTpl<Signatures...>, Slot>
{
    private:
        template<typename Signal2, typename Slot2>
        friend struct owning_connection;

        using signal = SignalTpl<Signatures...>;

    public:
        connection(signal& sig, Slot& slot):
            psignal_(&sig),
            event_closure_ids_
            (
                std::make_tuple
                (
                    psignal_->add_raw_event_closure
                    (
                        &detail::slot_caller<Slot, Signatures>::call,
                        &slot
                    )...
                )
            ),
            destruction_closure_id_(add_raw_destruction_closure())
        {
        }

        connection(const connection&) = delete;

        connection(connection&& r):
            psignal_(r.psignal_),
            event_closure_ids_(std::move(r.event_closure_ids_)),
            destruction_closure_id_(add_raw_destruction_closure())
        {
            r.remove_raw_destruction_closure();
            r.psignal_ = nullptr;
        }

        connection& operator=(const connection&) = delete;

        connection& operator=(connection&&) = delete;

        ~connection()
        {
            close();
        }

        void close()
        {
            if(psignal_)
            {
                remove_raw_destruction_closure();

                //Remove event closure of each signature.
                (
                    psignal_->remove_raw_event_closure
                    (
                        std::get<detail::raw_closure_id<Signatures>>(event_closure_ids_)
                    ),
                    ...
                );

                psignal_ = nullptr;
            }
        }

    private:
        auto add_raw_destruction_closure()
        {
            return psignal_->add_raw_destruction_closure
            (
                &on_signal_destruction,
                this
            );
        }

        void remove_raw_destruction_closure()
        {
            psignal_->remove_raw_destruction_closure
            (
                destruction_closure_id_
            );
        }

        static void on_signal_destruction(void* pvself)
        {
            auto& self = *reinterpret_cast<connection*>(pvself);
            self.psignal_ = nullptr;
        }

    private:
        //Pointer to signal.
        //Set to nullptr when connection is closed or moved from.
        signal* psignal_;

        std::tuple
        <
            detail::raw_closure_id<Signatures>...
        > event_closure_ids_;

        detail::raw_closure_id<void()> destruction_closure_id_;
};

} //namespace

#endif

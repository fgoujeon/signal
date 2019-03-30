//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGSIG_SIGNAL_HPP
#define FGSIG_SIGNAL_HPP

#include "owning_connection.hpp"
#include "connection.hpp"
#include "detail/raw_closure.hpp"
#include "detail/voidp_function_ptr.hpp"
#include <list>
#include <iterator>
#include <type_traits>
#include <utility>

namespace fgsig
{

namespace detail
{
    /*
    Base types for signal
    We have to use inheritance to let the compiler do the overload resolution
    for the emit() member function.
    */

    template<typename... Signatures>
    struct signal_base;

    template<typename Signature, typename... Signatures>
    struct signal_base<Signature, Signatures...>:
        public signal_base<Signature>,
        public signal_base<Signatures...>
    {
        public:
            using signal_base<Signature>::emit;
            using signal_base<Signatures...>::emit;

            using signal_base<Signature>::add_raw_event_closure;
            using signal_base<Signatures...>::add_raw_event_closure;

            using signal_base<Signature>::remove_raw_event_closure;
            using signal_base<Signatures...>::remove_raw_event_closure;
    };

    //leaf specialization
    template<typename R, typename... Args>
    struct signal_base<R(Args...)>
    {
        static_assert(std::is_same_v<R, void>, "The return type of a signal signature must be void.");

        private:
            using signature = void(Args...);

        public:
            void emit(Args... args)
            {
                //Call slots.
                {
                    struct recursivity_level_incrementer
                    {
                        recursivity_level_incrementer(unsigned int& r):
                            recursivity_level_(r)
                        {
                            ++recursivity_level_;
                        }

                        ~recursivity_level_incrementer()
                        {
                            --recursivity_level_;
                        }

                        unsigned int& recursivity_level_;
                    };

                    recursivity_level_incrementer rli{recursivity_level_};

                    for(const auto& c: closures_)
                        c.pf(c.pvslot, std::forward<Args>(args)...);
                }

                //Clean closure list in case remove_raw_event_closure() has
                //been called when we were calling slots.
                if(must_clean_closure_list_ && recursivity_level_ == 0)
                {
                    closures_.remove_if
                    (
                        [](const auto& c)
                        {
                            return c.pf == &noop;
                        }
                    );
                    must_clean_closure_list_ = false;
                }
            }

            raw_closure_id<signature> add_raw_event_closure(const voidp_function_ptr<signature> pf, void* pvslot)
            {
                closures_.emplace_back(pf, pvslot);
                return std::prev(closures_.end());
            }

            void remove_raw_event_closure(const raw_closure_id<signature> id)
            {
                if(recursivity_level_ == 0) //Are we iterating on closures_?
                {
                    //If not, erase right now.
                    closures_.erase(id);
                }
                else
                {
                    //If so, postpone erasing.
                    id->pf = &noop;
                    must_clean_closure_list_ = true;
                }
            }

        private:
            static void noop(void*, Args...)
            {
            }

        private:
            std::list<raw_closure<signature>> closures_;
            unsigned int recursivity_level_ = 0;
            bool must_clean_closure_list_ = false;
    };
}

template<typename... Signatures>
struct signal:
    private detail::signal_base<Signatures...>
{
    private:
        template<typename Signal, typename Slot>
        friend struct connection;

    public:
        template<typename Slot>
        using connection = connection<signal, Slot>;

        template<typename Slot>
        using owning_connection = owning_connection<signal, Slot>;

    public:
        signal() = default;

        signal(const signal&) = delete;

        signal(signal&&) = delete;

        signal& operator=(const signal&) = delete;

        signal& operator=(signal&&) = delete;

        ~signal()
        {
            //Notify connections that the signal is destroyed so that they
            //don't try to call remove_*() functions.
            destruction_subsignal_.emit();
        }

        using detail::signal_base<Signatures...>::emit;

    private:
        auto add_raw_destruction_closure(detail::voidp_function_ptr<void()> pf, void* pvconnection)
        {
            return destruction_subsignal_.add_raw_event_closure(pf, pvconnection);
        }

        void remove_raw_destruction_closure(const detail::raw_closure_id<void()> id)
        {
            destruction_subsignal_.remove_raw_event_closure(id);
        }

    private:
        detail::signal_base<void()> destruction_subsignal_;
};

template<typename Signal, typename Slot>
auto connect(Signal& sig, Slot&& slot)
{
    using decaid_signal_t = std::decay_t<Signal>;
    using decaid_slot_t = std::decay_t<Slot>;

    static_assert(!std::is_const_v<Signal>);

    if constexpr(std::is_rvalue_reference_v<decltype(slot)>)
    {
        return owning_connection<decaid_signal_t, decaid_slot_t>{sig, std::move(slot)};
    }
    else
    {
        return connection<decaid_signal_t, decaid_slot_t>{sig, slot};
    }
}

} //namespace

#endif

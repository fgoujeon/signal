//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGL_SIGNAL_HPP
#define FGL_SIGNAL_HPP

#include <list>
#include <tuple>
#include <iterator>
#include <type_traits>

namespace fgl
{

namespace signal_detail
{
    template<class Signature>
    struct fn_ptr;

    template<class R, class... Args>
    struct fn_ptr<R(Args...)>
    {
        using type = R(*)(void*, Args...);
    };

    template<class Signature>
    using fn_ptr_t = typename fn_ptr<Signature>::type;



    template<class Signature>
    struct callback
    {
        callback(fn_ptr_t<Signature> pf, void* pvslot):
            pf(pf),
            pvslot(pvslot)
        {
        }

        fn_ptr_t<Signature> pf;
        void* pvslot;
    };



    template<class Signature>
    using callback_list = std::list<callback<Signature>>;

    template<class Signature>
    using callback_id = typename callback_list<Signature>::iterator;



    template<class Slot, class Signature>
    struct on_event_holder;

    template<class Slot, class R, class... Args>
    struct on_event_holder<Slot, R(Args...)>
    {
        static R on_event(void* pvslot, Args... args)
        {
            auto& slot = *reinterpret_cast<Slot*>(pvslot);
            return slot(args...);
        }
    };



    /*
    Base classes for signal
    We have to use inheritance to let the compiler do the overload resolution
    for the emit() member function.
    */

    template<class... Signatures>
    class signal_base;

    template<class Signature, class... Signatures>
    class signal_base<Signature, Signatures...>:
        public signal_base<Signature>,
        public signal_base<Signatures...>
    {
        public:
            using signal_base<Signature>::emit;
            using signal_base<Signatures...>::emit;

            using signal_base<Signature>::add_callback;
            using signal_base<Signatures...>::add_callback;

            using signal_base<Signature>::remove_callback;
            using signal_base<Signatures...>::remove_callback;
    };

    //leaf specialization
    template<class R, class... Args>
    class signal_base<R(Args...)>
    {
        private:
            using signature = R(Args...);

        public:
            void emit(Args... args)
            {
                for(const auto& s: callbacks_)
                    s.pf(s.pvslot, args...);
            }

            callback_id<signature> add_callback(const fn_ptr_t<signature> pf, void* pvslot)
            {
                callbacks_.emplace_back(pf, pvslot);
                return std::prev(callbacks_.end());
            }

            void remove_callback(const callback_id<signature> id)
            {
                callbacks_.erase(id);
            }

        private:
            std::list<callback<signature>> callbacks_;
    };
}

template<class... Signature>
class signal:
    private signal_detail::signal_base<Signature...>
{
    private:
        template<class Slot>
        class connection
        {
            public:
                connection(signal& sig, Slot& slot):
                    psignal_(&sig),
                    ids_
                    (
                        std::make_tuple
                        (
                            psignal_->add_callback(&signal_detail::on_event_holder<Slot, Signature>::on_event, &slot)...
                        )
                    )
                {
                }

                connection(const connection&) = delete;

                connection(connection&& r):
                    psignal_(r.psignal_),
                    ids_(r.ids_)
                {
                    r.psignal_ = nullptr;
                }

                connection& operator=(const connection&) = delete;

                connection& operator=(connection&&) = delete;

                ~connection()
                {
                    if(psignal_)
                    {
                        (psignal_->remove_callback(std::get<signal_detail::callback_id<Signature>>(ids_)), ...);
                    }
                }

            private:
                signal* psignal_; //set to nullptr when moved from
                std::tuple
                <
                    signal_detail::callback_id<Signature>...
                > ids_;
        };

    public:
        signal() = default;

        signal(const signal&) = delete;

        signal(signal&&) = delete;

        signal& operator=(const signal&) = delete;

        signal& operator=(signal&&) = delete;

        template<class Slot>
        auto connect(Slot& slot)
        {
            using decaid_slot = std::decay_t<Slot>;
            return connection<decaid_slot>{*this, slot};
        }

        using signal_detail::signal_base<Signature...>::emit;
};

} //namespace fgl

#endif

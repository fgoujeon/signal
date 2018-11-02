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
            return slot(std::forward<Args>(args)...);
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

            using signal_base<Signature>::add_event_callback;
            using signal_base<Signatures...>::add_event_callback;

            using signal_base<Signature>::remove_event_callback;
            using signal_base<Signatures...>::remove_event_callback;
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
                    s.pf(s.pvslot, std::forward<Args>(args)...);
            }

            callback_id<signature> add_event_callback(const fn_ptr_t<signature> pf, void* pvslot)
            {
                callbacks_.emplace_back(pf, pvslot);
                return std::prev(callbacks_.end());
            }

            void remove_event_callback(const callback_id<signature> id)
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
                            psignal_->add_event_callback
                            (
                                &signal_detail::on_event_holder<Slot, Signature>::on_event,
                                &slot
                            )...
                        )
                    ),
                    destruction_callback_id_
                    (
                        psignal_->add_destruction_callback
                        (
                            &on_signal_destruction, this
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
                        psignal_->remove_destruction_callback(destruction_callback_id_);

                        //Remove event callback of each signature.
                        (
                            psignal_->remove_event_callback
                            (
                                std::get<signal_detail::callback_id<Signature>>(ids_)
                            ),
                            ...
                        );
                    }
                }

            private:
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
                    signal_detail::callback_id<Signature>...
                > ids_;

                signal_detail::callback_id<void()> destruction_callback_id_;
        };

        template<class Slot>
        class owning_connection
        {
            public:
                owning_connection(signal& sig, Slot&& slot):
                    slot_(std::move(slot)),
                    connection_(sig, slot_)
                {
                }

            private:
                Slot slot_;
                connection<Slot> connection_;
        };

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
            destruction_signal_.emit();
        }

        template<class Slot>
        auto connect(Slot&& slot)
        {
            using decaid_slot = std::decay_t<Slot>;
            return private_connect<decaid_slot>(std::forward<Slot>(slot));
        }

        using signal_detail::signal_base<Signature...>::emit;

    private:
        template<class DecaidSlot>
        auto private_connect(DecaidSlot& slot)
        {
            return connection<DecaidSlot>{*this, slot};
        }

        template<class DecaidSlot>
        auto private_connect(DecaidSlot&& slot)
        {
            return owning_connection<DecaidSlot>{*this, std::move(slot)};
        }

        auto add_destruction_callback(signal_detail::fn_ptr_t<void()> pf, void* pvconnection)
        {
            return destruction_signal_.add_event_callback(pf, pvconnection);
        }

        void remove_destruction_callback(const signal_detail::callback_id<void()> id)
        {
            destruction_signal_.remove_event_callback(id);
        }

    private:
        signal_detail::signal_base<void()> destruction_signal_;
};

} //namespace fgl

#endif

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

    template<class... Args>
    struct fn_ptr<void(Args...)>
    {
        using type = void(*)(void*, Args...);
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

    template<class Slot, class... Args>
    struct on_event_holder<Slot, void(Args...)>
    {
        static void on_event(void* pvslot, Args... args)
        {
            auto& slot = *reinterpret_cast<Slot*>(pvslot);
            slot(std::forward<Args>(args)...);
        }
    };



    /*
    Base classes for signal
    We have to use inheritance to let the compiler do the overload resolution
    for the emit() member function.
    */

    template<class Signature>
    class subsignal;

    //leaf specialization
    template<class R, class... Args>
    class subsignal<R(Args...)>
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

                    for(const auto& cback: callbacks_)
                        cback.pf(cback.pvslot, std::forward<Args>(args)...);
                }

                //Clean callback list in case remove_event_callback() has
                //been called when we were calling slots.
                if(must_clean_callback_list_ && recursivity_level_ == 0)
                {
                    callbacks_.remove_if
                    (
                        [](const auto& cback)
                        {
                            return cback.pf == &noop;
                        }
                    );
                    must_clean_callback_list_ = false;
                }
            }

            callback_id<signature> add_event_callback(const fn_ptr_t<signature> pf, void* pvslot)
            {
                callbacks_.emplace_back(pf, pvslot);
                return std::prev(callbacks_.end());
            }

            void remove_event_callback(const callback_id<signature> id)
            {
                if(recursivity_level_ == 0) //Are we iterating on callbacks_?
                {
                    //If not, erase right now.
                    callbacks_.erase(id);
                }
                else
                {
                    //If so, postpone erasing.
                    id->pf = &noop;
                    must_clean_callback_list_ = true;
                }
            }

        private:
            static void noop(void*, Args...)
            {
            }

        private:
            std::list<callback<signature>> callbacks_;
            unsigned int recursivity_level_ = 0;
            bool must_clean_callback_list_ = false;
    };
}

template<class... Signatures>
class signal:
    private signal_detail::subsignal<Signatures>...
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
                                &signal_detail::on_event_holder<Slot, Signatures>::on_event,
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
                    close();
                }

                void close()
                {
                    if(psignal_)
                    {
                        psignal_->remove_destruction_callback(destruction_callback_id_);

                        //Remove event callback of each signature.
                        (
                            psignal_->remove_event_callback
                            (
                                std::get<signal_detail::callback_id<Signatures>>(ids_)
                            ),
                            ...
                        );

                        psignal_ = nullptr;
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
                    signal_detail::callback_id<Signatures>...
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

                void close()
                {
                    connection_.close();
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
            destruction_subsignal_.emit();
        }

        template<class Slot>
        auto connect(Slot&& slot)
        {
            using decaid_slot = std::decay_t<Slot>;
            return private_connect<decaid_slot>(std::forward<Slot>(slot));
        }

        using signal_detail::subsignal<Signatures>::emit...;

    private:
        using signal_detail::subsignal<Signatures>::add_event_callback...;
        using signal_detail::subsignal<Signatures>::remove_event_callback...;

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
            return destruction_subsignal_.add_event_callback(pf, pvconnection);
        }

        void remove_destruction_callback(const signal_detail::callback_id<void()> id)
        {
            destruction_subsignal_.remove_event_callback(id);
        }

    private:
        signal_detail::subsignal<void()> destruction_subsignal_;
};

} //namespace fgl

#endif

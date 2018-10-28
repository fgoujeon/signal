//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGL_SIGNAL_HPP
#define FGL_SIGNAL_HPP

#include <list>

namespace fgl
{

template<class Signature>
class signal;

template<class R, class... Args>
class signal<R(Args...)>
{
    private:
        using fn_ptr = R(*)(void*, Args...);

        struct callback
        {
            callback(fn_ptr pf, void* pvslot):
                pf(pf),
                pvslot(pvslot)
            {
            }

            fn_ptr pf;
            void* pvslot;
        };

        using callback_list = std::list<callback>;

        using callback_id = typename callback_list::iterator;

        template<class Slot>
        class connection
        {
            public:
                connection(signal& sig, Slot& slot):
                    psignal_(&sig),
                    id_(psignal_->add_callback(&on_event, &slot))
                {
                }

                connection(const connection&) = delete;

                connection(connection&& r):
                    psignal_(r.psignal_),
                    id_(r.id_)
                {
                    r.psignal_ = nullptr;
                }

                connection& operator=(const connection&) = delete;

                connection& operator=(connection&&) = delete;

                ~connection()
                {
                    if(psignal_)
                        psignal_->remove_callback(id_);
                }

            private:
                static R on_event(void* pvslot, Args... args)
                {
                    auto& slot = *reinterpret_cast<Slot*>(pvslot);
                    return slot(args...);
                }

            private:
                signal* psignal_; //set to nullptr when moved from
                callback_id id_;
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

        void emit(Args... args)
        {
            for(const auto& s: callbacks_)
                s.pf(s.pvslot, args...);
        }

    private:
        callback_id add_callback(const fn_ptr pf, void* pvslot)
        {
            callbacks_.emplace_back(pf, pvslot);
            return std::prev(callbacks_.end());
        }

        void remove_callback(const callback_id id)
        {
            callbacks_.erase(id);
        }

    private:
        callback_list callbacks_;
};

} //namespace fgl

#endif

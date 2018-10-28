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
            callback(fn_ptr pf, void* context):
                pf(pf),
                context(context)
            {
            }

            fn_ptr pf;
            void* context;
        };

        using callback_list = std::list<callback>;

        using callback_id = typename callback_list::iterator;

        template<class Slot>
        class connection
        {
            public:
                template<class Slot2>
                connection(signal& sig, Slot2&& slot):
                    psignal_(&sig),
                    slot_(std::forward<Slot2>(slot)),
                    id_(psignal_->add_callback(&on_event, &slot_))
                {
                }

                connection(const connection&) = delete;

                connection(connection&& r):
                    psignal_(r.psignal_),
                    slot_(r.slot_),
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
                static R on_event(void* context, Args... args)
                {
                    auto& slot = *reinterpret_cast<Slot*>(context);
                    return slot(args...);
                }

            private:
                signal* psignal_; //set to nullptr when moved from
                Slot slot_;
                callback_id id_;
        };

    public:
        signal() = default;

        signal(const signal&) = delete;

        signal(signal&&) = delete;

        signal& operator=(const signal&) = delete;

        signal& operator=(signal&&) = delete;

        template<class Slot>
        auto connect(Slot&& slot)
        {
            using decaid_slot = std::decay_t<Slot>;
            return connection<decaid_slot>{*this, std::forward<Slot>(slot)};
        }

        void emit(Args... args)
        {
            for(const auto& s: callbacks_)
                s.pf(s.context, args...);
        }

    private:
        callback_id add_callback(const fn_ptr pf, void* context)
        {
            callbacks_.emplace_back(pf, context);
            auto it = callbacks_.end();
            --it;
            return it;
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

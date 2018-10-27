#ifndef FLG_SIGNALS_HPP
#define FLG_SIGNALS_HPP

#include <list>

namespace fgl { namespace signals
{

template<class Arg>
class signal
{
    private:
        template<class Slot>
        class connection
        {
            public:
                template<class Slot2>
                connection(signal& sig, Slot2&& slot):
                    psignal_(&sig),
                    slot_(std::forward<Slot2>(slot))
                {
                    id_ = psignal_->add_callback(&on_event, this);
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
                static void on_event(void* context, typename signal::arg_type value)
                {
                    auto& self = *reinterpret_cast<connection*>(context);
                    self.slot_(value);
                }

            private:
                signal* psignal_; //set to nullptr when moved from
                Slot slot_;
                typename signal::callback_id id_;
        };

        using fn_ptr = void(*)(void*, Arg);

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

    public:
        using arg_type = Arg;

    public:
        signal() = default;

        signal(const signal&) = delete;

        signal(signal&&) = delete;

        signal& operator=(const signal&) = delete;

        signal& operator=(signal&&) = delete;

        template<class Slot>
        auto connect(Slot&& slot)
        {
            return connection<Slot>{*this, slot};
        }

        void operator()(Arg arg)
        {
            for(auto& s: callbacks_)
                s.pf(s.context, arg);
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

}} //namespace fgl::signals

#endif

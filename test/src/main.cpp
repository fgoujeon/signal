//#include <fgl/signals.hpp>
#include "../../include/fgl/signal.hpp"
#include <string>
#include <cassert>

using signal = fgl::signal<bool(const std::string&)>;

struct slot
{
    private:
        struct internal_slot
        {
            bool operator()(const std::string& value)
            {
                self.str_ += "0" + value;
                return true;
            }

            slot& self;
        };

    public:
        slot(std::string& str, signal& sig):
            str_(str),
            internal_slot_{*this},
            connection_(sig.connect(internal_slot_))
        {
        }

    private:
        std::string& str_;
        internal_slot internal_slot_;
        signal::connection<internal_slot> connection_;
};

int main()
{
    signal sig;

    auto str = std::string{""};

    auto slot0 = slot{str, sig};

    //temporary slot
    {
        auto slot1 = [&str](const std::string& value)
        {
            str += "1" + value;
            return true;
        };

        auto slot1_connection = sig.connect(slot1);

        sig.emit("a");

        //automatic disconnection
    }

    sig.emit("b");

    assert(str == "0a1a0b");
}

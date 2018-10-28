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
            connection_(sig.connect(internal_slot{*this}))
        {
        }

    private:
        std::string& str_;
        signal::connection<internal_slot> connection_;
};

int main()
{
    signal sig;

    auto str = std::string{""};

    auto slot0 = slot{str, sig};

    //temporary slot
    {
        auto slot1_connection = sig.connect
        (
            [&str](const std::string& value)
            {
                str += "1" + value;
                return true;
            }
        );

        sig("a");
    }

    sig("b");

    assert(str == "0a1a0b");
}

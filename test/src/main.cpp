#include <fgl/signal.hpp>
#include <string>
#include <cassert>

using signal = fgl::signal
<
    bool(int),
    bool(const std::string&)
>;

const std::string& to_string(const std::string& str)
{
    return str;
}

std::string to_string(const int i)
{
    return std::to_string(i);
}

struct slot
{
    private:
        struct internal_slot
        {
            bool operator()(const int value)
            {
                self.str_ += "0i" + std::to_string(value);
                return true;
            }

            bool operator()(const std::string& value)
            {
                self.str_ += "0s" + value;
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
        auto slot1 = [&str](const auto& value)
        {
            str += "1" + to_string(value);
            return true;
        };

        auto slot1_connection = sig.connect(slot1);

        sig.emit(42);

        //automatic disconnection
    }

    sig.emit("b");

    assert(str == "0i421420sb");
}

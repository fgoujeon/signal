#include <fgl/signal.hpp>
#include <string>
#include <sstream>
#include <cassert>

using signal = fgl::signal
<
    bool(int),
    bool(const std::string&)
>;

struct slot_with_non_owing_connection
{
    private:
        struct internal_slot
        {
            bool operator()(const int value)
            {
                self.oss_ << "0i" << value;
                return true;
            }

            bool operator()(const std::string& value)
            {
                self.oss_ << "0s" << value;
                return true;
            }

            slot_with_non_owing_connection& self;
        };

    public:
        slot_with_non_owing_connection(std::ostringstream& oss, signal& sig):
            oss_(oss),
            internal_slot_{*this},
            connection_(sig.connect(internal_slot_))
        {
        }

    private:
        std::ostringstream& oss_;
        internal_slot internal_slot_;
        signal::connection<internal_slot> connection_;
};

struct slot_with_owning_connection
{
    private:
        struct internal_slot
        {
            bool operator()(const int value)
            {
                self.oss_ << "2i" << value;
                return true;
            }

            bool operator()(const std::string& value)
            {
                self.oss_ << "2s" << value;
                return true;
            }

            slot_with_owning_connection& self;
        };

    public:
        slot_with_owning_connection(std::ostringstream& oss, signal& sig):
            oss_(oss),
            connection_(sig.connect(internal_slot{*this}))
        {
        }

    private:
        std::ostringstream& oss_;
        signal::owning_connection<internal_slot> connection_;
};

int main()
{
    signal sig;
    auto oss = std::ostringstream{};

    //permanent slot, class, non-owning connection
    auto slot0 = slot_with_non_owing_connection{oss, sig};

    //temporary slot, lambda, non-owning connection
    {
        auto slot1 = [&oss](const auto& value)
        {
            oss << "1" << value;
            return true;
        };

        auto slot1_connection = sig.connect(slot1);

        sig.emit(42);

        //automatic disconnection
    }

    //temporary slot, class, owning connection
    {
        auto slot2 = slot_with_owning_connection{oss, sig};

        sig.emit("a");
    }

    //temporary slot, lambda, owning connection
    {
        auto slot3_connection = sig.connect
        (
            [&oss](const auto& value)
            {
                oss << "3" << value;
                return true;
            }
        );

        sig.emit(8);

        //automatic disconnection
    }

    sig.emit("b");

    const auto expected_str =
        //emit(42)
        "0i42" //slot0
        "142" //slot1

        //emit("a")
        "0sa" //slot0
        "2sa" //slot2

        //emit(8)
        "0i8" //slot0
        "38" //slot3

        //emit("b")
        "0sb" //slot0
    ;

    assert(oss.str() == expected_str);
}

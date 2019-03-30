#ifndef TESTS_BASIC_HPP
#define TESTS_BASIC_HPP

#include <fgsig.hpp>
#include <sstream>
#include <string>

namespace tests::basic
{

using signal = fgsig::signal
<
    void(int),
    void(const std::string&)
>;

struct slot_with_non_owing_connection
{
    private:
        struct internal_slot
        {
            void operator()(const int value)
            {
                self.oss_ << "0i" << value;
            }

            void operator()(const std::string& value)
            {
                self.oss_ << "0s" << value;
            }

            slot_with_non_owing_connection& self;
        };

    public:
        slot_with_non_owing_connection(std::ostringstream& oss, signal& sig):
            oss_(oss),
            internal_slot_{*this},
            connection_(fgsig::connect(sig, internal_slot_))
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
            void operator()(const int value)
            {
                self.oss_ << "2i" << value;
            }

            void operator()(const std::string& value)
            {
                self.oss_ << "2s" << value;
            }

            slot_with_owning_connection& self;
        };

    public:
        slot_with_owning_connection(std::ostringstream& oss, signal& sig):
            oss_(oss),
            connection_(fgsig::connect(sig, internal_slot{*this}))
        {
        }

    private:
        std::ostringstream& oss_;
        signal::owning_connection<internal_slot> connection_;
};

bool test()
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
        };

        auto slot1_connection = fgsig::connect(sig, slot1);

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
        auto slot3_connection = fgsig::connect
        (
            sig,
            [&oss](const auto& value)
            {
                oss << "3" << value;
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

    return oss.str() == expected_str;
}

} //namespace

#endif

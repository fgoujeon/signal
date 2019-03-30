#ifndef TESTS_MOVE_CONNECTION_HPP
#define TESTS_MOVE_CONNECTION_HPP

#include <fgl/signals.hpp>
#include <sstream>
#include <string>
#include <functional>

namespace tests::move_connection
{

using signal = fgl::signals::signal<void(int)>;

bool test()
{
    std::ostringstream oss;
    signal sig;

    //non-owning connection
    auto slot0 = [&oss](const auto& value)
    {
        oss << "0" << value;
    };
    auto connection0 = fgl::signals::connect(sig, slot0);
    auto connection0b = std::move(connection0);

    //owning connection
    auto slot1 = [&oss](const auto& value)
    {
        oss << "1" << value;
    };
    auto connection1 = fgl::signals::connect(sig, std::move(slot1));
    auto connection1b = std::move(connection1);

    //owning connection with std::function slot
    auto slot2 = std::function<void(int)>
    {
        [&oss](const auto& value)
        {
            oss << "2" << value;
        }
    };
    auto connection2 = fgl::signals::connect(sig, std::move(slot2));
    auto connection2b = std::move(connection2);

    //static checks
    {
        using slot0_t = decltype(slot0);
        using connection0_t = decltype(connection0);
        using connection0b_t = decltype(connection0b);
        using expected_connection0_t = fgl::signals::connection<signal, slot0_t>;
        static_assert(std::is_same_v<connection0_t, expected_connection0_t>);
        static_assert(std::is_same_v<connection0b_t, expected_connection0_t>);

        using slot1_t = decltype(slot1);
        using connection1_t = decltype(connection1);
        using connection1b_t = decltype(connection1b);
        using expected_connection1_t = fgl::signals::owning_connection<signal, slot1_t>;
        static_assert(std::is_same_v<connection1_t, expected_connection1_t>);
        static_assert(std::is_same_v<connection1b_t, expected_connection1_t>);

        using slot2_t = decltype(slot2);
        using connection2_t = decltype(connection2);
        using connection2b_t = decltype(connection2b);
        using expected_connection2_t = fgl::signals::owning_connection<signal, slot2_t>;
        static_assert(std::is_same_v<connection2_t, expected_connection2_t>);
        static_assert(std::is_same_v<connection2b_t, expected_connection2_t>);
    }

    sig.emit(99);

    const auto expected_str =
        "099"
        "199"
        "299"
    ;
    return oss.str() == expected_str;
}

} //namespace

#endif

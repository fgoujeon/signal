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

    auto slot0 = [&oss](const auto& value)
    {
        oss << "0" << value;
    };
    auto connection0 = sig.connect(slot0);
    auto connection0b = std::move(connection0);

    auto connection1 = sig.connect
    (
        [&oss](const auto& value)
        {
            oss << "1" << value;
        }
    );
    auto connection1b = std::move(connection1);

    auto connection2 = sig.connect
    (
        std::function<void(int)>([&oss](const auto& value)
        {
            oss << "2" << value;
        }
    ));
    auto connection2b = std::move(connection2);
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

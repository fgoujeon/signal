#ifndef TESTS_MOVE_CONNECTION_HPP
#define TESTS_MOVE_CONNECTION_HPP

#include <fgl/signal.hpp>
#include <sstream>
#include <string>

namespace tests::move_connection
{

using signal = fgl::signal<void(int)>;

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

    sig.emit(99);

    const auto expected_str =
        "099"
        "199"
    ;

    return oss.str() == expected_str;
}

} //namespace

#endif

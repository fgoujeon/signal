#ifndef TESTS_MOVE_HPP
#define TESTS_MOVE_HPP

#include <fgl/signals.hpp>
#include <memory>

namespace tests::move
{

using signal = fgl::signals::signal
<
    void(std::unique_ptr<int>&&, const std::string&)
>;

bool test()
{
    auto ok = true;

    signal sig;

    auto connection0 = sig.connect
    (
        [&ok](std::unique_ptr<int>&& upi, const std::string& str)
        {
            ok = ok && upi && *upi == 4 && str == "test";
        }
    );

    auto connection1 = sig.connect
    (
        [&ok](std::unique_ptr<int>&& upi, const std::string& str)
        {
            ok = ok && upi && *upi == 4 && str == "test";
            auto upi2 = std::move(upi);
        }
    );

    auto connection2 = sig.connect
    (
        [&ok](std::unique_ptr<int>&& upi, const std::string& str)
        {
            ok = ok && !upi && str == "test";
        }
    );

    sig.emit(std::move(std::make_unique<int>(4)), "test");

    return ok;
}

} //namespace

#endif

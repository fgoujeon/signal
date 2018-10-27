//#include <fgl/signals.hpp>
#include "../../include/fgl/signals.hpp"
#include <string>
#include <cassert>

int main()
{
    fgl::signals::signal<bool(const std::string&)> signal;

    auto str = std::string{""};

    auto connection0 = signal.connect
    (
        [&str](const std::string& value)
        {
            str += "0" + value;
            return true;
        }
    );

    {
        auto connection1 = signal.connect
        (
            [&str](const std::string& value)
            {
                str += "1" + value;
                return true;
            }
        );

        signal("a");
    }

    signal("b");

    assert(str == "0a1a0b");
}

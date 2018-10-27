//#include <fgl/signals.hpp>
#include "../../include/fgl/signals.hpp"
#include <cassert>

int main()
{
    fgl::signals::signal<bool(int)> signal;

    auto i = 0;

    auto connection = signal.connect
    (
        [&i](const int value)
        {
            i += value;
            return true;
        }
    );

    {
        auto connection2 = signal.connect
        (
            [&i](const int value)
            {
                i += value * 10;
                return true;
            }
        );

        signal(1);
    }

    signal(2);

    assert(i == 13);
}

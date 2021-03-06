#ifndef TESTS_SIGNAL_DESTROYED_BEFORE_SLOT_HPP
#define TESTS_SIGNAL_DESTROYED_BEFORE_SLOT_HPP

//Check that automatic disconnection doesn't lead to segfault when signal
//is destroyed before slot.

#include <fgsig.hpp>
#include <memory>

namespace tests::signal_destroyed_before_slot
{

using signal = fgsig::signal<void(int)>;

bool test()
{
    {
        auto psig = std::make_unique<signal>();
        auto connection = fgsig::connect(*psig, [](int){});
        psig.reset();
    }

    return true;
}

} //namespace

#endif

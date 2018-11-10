#ifndef TESTS_BASIC_EXAMPLE_HPP
#define TESTS_BASIC_EXAMPLE_HPP

#include "../utility/cout_redirector.hpp"
#include <fgl/signals.hpp>
#include <iostream>

namespace tests::basic_example
{

void run()
{
//tag::example[]
fgl::signals::signal<void(int)> signal;
auto connection = signal.connect([](int value){std::cout << value << '\n';});
signal.emit(42);
//end::example[]
}

bool test()
{
    std::ostringstream oss;
    utility::cout_redirector redirector{oss};
    run();
    return oss.str() == "42\n";
}

} //namespace

#endif

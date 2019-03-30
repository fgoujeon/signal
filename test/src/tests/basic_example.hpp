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
auto connection0 = fgl::signals::connect(signal, [](int value){std::cout << "Hello " << value << '\n';});
auto connection1 = fgl::signals::connect(signal, [](int value){std::cout << "World " << value << '\n';});
signal.emit(42);
//end::example[]
}

bool test()
{
    std::ostringstream oss;
    utility::cout_redirector redirector{oss};
    run();

    auto expected_str =
        "Hello 42\n"
        "World 42\n"
    ;

    return oss.str() == expected_str;
}

} //namespace

#endif

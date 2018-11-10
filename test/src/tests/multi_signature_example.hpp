#ifndef TESTS_MULTI_SIGNATURE_EXAMPLE_HPP
#define TESTS_MULTI_SIGNATURE_EXAMPLE_HPP

#include "../utility/cout_redirector.hpp"
#include <fgl/signals.hpp>
#include <iostream>

namespace tests::multi_signature_example
{

struct whatever_type_you_want
{
};

std::ostream& operator<<(std::ostream& l, const whatever_type_you_want&)
{
    return l << "whatever string";
}

void run()
{
//tag::example[]
fgl::signals::signal
<
    void(int),
    void(const std::string&),
    void(const whatever_type_you_want&)
> signal;

std::ostringstream oss;
auto connection = signal.connect([&oss](const auto& value){oss << value << '\n';});

signal.emit(42);
signal.emit("test");
signal.emit(whatever_type_you_want{});

std::cout << oss.str();
//end::example[]
}

bool test()
{
    std::ostringstream oss;
    {
        utility::cout_redirector redirector{oss};
        run();
    }

    const auto expected_out =
        "42\n"
        "test\n"
        "whatever string\n"
    ;

    return oss.str() == expected_out;
}

} //namespace

#endif

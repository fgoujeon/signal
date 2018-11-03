#ifndef TESTS_MULTI_SIGNATURE_EXAMPLE_HPP
#define TESTS_MULTI_SIGNATURE_EXAMPLE_HPP

#include "../utility/cout_redirector.hpp"
#include <fgl/signal.hpp>
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
fgl::signal
<
    void(int),
    void(const std::string&),
    void(const whatever_type_you_want&)
> my_multi_signal;

std::ostringstream oss;
auto slot = [&oss](const auto& value){oss << value << '\n';};

auto connection = my_multi_signal.connect(slot);

my_multi_signal.emit(42);
my_multi_signal.emit("test");
my_multi_signal.emit(whatever_type_you_want{});

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

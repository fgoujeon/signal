#ifndef TESTS_FULL_EXAMPLE_HPP
#define TESTS_FULL_EXAMPLE_HPP

#include "../utility/cout_redirector.hpp"

//tag::example[]
#include <fgl/signal.hpp>
#include <string>
#include <sstream>
#include <iostream>
//end::example[]

namespace tests::full_example
{

//tag::example[]

struct event_emitter
{
    public:
        using signal = fgl::signal
        <
            void(int),
            void(const std::string&),
            void(double)
        >;

    public:
        template<class Slot>
        auto connect(Slot&& slot)
        {
            return signal_.connect(std::forward<Slot>(slot));
        }

        void run()
        {
            signal_.emit(1);
            signal_.emit('2');
            signal_.emit("3");
            signal_.emit(4.0);
        }

    private:
        signal signal_;
};

struct event_receiver
{
    private:
        struct slot
        {
            void operator()(const int value)
            {
                self.oss_ << "int: " << std::to_string(value) << '\n';
            }

            void operator()(const std::string& value)
            {
                self.oss_ << "string: " << value << '\n';
            }

            void operator()(const double value)
            {
                self.oss_ << "double: " << value << '\n';
            }

            event_receiver& self;
        };

    public:
        event_receiver(event_emitter& emitter):
            slot_{*this},
            connection_(emitter.connect(slot_))
        {
        }

        std::string get_output() const
        {
            return oss_.str();
        }

    private:
        std::ostringstream oss_;
        slot slot_;
        event_emitter::signal::connection<slot> connection_;
};

int main()
{
    event_emitter emitter;
    event_receiver receiver{emitter};
    emitter.run();
    std::cout << receiver.get_output();
    return 0;
}
//end::example[]

bool test()
{
    std::ostringstream oss;
    {
        utility::cout_redirector redirector{oss};
        main();
    }

    const auto expected_out =
        "int: 1\n"
        "int: 50\n"
        "string: 3\n"
        "double: 4\n"
    ;

    return oss.str() == expected_out;
}

} //namespace

#endif

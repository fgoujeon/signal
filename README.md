# fgl::signal
fgl::signal is a fast, type-safe, multi-signature, C++17 signal library.

**THIS LIBRARY IS AT EARLY DEVELOPMENT STAGE AND SHOULDN'T BE USED IN PRODUCTION!**

## Signal
> Signals and slots is a language construct [...] for communication between objects which makes it easy to implement the observer pattern while avoiding boilerplate code.

— [Wikipedia](https://en.wikipedia.org/wiki/Signals_and_slots)

fgl::signal lets you send events with minimum code:
```c++
fgl::signal<void(int)> my_signal;
auto slot = [](const int value){std::cout << value << '\n';};
auto connection = my_signal.connect(slot); //automatic disconnection at end of scope
my_signal.emit(42);
```

Output:
```
42
```

## Type-Safe
The `fgl::signal` class template takes a function signature as template parameter. It won't let you connect a slot whose signature doesn't match.

## Multi-Signature
The `fgl::signal` class template can actually take more than one function signature as template parameters:
```c++
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
```

Output:
```
42
test
whatever string
```

## Fast
See [benchmark](https://github.com/fgoujeon/signal-benchmark).

Despite its type-safe interface, fgl::signal internally uses `void*`-based type erasure.

### Full Example
```c++
#include <fgl/signal.hpp>
#include <string>
#include <sstream>
#include <iostream>

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
}
```

Output:
```
int: 1
int: 50
string: 3
double: 4
```

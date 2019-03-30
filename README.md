# fgsig
fgsig is a fast, type-safe, multi-signature, C++17 signal/slot library.

**THIS LIBRARY IS AT EARLY DEVELOPMENT STAGE AND SHOULDN'T BE USED IN PRODUCTION!**

## Signals and Slots
Signals and slots is a mechanism for communication between objects which makes it easy to implement the [observer pattern](https://en.wikipedia.org/wiki/Observer_pattern) while avoiding boilerplate code.

fgsig lets you send notifications to any number of observers (or slots) with minimum code:
```c++
fgsig::signal<void(int)> signal;
auto connection0 = fgsig::connect(signal, [](int value){std::cout << "Hello " << value << '\n';});
auto connection1 = fgsig::connect(signal, [](int value){std::cout << "World " << value << '\n';});
signal.emit(42);
```

Output:
```
Hello 42
World 42
```

Note that keeping the `connection` instance alive is mandatory to maintain the connection between the signal and the slot. When a `connection` instance is destroyed, the connection it holds is closed.

## Type-Safe
The `fgsig::signal` class template takes a function signature as template parameter. It won't let you connect a slot whose signature doesn't match.

## Multi-Signature
The `fgsig::signal` class template can actually take more than one function signature as template parameters:
```c++
fgsig::signal
<
    void(int),
    void(const std::string&),
    void(const whatever_type_you_want&)
> signal;

std::ostringstream oss;
auto connection = fgsig::connect(signal, [&oss](const auto& value){oss << value << '\n';});

signal.emit(42);
signal.emit("test");
signal.emit(whatever_type_you_want{});

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

Despite its type-safe interface, fgsig internally uses `void*`-based type erasure, which is the fastest technique of type erasure.

## No Dependency
fgsig doesn't depend on any other library than the C++ standard library.

## Full Example
Here is how you could use fgsig in a real-life project:

```c++
#include <fgl/signals.hpp>
#include <string>
#include <sstream>
#include <iostream>

/*
This class represents a car.
You can fill its fuel tank and drive it.
Also, it sends events using fgsig.
*/
struct car
{
    public:
        //properties
        struct fuel_level_l { unsigned int value = 0; }; //fuel level in L
        struct speed_kmh    { unsigned int value = 0; }; //speed in km/h

        //events
        template<class Property>
        struct property_change_event
        {
            decltype(Property::value) value;
        };
        struct stall_event{};

        using signal = fgsig::signal
        <
            void(const property_change_event<fuel_level_l>&),
            void(const property_change_event<speed_kmh>&),
            void(const stall_event&)
        >;

    public:
        //Connect to signal to receive events.
        template<class Slot>
        auto connect(Slot&& slot)
        {
            return fgsig::connect(signal_, std::forward<Slot>(slot));
        }

        //Add some fuel.
        //Quantity is in L.
        void add_fuel_l(const unsigned int quantity_l)
        {
            set<fuel_level_l>(get<fuel_level_l>() + quantity_l);
        }

        //Drive 20 km at 100 km/h.
        //Return true if the car has enough fuel.
        bool drive_20_km()
        {
            if(get<fuel_level_l>() > 0)
            {
                set<speed_kmh>(100);
                set<fuel_level_l>(get<fuel_level_l>() - 1);
                return true;
            }
            else
            {
                signal_.emit(stall_event{});
                set<speed_kmh>(0);
                return false;
            }
        }

    private:
        //Get value of given property.
        template<class Property>
        const decltype(Property::value)& get() const
        {
            return std::get<Property>(properties_).value;
        }

        //Set value of given property, and send corresponding
        //property_change_event if value changes.
        template<class Property>
        void set(const decltype(Property::value)& new_value)
        {
            auto& current_value = std::get<Property>(properties_).value;
            if(current_value != new_value)
            {
                current_value = new_value;
                signal_.emit(property_change_event<Property>{new_value});
            }
        }

    private:
        std::tuple
        <
            fuel_level_l,
            speed_kmh
        > properties_;

        signal signal_;
};

/*
This class prints out the current state of the given car.
*/
struct car_monitor
{
    public:
        car_monitor(car& c):
            connection_{c.connect([this](const auto& event){handle_event(event);})}
        {
        }

    private:
        void handle_event(const car::property_change_event<car::speed_kmh>& event)
        {
            std::cout << "Speed = " << event.value << " km/h\n";
        }

        void handle_event(const car::property_change_event<car::fuel_level_l>& event)
        {
            std::cout << "Fuel level = " << event.value << " L\n";
        }

        void handle_event(const car::stall_event&)
        {
            std::cout << "Car stalled\n";
        }

    private:
        fgsig::any_connection connection_;
};

int main()
{
    car c;
    car_monitor monitor{c};

    c.add_fuel_l(10);
    while(c.drive_20_km());

    return 0;
}
```

Output:
```
Fuel level = 10 L
Speed = 100 km/h
Fuel level = 9 L
Fuel level = 8 L
Fuel level = 7 L
Fuel level = 6 L
Fuel level = 5 L
Fuel level = 4 L
Fuel level = 3 L
Fuel level = 2 L
Fuel level = 1 L
Fuel level = 0 L
Car stalled
Speed = 0 km/h
```

## Limitations

### Thread Safety
fgsig doesn't provide any thread safety mechanism.

Users are encouraged to handle thread safety at a higher level. Possible solutions are:
* an implementation of the Active Object design pattern;
* a `boost::asio::io_context` running on a single thread.

# fgl::signal
fgl::signal is a fast, type-safe, multi-signature, C++17 signal library.

**THIS LIBRARY IS AT EARLY DEVELOPMENT STAGE AND SHOULDN'T BE USED IN PRODUCTION!**

## Signal
> Signals and slots is a language construct [...] for communication between objects which makes it easy to implement the observer pattern while avoiding boilerplate code.

— [Wikipedia](https://en.wikipedia.org/wiki/Signals_and_slots)

fgl::signal lets you send events with minimum code:
```c++
fgl::signal<void(int)> signal;
auto connection = signal.connect([](int value){std::cout << value << '\n';});
signal.emit(42);
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
> signal;

std::ostringstream oss;
auto connection = signal.connect([&oss](const auto& value){oss << value << '\n';});

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

Despite its type-safe interface, fgl::signal internally uses `void*`-based type erasure, which is the fastest technique of type erasure.

### Full Example
Here is how you could use fgl::signal in a real-life project:

```c++
#include <fgl/signal.hpp>
#include <string>
#include <sstream>
#include <iostream>

/*
This class represents a car.
You can fill its fuel tank and drive it.
Also, it sends events using fgl::signal.
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

        using signal = fgl::signal
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
            return signal_.connect(std::forward<Slot>(slot));
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
    private:
        struct slot
        {
            template<class Event>
            void operator()(const Event& event)
            {
                self.handle_event(event);
            }

            car_monitor& self;
        };

    public:
        car_monitor(car& c):
            connection_(c.connect(slot{*this}))
        {
        }

    private:
        void handle_event(const car::property_change_event<car::speed_kmh>& event)
        {
            std::cout << "Speed = " << std::to_string(event.value) << " km/h\n";
        }

        void handle_event(const car::property_change_event<car::fuel_level_l>& event)
        {
            std::cout << "Fuel level = " << std::to_string(event.value) << " L\n";
        }

        void handle_event(const car::stall_event&)
        {
            std::cout << "Car stalled\n";
        }

    private:
        car::signal::owning_connection<slot> connection_;
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

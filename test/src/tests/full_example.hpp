#ifndef TESTS_FULL_EXAMPLE_HPP
#define TESTS_FULL_EXAMPLE_HPP

#include "../utility/cout_redirector.hpp"

//tag::example[]
#include <fgl/signals.hpp>
#include <string>
#include <sstream>
#include <iostream>
//end::example[]

namespace tests::full_example
{

//tag::example[]

/*
This class represents a car.
You can fill its fuel tank and drive it.
Also, it sends events using fgl::signals.
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

        using signal = fgl::signals::signal
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
            return fgl::signals::connect(signal_, std::forward<Slot>(slot));
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
        fgl::signals::any_connection connection_;
};

int main()
{
    car c;
    car_monitor monitor{c};

    c.add_fuel_l(10);
    while(c.drive_20_km());

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
        "Fuel level = 10 L\n"
        "Speed = 100 km/h\n"
        "Fuel level = 9 L\n"
        "Fuel level = 8 L\n"
        "Fuel level = 7 L\n"
        "Fuel level = 6 L\n"
        "Fuel level = 5 L\n"
        "Fuel level = 4 L\n"
        "Fuel level = 3 L\n"
        "Fuel level = 2 L\n"
        "Fuel level = 1 L\n"
        "Fuel level = 0 L\n"
        "Car stalled\n"
        "Speed = 0 km/h\n"
    ;

    return oss.str() == expected_out;
}

} //namespace

#endif

//Copyright Florian Goujeon 2018.
//Distributed under the Boost Software License, Version 1.0.
//(See accompanying file LICENSE_1_0.txt or copy at
//https://www.boost.org/LICENSE_1_0.txt)
//Official repository: https://github.com/fgoujeon/signal

#ifndef FGL_SIGNALS_ANY_CONNECTION_HPP
#define FGL_SIGNALS_ANY_CONNECTION_HPP

#include <memory>
#include <type_traits>
#include <utility>

namespace fgl::signals
{

/*
any_connection is a type-erasing container for any connection or
owning_connection object.
*/

struct any_connection
{
    private:
        struct abstract_connection_holder
        {
            virtual ~abstract_connection_holder(){}
            virtual void close() = 0;
        };

        template<typename Connection>
        struct connection_holder: abstract_connection_holder
        {
            public:
                connection_holder(Connection&& c):
                    connection_(std::move(c))
                {
                }

                void close()
                {
                    connection_.close();
                }

            private:
                Connection connection_;
        };

    public:
        template<typename Connection>
        any_connection(Connection&& c):
            holder_
            {
                std::make_unique<connection_holder<std::decay_t<Connection>>>
                (
                    std::forward<Connection>(c)
                )
            }
        {
        }

        any_connection(const any_connection&) = delete;

        any_connection(any_connection&&) = delete;

        any_connection& operator=(const any_connection&) = delete;

        any_connection& operator=(any_connection&&) = delete;

        void close()
        {
            holder_->close();
        }

    private:
        std::unique_ptr<abstract_connection_holder> holder_;
};

} //namespace

#endif

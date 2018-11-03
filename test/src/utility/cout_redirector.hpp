#ifndef UTILITY_COUT_REDIRECTOR_HPP
#define UTILITY_COUT_REDIRECTOR_HPP

#include <iostream>

namespace utility
{

struct cout_redirector
{
    public:
        cout_redirector(std::ostream& out):
            original_cout_buf_(std::cout.rdbuf())
        {
            std::cout.rdbuf(out.rdbuf()); //redirect std::cout to oss_
        }

        ~cout_redirector()
        {
            std::cout.rdbuf(original_cout_buf_); //cancel redirection
        }

    private:
        std::streambuf* original_cout_buf_;
};

} //namespace

#endif

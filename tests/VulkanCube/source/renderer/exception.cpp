#include "exception.h"

Exception::Exception(
    const std::string &inMessage)
    :
    _message(inMessage)
{}

const char *
Exception::what() const
{
    return this->_message.c_str();
}

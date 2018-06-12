#include "exception.h"

Exception::Exception(
    const std::string &inMessage) throw()
    :
    _message(inMessage)
{}

Exception::~Exception() throw()
{}

const char *
Exception::what() const throw()
{
    return this->_message.c_str();
}

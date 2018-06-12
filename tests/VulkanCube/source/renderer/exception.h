#ifndef VULKAN_RENDERER_EXCEPTION_H
#define VULKAN_RENDERER_EXCEPTION_H

#include <string>
#include <stdexcept>

class Exception :
    public std::exception
{
public:
    Exception(
        const std::string &inMessage) throw();

    ~Exception() throw();

    const char *
    what() const throw() override;

private:
    std::string _message;
};

#endif // VULKAN_RENDERER_EXCEPTION_H

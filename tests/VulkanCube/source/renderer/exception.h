#ifndef VULKAN_RENDERER_EXCEPTION_H
#define VULKAN_RENDERER_EXCEPTION_H

#include <stdexcept>

class Exception :
    public std::exception
{
public:
    Exception(
        const std::string &inMessage);

    const char *
    what() const override;

private:
    std::string _message;
};

#endif // VULKAN_RENDERER_EXCEPTION_H

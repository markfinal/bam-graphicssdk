#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <memory>

class Renderer
{
public:
    Renderer();
    ~Renderer();

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

#endif // VULKAN_RENDERER_H

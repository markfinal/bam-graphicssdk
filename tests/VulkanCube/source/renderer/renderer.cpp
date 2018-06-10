#include "impl.h"
#include "exception.h"

Renderer::Renderer()
    :
    _impl(new Impl)
{}

Renderer::~Renderer()
{
    auto impl = this->_impl.get();
    impl->clean_up();
}

void
Renderer::init()
{
    auto impl = this->_impl.get();
    impl->create_instance();
    impl->enumerate_physics_devices();
    impl->create_logical_device();
}

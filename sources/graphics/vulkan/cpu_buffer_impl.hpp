#ifndef STRAITX_VULKAN_CPU_BUFFER_IMPL_HPP
#define STRAITX_VULKAN_CPU_BUFFER_IMPL_HPP

#include "core/os/vulkan.hpp"
#include "graphics/cpu_buffer.hpp"

namespace Vk{

struct CPUBufferImpl{
    VkBuffer &Handle;
    VkDeviceMemory &Memory;
    u32 &Size;
    void *&Pointer;

    // NOTE: CPUBuffer should already have an owner because we can't cast type of pointer in a reference to pointer variable
    CPUBufferImpl(CPUBuffer &buffer);

    CPUBufferImpl(VkBuffer &handle, VkDeviceMemory &memory, u32 &size, void *&pointer);

    void Create(u32 size);

    void Destroy();

    static void NewImpl(CPUBuffer &buffer, u32 size);

    static void DeleteImpl(CPUBuffer &buffer);
};

}//namespace Vk::

#endif//STRAITX_VULKAN_CPU_BUFFER_IMPL_HPP
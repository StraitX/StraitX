#include "graphics/vulkan/command_pool.hpp"

namespace StraitX{
namespace Vk{


Error CommandPool::Create(Vk::LogicalDevice *owner, Vk::Queue queue){
    Owner = owner;
    Queue = queue;
    VkCommandPoolCreateInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.queueFamilyIndex = queue.FamilyIndex;
    if(vkCreateCommandPool(Owner->Handle, &info, nullptr, &Handle) != VK_SUCCESS)
        return Error::Failure;
    return Error::Success;
}

void CommandPool::Destroy(){
    vkDestroyCommandPool(Owner->Handle, Handle, nullptr);
}

};//namespace Vk::
};//namespace StraitX::
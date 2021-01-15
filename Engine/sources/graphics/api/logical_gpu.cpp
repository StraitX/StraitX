#include "core/log.hpp"
#include "graphics/api/logical_gpu.hpp"
#include "graphics/api/graphics_api.hpp"
#include "graphics/vulkan/logical_gpu_impl.hpp"

namespace StraitX{

LogicalGPU *LogicalGPU::New(){
    switch (GraphicsAPI::Instance().CurrentAPI()) {
    case GraphicsAPI::Vulkan:
        return new Vk::LogicalGPUImpl();
    default:
        LogWarn("LogicalGPU::New: Unsupported API");
        return nullptr;
    }
}

void LogicalGPU::Delete(LogicalGPU *gpu){
    switch (GraphicsAPI::Instance().CurrentAPI()) {
    case GraphicsAPI::Vulkan:
        delete ((Vk::LogicalGPUImpl*)gpu);
        return;
    default:
        LogWarn("LogicalGPU::Delete: Unsupported API");
    }
}

}//namespace StraitX::
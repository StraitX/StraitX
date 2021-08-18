#include "graphics/api/graphics_api.hpp"
#include "core/os/vulkan.hpp"

#if defined(SX_VULKAN_SUPPORTED)
    #include "graphics/api/vulkan/graphics_api_impl.hpp"
#endif

GraphicsAPIBackendImpl *GraphicsAPI::s_Impl = nullptr;
GraphicsAPIBackend GraphicsAPI::s_Backend = GraphicsAPIBackend::None;

Result GraphicsAPI::CreateBackend(GraphicsAPIBackend backend){
#if defined(SX_VULKAN_SUPPORTED)
    if(backend == GraphicsAPIBackend::Vulkan)
        s_Impl = &Vk::GraphicsAPIBackendImpl::s_Instance;
#endif

    if(s_Impl)
        return s_Impl->Create();
    
    return Result::Unsupported;
}

void GraphicsAPI::DestroyBackend(){
    if(s_Impl)
        s_Impl->Destroy();
    s_Impl = nullptr;
}
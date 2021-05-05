#ifndef STRAITX_APPLICATION_HPP
#define STRAITX_APPLICATION_HPP

#include "platform/events.hpp"
#include "core/math/vector2.hpp"
#include "graphics/api/graphics_api.hpp"
#include "graphics/api/swapchain.hpp"

namespace StraitX{

class Engine;

struct ApplicationConfig{
    const char *ApplicationName = "StraitX Game";
    GraphicsAPI::API DesiredAPI = GraphicsAPI::API::OpenGL;
    SwapchainProperties SwapchainProps = {};
};

class Application{
private:
    Engine *m_Engine = nullptr;
    friend class Engine;
public:
    Application() = default;

    virtual ~Application() = default;

    virtual Result OnInitialize();
    // return true if event was handled
    virtual bool OnEvent(const Event &event);

    virtual void OnFinalize();

    virtual void OnUpdate(float dt);

    void Stop();

private:
    void SetEngine(Engine *engine);

};

}; // namespace StraitX::

extern StraitX::ApplicationConfig GetApplicationConfig();

#endif // STRAITX_APPLICATION_HPP
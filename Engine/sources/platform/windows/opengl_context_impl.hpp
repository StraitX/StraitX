#ifndef STRAITX_WINDOWS_OPENGL_CONTEXT_IMPL_HPP
#define STRAITX_WINDOWS_OPENGL_CONTEXT_IMPL_HPP

#include "platform/windows/window_impl.hpp"
#include "platform/result.hpp"
#include "platform/types.hpp"

struct HGLRC__;
struct HDC__;

namespace StraitX {
namespace Windows{

class OpenGLContextImpl {
private:
    HGLRC__ *m_Handle = nullptr;
    HDC__ *m_DeviceContext = nullptr;
    HWND__ *m_WindowHandle = nullptr;
public:
    OpenGLContextImpl() = default;

    Result Create(WindowImpl& window, const Version& version);

    Result CreateDummy();

    void Destroy();

    void DestroyDummy();

    Result MakeCurrent();

    void SwapBuffers();
private:
    size_t ChooseBestFormat(int formats[], unsigned int count);
};  

} // namespace Windows::
} // namespace StraitX::

#endif // STRAITX_WINDOWS_OPENGL_CONTEXT_IMPL_HPP
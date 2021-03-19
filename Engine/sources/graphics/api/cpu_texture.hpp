#ifndef STRAITX_CPU_TEXTURE_HPP
#define STRAITX_CPU_TEXTURE_HPP

#include "core/math/vector2.hpp"
#include "graphics/api/cpu_buffer.hpp"
#include "graphics/api/format.hpp"

namespace StraitX{

class CPUTexture: private CPUBuffer{
private:
    TextureFormat m_Format = {};
    u32 m_Width = 0;
    u32 m_Height = 0;
public:
    void New(u32 width, u32 height, TextureFormat format);

    void New(u32 width, u32 height, TextureFormat format, const void *pixels);

    void Delete();

    constexpr void *Pointer()const;

    void CopyData(const void *source);

    constexpr GPUResourceHandle Handle()const;

    constexpr Vector2u Size()const;

    constexpr TextureFormat Format()const;
};

sx_inline void CPUTexture::New(u32 width, u32 height, TextureFormat format){
    CPUBuffer::New(width * height * GetPixelSize(format));
    m_Format = format;
    m_Width = width;
    m_Height = height;
}

sx_inline void CPUTexture::New(u32 width, u32 height, TextureFormat format, const void *pixels){
    New(width, height, format);
    CopyData(pixels);
}

sx_inline void CPUTexture::Delete(){
    CPUBuffer::Delete();
}

constexpr void *CPUTexture::Pointer()const{
    return CPUBuffer::Pointer();
}

sx_inline void CPUTexture::CopyData(const void *source){
    CPUBuffer::CopyData(source, m_Width * m_Height * GetPixelSize(m_Format));
}

constexpr GPUResourceHandle CPUTexture::Handle()const{
    return CPUBuffer::Handle();
}

constexpr Vector2u CPUTexture::Size()const{
    return {m_Width, m_Height};
}

constexpr TextureFormat CPUTexture::Format()const{
    return m_Format;
}

}//namespace StraitX::

#endif//STRAITX_CPU_TEXTURE_HPP
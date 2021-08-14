#include "subsystems/imgui/imgui_backend.hpp"
#include "subsystems/imgui/imgui.hpp"
#include "graphics/cpu_texture.hpp"
#include "graphics/dma.hpp"
#include "core/string.hpp"
#include "core/os/keyboard.hpp"
#include "main/engine.hpp"

ShaderBinding ImGuiBackend::s_Bindings[2] = {
	{0, 1, ShaderBindingType::Texture, Shader::Fragment},
	{1, 1, ShaderBindingType::UniformBuffer, Shader::Vertex},
};

VertexAttribute ImGuiBackend::s_Attributes[3]={
	VertexAttribute::Float32x2,
	VertexAttribute::Float32x2,
	VertexAttribute::UNorm8x4,
};

const char *s_VertexShader = R"(
#version 440 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;

layout(binding = 1) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

layout(location = 0)out vec4 Color;
layout(location = 1)out vec2 UV;

void main()
{
	Color = aColor;
    UV = aUV;
    gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
	gl_Position.y *= -1.0;
}
)";

const char *s_FragmentShader = R"(
#version 440 core
layout(location = 0) out vec4 fColor;
layout(binding = 0) uniform sampler2D sTexture;

layout(location = 0)in vec4 Color;
layout(location = 1)in vec2 UV;

void main()
{
    fColor = Color * texture(sTexture, UV.st);
}
)";

static ImGuiBackend s_ImGuiBackend;

void ImGuiBackend::Register(EngineDelegates &delegates){
	delegates.OnBeginFrame.Bind<ImGuiBackend, &ImGuiBackend::OnBeginFrame>(&s_ImGuiBackend);
	delegates.OnUpdate.Bind<ImGuiBackend, &ImGuiBackend::OnUpdate>(&s_ImGuiBackend);
	delegates.OnEvent.Bind<ImGuiBackend, &ImGuiBackend::OnEvent>(&s_ImGuiBackend);
	delegates.OnEndFrame.Bind<ImGuiBackend, &ImGuiBackend::OnEndFrame>(&s_ImGuiBackend);
	delegates.OnFinalize.Bind<ImGuiBackend, &ImGuiBackend::OnFinalize>(&s_ImGuiBackend);

	s_ImGuiBackend.Initialize();
}

void ImGuiBackend::Initialize(){
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();

	static constexpr float s_BaseDPI = 93;
	auto screen_dpi = Engine::Get().MainWindow().Screen().DPI;

	ImGui::StyleColorsDark();
	ImGui::GetStyle().WindowRounding = 8;
	ImGui::GetStyle().ScaleAllSizes((screen_dpi.x > screen_dpi.y ? screen_dpi.x : screen_dpi.y) / s_BaseDPI);

    io.BackendPlatformName = "StraitX ImGui Backend";

	io.DisplayFramebufferScale = ImVec2(1, 1);


	io.KeyMap[ImGuiKey_Tab] = (int)Key::Tab;
    io.KeyMap[ImGuiKey_LeftArrow] = (int)Key::Left;
    io.KeyMap[ImGuiKey_RightArrow] = (int)Key::Right;
    io.KeyMap[ImGuiKey_UpArrow] = (int)Key::Up;
    io.KeyMap[ImGuiKey_DownArrow] = (int)Key::Down;
    io.KeyMap[ImGuiKey_PageUp] = (int)Key::PageUp;
    io.KeyMap[ImGuiKey_PageDown] = (int)Key::PageDown;
    io.KeyMap[ImGuiKey_Home] = (int)Key::Home;
    io.KeyMap[ImGuiKey_End] = (int)Key::End;
    io.KeyMap[ImGuiKey_Insert] = (int)Key::Insert;
    io.KeyMap[ImGuiKey_Delete] = (int)Key::Delete;
    io.KeyMap[ImGuiKey_Backspace] = (int)Key::Backspace;
    io.KeyMap[ImGuiKey_Space] = (int)Key::Space;
    io.KeyMap[ImGuiKey_Enter] = (int)Key::Enter;
    io.KeyMap[ImGuiKey_Escape] = (int)Key::Escape;
    io.KeyMap[ImGuiKey_KeyPadEnter] = (int)Key::KeypadEnter;
    io.KeyMap[ImGuiKey_A] = (int)Key::A;
    io.KeyMap[ImGuiKey_C] = (int)Key::C;
    io.KeyMap[ImGuiKey_V] = (int)Key::V;
    io.KeyMap[ImGuiKey_X] = (int)Key::X;
    io.KeyMap[ImGuiKey_Y] = (int)Key::Y;
    io.KeyMap[ImGuiKey_Z] = (int)Key::Z;

	{
		unsigned char *pixels = nullptr;
		int width = 0;
		int height = 0;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		CPUTexture staging_font_texture;
		staging_font_texture.New(width, height, TextureFormat::RGBA8, pixels);


		SamplerProperties font_text_sampler_props;
		font_text_sampler_props.MagFiltering = FilteringMode::Nearest;
		font_text_sampler_props.MinFiltering = FilteringMode::Nearest;

		m_ImGuiFont.New(width, height, TextureFormat::RGBA8, TextureUsageBits((int)TextureUsageBits::Sampled | (int)TextureUsageBits::TransferDst), font_text_sampler_props);
		DMA::ChangeLayout(m_ImGuiFont, TextureLayout::TransferDstOptimal);
		DMA::Copy(staging_font_texture, m_ImGuiFont);
		DMA::ChangeLayout(m_ImGuiFont, TextureLayout::ShaderReadOnlyOptimal);

		io.Fonts->SetTexID(ImTextureID(m_ImGuiFont.Handle().U64));
	}

	{
		m_DescriptorSetLayout = new DescriptorSetLayout(s_Bindings);

		m_DescriptorSetPool = DescriptorSetPool::New(m_DescriptorSetLayout, 1);

		m_Set = m_DescriptorSetPool->AllocateSet();
	}

	{
		m_Shaders[0] = Shader::New(Shader::Vertex, Shader::Lang::GLSL, (const u8*)s_VertexShader, String::Length(s_VertexShader));		
		m_Shaders[1] = Shader::New(Shader::Fragment, Shader::Lang::GLSL, (const u8*)s_FragmentShader, String::Length(s_FragmentShader));	

		SX_ASSERT(m_Shaders[0]->IsValid());
		SX_ASSERT(m_Shaders[1]->IsValid());

		GraphicsPipelineProperties props;
		props.Shaders = m_Shaders;
		props.VertexAttributes = s_Attributes;
		props.PrimitivesTopology = PrimitivesTopology::Triangles;
		props.RasterizationMode = RasterizationMode::Fill;
		props.BlendFunction = BlendFunction::Add;
		props.DepthFunction = DepthFunction::Always;
		props.SrcBlendFactor = BlendFactor::SrcAlpha;
		props.DstBlendFactor = BlendFactor::OneMinusSrcAlpha;
		props.Pass = GraphicsContext::Get().FramebufferPass();
		props.DescriptorSetLayout = m_DescriptorSetLayout; 	

		m_Pipeline = GraphicsPipeline::New(props);

		SX_ASSERT(m_Pipeline->IsValid());
 	}

	m_UniformBuffer.New(sizeof(Uniform), GPUMemoryType::DynamicVRAM, GPUBuffer::TransferDestination | GPUBuffer::UniformBuffer);
	m_Set->UpdateTextureBinding(0, 0, m_ImGuiFont);
	m_Set->UpdateUniformBinding(1, 0, m_UniformBuffer);

	m_VertexBuffer.New(40*sizeof(ImDrawVert), GPUMemoryType::DynamicVRAM, GPUBuffer::VertexBuffer | GPUBuffer::TransferDestination);
	m_IndexBuffer.New(40*sizeof(ImDrawIdx), GPUMemoryType::DynamicVRAM, GPUBuffer::IndexBuffer | GPUBuffer::TransferDestination);
}

void ImGuiBackend::OnBeginFrame(){
	ImGuiIO& io = ImGui::GetIO();

	auto window_size = Engine::Get().MainWindow().Size();

	io.DisplaySize = ImVec2((float)window_size.x, (float)window_size.y);
	io.DeltaTime = 0.016;

	auto mouse_pos = Mouse::RelativePosition(Engine::Get().MainWindow());

	io.MousePos = ImVec2((float)mouse_pos.x/io.DisplayFramebufferScale.x, (float)mouse_pos.y/io.DisplayFramebufferScale.y);

	if(Mouse::IsButtonPressed(Mouse::Left))
		io.MouseDown[ImGuiMouseButton_Left] = true;
	if(Mouse::IsButtonPressed(Mouse::Right))
		io.MouseDown[ImGuiMouseButton_Right] = true;
	if(Mouse::IsButtonPressed(Mouse::Middle))
		io.MouseDown[ImGuiMouseButton_Middle] = true;

	ImGui::NewFrame();
}

void ImGuiBackend::OnUpdate(float dt){
	ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = dt;
}

void ImGuiBackend::OnEndFrame(){
	ImGui::Render();

	const ImDrawData *data = ImGui::GetDrawData();

	auto window_size = Engine::Get().MainWindow().Size();
	ImVec2 clip_off = data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = data->FramebufferScale;

	Uniform uniform;
	uniform.u_Scale.x = clip_scale.x * 2.0f / data->DisplaySize.x;
	uniform.u_Scale.y = clip_scale.y * 2.0f / data->DisplaySize.y;
	uniform.u_Translate.x = -1.0f - data->DisplayPos.x * uniform.u_Scale.x;
	uniform.u_Translate.y = -1.0f - data->DisplayPos.x * uniform.u_Scale.y;

	DMA::Copy(&uniform, m_UniformBuffer);


	for(int i = 0; i<data->CmdListsCount; i++){

		ImDrawList* cmd_list = data->CmdLists[i];

		size_t vertex_size = cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
		size_t index_size = cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);

		if(vertex_size > m_VertexBuffer.Size())
			m_VertexBuffer.Realloc(vertex_size);

		if(index_size > m_IndexBuffer.Size())
			m_IndexBuffer.Realloc(index_size);

		DMA::Copy(cmd_list->VtxBuffer.Data, m_VertexBuffer, vertex_size);
		DMA::Copy(cmd_list->IdxBuffer.Data, m_IndexBuffer, index_size);

		m_CmdBuffer.BindPipeline(m_Pipeline);
		m_CmdBuffer.BindDescriptorSet(m_Set);
		m_CmdBuffer.BindVertexBuffer(m_VertexBuffer);
		m_CmdBuffer.BindIndexBuffer(m_IndexBuffer, IndicesType::Uint16);

		m_CmdBuffer.SetViewport(window_size.x, window_size.y, 0, 0);

		m_CmdBuffer.BeginRenderPass(GraphicsContext::Get().FramebufferPass(), GraphicsContext::Get().CurrentFramebuffer());

		for(const auto &cmd: cmd_list->CmdBuffer){
			ImVec4 clip_rect;
			clip_rect.x = (cmd.ClipRect.x - clip_off.x) * clip_scale.x;
			clip_rect.y = (cmd.ClipRect.y - clip_off.y) * clip_scale.y;
			clip_rect.z = (cmd.ClipRect.z - clip_off.x) * clip_scale.x;
			clip_rect.w = (cmd.ClipRect.w - clip_off.y) * clip_scale.y;

			if (clip_rect.x < 0.0f)
				clip_rect.x = 0.0f;
			if (clip_rect.y < 0.0f)
				clip_rect.y = 0.0f;


			Vector2f offset(clip_rect.x, clip_rect.y);
			Vector2f extent(clip_rect.z - clip_rect.x, clip_rect.w - clip_rect.y);

			m_CmdBuffer.SetScissors(extent.x, extent.y, offset.x, offset.y);

			m_CmdBuffer.DrawIndexed(cmd.ElemCount, cmd.IdxOffset);
		}
		m_CmdBuffer.EndRenderPass();

		GraphicsContext::Get().ExecuteCmdBuffer(m_CmdBuffer);
		m_CmdBuffer.Reset();
	}

	ImGuiIO& io = ImGui::GetIO();

	for(int i = 0; i<lengthof(io.MouseDown); i++)
		io.MouseDown[i] = false;
		
}

void ImGuiBackend::OnEvent(const Event &e){
	ImGuiIO &io = ImGui::GetIO();

	switch(e.Type){
	case EventType::TextEntered:
		io.AddInputCharacter(e.TextEntered.Unicode);
		break;
	case EventType::KeyPress:
		io.KeysDown[(size_t)e.KeyPress.KeyCode] = true;
		
		io.KeyAlt = io.KeysDown[(size_t)Key::LeftAlt] || io.KeysDown[(size_t)Key::RightAlt];
		io.KeyShift = io.KeysDown[(size_t)Key::LeftShift] || io.KeysDown[(size_t)Key::RightShift];
		io.KeyCtrl = io.KeysDown[(size_t)Key::LeftControl] || io.KeysDown[(size_t)Key::RightControl];
		io.KeySuper = io.KeysDown[(size_t)Key::LeftSuper] || io.KeysDown[(size_t)Key::RightSuper];
		break;
	case EventType::KeyRelease:
		io.KeysDown[(size_t)e.KeyRelease.KeyCode] = false;

		io.KeyAlt = io.KeysDown[(size_t)Key::LeftAlt] || io.KeysDown[(size_t)Key::RightAlt];
		io.KeyShift = io.KeysDown[(size_t)Key::LeftShift] || io.KeysDown[(size_t)Key::RightShift];
		io.KeyCtrl = io.KeysDown[(size_t)Key::LeftControl] || io.KeysDown[(size_t)Key::RightControl];
		io.KeySuper = io.KeysDown[(size_t)Key::LeftSuper] || io.KeysDown[(size_t)Key::RightSuper];
		break;
	case EventType::FocusOut:
	case EventType::FocusIn:
		for(int i = 0; i<lengthof(io.KeysDown); i++)
			io.KeysDown[i] = 0;
		io.KeyAlt = false;
		io.KeyShift = false;
		io.KeyCtrl = false;
		io.KeySuper = false;
		break;
	case EventType::MouseWheel:
    	io.MouseWheel += (float)e.MouseWheel.Delta/2.f;
		break;
	default:
		(void)0;
	}
}

void ImGuiBackend::OnFinalize(){
	m_UniformBuffer.Delete();
	m_VertexBuffer.Delete();
	m_IndexBuffer.Delete();

	m_DescriptorSetPool->FreeSet(m_Set);

	DescriptorSetPool::Delete(m_DescriptorSetPool);

	delete m_DescriptorSetLayout;

	Shader::Delete((Shader *)m_Shaders[0]);
	Shader::Delete((Shader *)m_Shaders[1]);

	GraphicsPipeline::Delete(m_Pipeline);
	m_ImGuiFont.Delete();
}
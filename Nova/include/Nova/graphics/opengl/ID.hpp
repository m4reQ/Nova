#pragma once
#include <glad/gl.h>
#include <Nova/core/Utility.hpp>

namespace Nova
{
	struct ShaderProgramID : public StrongTypedef<ShaderProgramID, GLuint>
	{
		using StrongTypedef::StrongTypedef;
	};
	
	struct BufferID : public StrongTypedef<BufferID, GLuint>
	{
		using StrongTypedef::StrongTypedef;
	};

	class VertexArrayID : public StrongTypedef<VertexArrayID, GLuint>
	{
		using StrongTypedef::StrongTypedef;
	};

	class FramebufferID : public StrongTypedef<FramebufferID, GLuint>
	{
		using StrongTypedef::StrongTypedef;
	};
}
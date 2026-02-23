#pragma once
#include <Nova/graphics/opengl/Buffer.hpp>
#include <Nova/graphics/opengl/GLObject.hpp>
#include <Nova/graphics/opengl/ID.hpp>
#include <Nova/graphics/opengl/VertexDescriptor.hpp>
#include <vector>
#include <optional>
#include <iterator>
#include <xutility>
#include <unordered_map>

namespace Nova
{
	struct VertexInput
	{
		GLuint Stride;
		std::vector<VertexDescriptor> Descriptors;
		std::optional<BufferID> BufferID = std::nullopt;
		GLint Offset = 0;
		GLint InstanceDivisor = 0;
	};

	class VertexArray : public GLObject<GL_VERTEX_ARRAY, VertexArrayID>
	{
	public:
		VertexArray() = default;
		
		VertexArray(const VertexArray&) = delete;

		VertexArray(VertexArray&& other) noexcept;

		VertexArray(GLuint id) noexcept;

		VertexArray(std::span<const VertexInput> layout);

		VertexArray(
			std::span<const VertexInput> layout,
			const Buffer& elementBuffer);

		VertexArray(std::initializer_list<VertexInput> layout);
		
		VertexArray(
			std::initializer_list<VertexInput> layout,
			const Buffer& elementBuffer);

		void AddVertexInput(const VertexInput &vertexInput);

		void AddVertexInput(
			GLuint stride,
			const std::vector<VertexDescriptor> &descriptors,
			BufferID bufferID,
			GLint offset,
			GLint instanceDivisor);

		void Use() const noexcept;

		void BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset = 0);

		void BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset, GLuint instanceDivisor);

		void BindElementBuffer(const Buffer &buffer) const noexcept;

		void Delete() noexcept override;

		VertexArray& operator=(const VertexArray&) = delete;
		
		VertexArray& operator=(VertexArray&& other) noexcept;

	private:
		std::unordered_map<GLuint, GLuint> m_BufferBindings;
		std::vector<GLuint> m_UsedBufferBindings;

		GLuint FindNextFreeBindingIndex();
	};
}

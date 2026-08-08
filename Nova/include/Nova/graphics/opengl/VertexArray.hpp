#pragma once
#include <Nova/graphics/opengl/VertexDescriptor.hpp>
#include <Nova/graphics/opengl/Buffer.hpp>
#include <vector>
#include <optional>
#include <iterator>
#include <xutility>
#include <unordered_map>
#include <glad/gl.h>

namespace Nova
{
	struct VertexInput
	{
		GLuint Stride;
		std::vector<VertexDescriptor> Descriptors;
		std::optional<GLuint> BufferID = std::nullopt;
		GLint Offset = 0;
		GLint InstanceDivisor = 0;
	};

	class VertexArray
	{
	public:
		VertexArray() = default;

		VertexArray(const VertexArray &) = delete;

		VertexArray(VertexArray &&other) noexcept;

		VertexArray(GLuint id) noexcept;

		VertexArray(std::span<const VertexInput> layout);

		VertexArray(
			std::span<const VertexInput> layout,
			const Buffer &elementBuffer);

		VertexArray(std::initializer_list<VertexInput> layout);

		VertexArray(
			std::initializer_list<VertexInput> layout,
			const Buffer &elementBuffer);

		~VertexArray() noexcept;

		void AddVertexInput(const VertexInput &vertexInput);

		void AddVertexInput(
			GLuint stride,
			const std::vector<VertexDescriptor> &descriptors,
			GLuint bufferID,
			GLint offset,
			GLint instanceDivisor);

		void Use() const noexcept;

		void BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset = 0);

		void BindVertexBuffer(const Buffer &buffer, GLuint bindingIndex, GLsizei stride, GLintptr offset, GLuint instanceDivisor);

		void BindElementBuffer(const Buffer &buffer) const noexcept;

		VertexArray &operator=(const VertexArray &) = delete;

		VertexArray &operator=(VertexArray &&other) noexcept
		{
			bufferBindings_ = std::move(other.bufferBindings_);
			usedBufferBindings_ = std::move(other.usedBufferBindings_);
			id_ = std::exchange(other.id_, 0);

			return *this;
		}

	private:
		std::unordered_map<GLuint, GLuint> bufferBindings_;
		std::vector<GLuint> usedBufferBindings_;
		GLuint id_ = 0;

		GLuint FindNextFreeBindingIndex();
	};
}

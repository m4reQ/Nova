#pragma once
#include <Nova/graphics/opengl/GLObject.hpp>
#include <Nova/graphics/opengl/ID.hpp>
#include <Nova/graphics/opengl/BufferView.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <span>
#include <vector>
#include <array>
#include <type_traits>

namespace Nova
{
	enum class BindingTarget
	{
		ArrayBuffer = GL_ARRAY_BUFFER,
		ElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,
		CopyReadBuffer = GL_COPY_READ_BUFFER,
		CopyWriteBuffer = GL_COPY_WRITE_BUFFER,
		PixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER,
		PixelPackBuffer = GL_PIXEL_PACK_BUFFER,
		QueryBuffer = GL_QUERY_BUFFER,
		TextureBuffer = GL_TEXTURE_BUFFER,
		TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
		UniformBuffer = GL_UNIFORM_BUFFER,
		DrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
		AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
		DispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER,
		ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER,
	};

	enum BufferFlags
	{
		None = 0,
		DynamicStorage = GL_DYNAMIC_STORAGE_BIT,
		Persistent = GL_MAP_PERSISTENT_BIT,
		Coherent = GL_MAP_COHERENT_BIT,
		ClientStorage = GL_CLIENT_STORAGE_BIT,
		Read = GL_MAP_READ_BIT,
		Write = GL_MAP_WRITE_BIT,
		ReadWrite = GL_MAP_READ_BIT | GL_MAP_WRITE_BIT,
	};

	enum BufferMapFlags
	{
		MapNone = 0,
		MapRead = GL_MAP_READ_BIT,
		MapWrite = GL_MAP_WRITE_BIT,
		MapReadWrite = Read | Write,
		MapPersistent = GL_MAP_PERSISTENT_BIT,
		MapCoherent = GL_MAP_COHERENT_BIT,
		MapInvalidateRange = GL_MAP_INVALIDATE_RANGE_BIT,
		MapInvalidateBuffer = GL_MAP_INVALIDATE_BUFFER_BIT,
		MapFlushExplicit = GL_MAP_FLUSH_EXPLICIT_BIT,
		MapUnsynchronized = GL_MAP_UNSYNCHRONIZED_BIT,
	};

	class Buffer : public GLObject<GL_BUFFER, BufferID>
	{
	public:
		Buffer() = default;
		Buffer(
			GLsizeiptr size,
			bool isWritable = false,
			bool isReadable = false,
			const void *data = nullptr);

		void Delete() noexcept override;

		template <typename T = void>
		constexpr BufferView<T> GetView(GLsizeiptr size = (GLsizeiptr)-1, GLintptr offset = 0)
		{
			return BufferView<T>(m_ID, m_BaseDataPtr, size, offset);
		}
		
		constexpr void *GetDataPtr() noexcept { return m_DataPtr; }

		constexpr const void *GetDataPtr() const noexcept { return m_DataPtr; }

		template <typename T>
		constexpr T *GetDataPtr() noexcept { return reinterpret_cast<T *>(m_DataPtr); }

		template <typename T>
		constexpr const T *GetDataPtr() noexcept { return reinterpret_cast<const T *>(m_DataPtr); }

		void Store(const void *data, std::size_t sizeBytes) noexcept;

		void Reset() { m_DataPtr = m_BaseDataPtr; }

		template <typename T>
		void Store(std::span<T> data)
		{
			Store(data.data(), data.size_bytes());
		}

		template <
			glm::length_t C,
			glm::length_t R,
			typename T,
			glm::qualifier Q = glm::qualifier::defaultp>
		void Store(const glm::mat<C, R, T, Q>& matrix)
		{
			Store(glm::value_ptr(matrix), sizeof(matrix));
		}

		template <
			glm::length_t L,
			typename T,
			glm::qualifier Q = glm::qualifier::defaultp>
		void Store(const glm::vec<L, T, Q>& vector)
		{
			Store(glm::value_ptr(vector), sizeof(vector));
		}

		template <
			typename T,
			glm::qualifier Q = glm::qualifier::defaultp>
		void Store(const glm::qua<T, Q>& quaternion)
		{
			Store(glm::value_ptr(quaternion), sizeof(quaternion));
		}

		void Commit() noexcept;

		void Commit(GLsizeiptr sizeBytes, GLsizeiptr offset) noexcept;

		void Bind(BindingTarget target) const;

		void Bind(BindingTarget target, GLuint index) const;

		void Bind(BindingTarget target, GLuint index, GLintptr offset, GLsizeiptr size) const;

		void AdvanceDataPtr(GLsizeiptr nBytes) noexcept;

		constexpr GLsizeiptr GetDataSize() const noexcept { return (std::uint8_t *)m_DataPtr - (std::uint8_t *)m_BaseDataPtr; }

		constexpr GLsizeiptr GetSize() const noexcept { return m_Size; }

		constexpr bool IsReadable() const noexcept { return m_IsReadable; }

		constexpr bool IsWritable() const noexcept { return m_IsWritable; }

	private:
		void *m_BaseDataPtr = nullptr;
		void *m_DataPtr = nullptr;
		GLsizeiptr m_Size;
		bool m_IsReadable;
		bool m_IsWritable;
	};
}

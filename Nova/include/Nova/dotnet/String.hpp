#pragma once
#include <Nova/dotnet/DotnetMemory.hpp>
#include <Nova/core/Build.hpp>
#include <string_view>
#include <utility>

namespace Nova
{
	template <typename T>
	class DotnetString
	{
	public:
		static DotnetString<T> Create(const std::basic_string_view<T> string)
		{
			const size_t size = string.length() * sizeof(T);
			T *data = (T*)DotnetMemory::AllocHGlobal(size);
			memcpy(data, string.data(), size);

			data[string.length()] = '\0';

			return DotnetString(data, true);
		}

		static DotnetString<T> CreateStack(const std::basic_string_view<T> string) noexcept
		{
			return DotnetString(string.data(), false);
		}

		DotnetString(const std::basic_string_view<T> string, bool stackAllocated = false)
			: isDisposed_(false),
			  shouldDispose_(!stackAllocated)
		{
			if (stackAllocated)
				data_ = const_cast<T*>(string.data());
			else
			{
				const auto size = string.length() * sizeof(T);
				data_ = static_cast<T*>(DotnetMemory::AllocHGlobal(size));
				std::memcpy(data_, string.data(), size);

				data_[string.length()] = '\0';
			}
		}

		DotnetString(DotnetString<T>&& other) noexcept
			: data_(std::exchange(other.data_, nullptr)),
			  isDisposed_(std::exchange(other.isDisposed_, true)),
			  shouldDispose_(std::exchange(other.shouldDispose_, false)) { }

		~DotnetString() noexcept
		{
			if (shouldDispose_ && !isDisposed_)
			{
				DotnetMemory::FreeHGlobal(data_);
				isDisposed_ = true;
			}
		}

		DotnetString& operator=(DotnetString&& other) noexcept
		{
			data_ = std::exchange(other.data_, nullptr);
			isDisposed_ = std::exchange(other.isDisposed_, true);
			shouldDispose_ = std::exchange(other.shouldDispose_, false);

			return *this;
		}

		std::basic_string<T> GetString() noexcept { return data_; }

		constexpr const std::basic_string_view<T> GetView() const noexcept { return data_; }

	private:
		T* data_;
		bool shouldDispose_;
		bool isDisposed_;
	};

	using DotnetANSIString = DotnetString<char>;
	using DotnetUTF8String = DotnetString<wchar_t>;
}

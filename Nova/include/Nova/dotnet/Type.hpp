#pragma once
#include <Nova/dotnet/TypeInfo.hpp>
#include <Nova/dotnet/Host.hpp>
#include <string_view>

namespace Nova
{
	class DotnetType
	{
	public:
		DotnetType() = default;
		
		constexpr DotnetType(DotnetType&& other) noexcept = default;

		constexpr DotnetType(TypeInfo&& typeInfo) noexcept
			: info_(std::move(typeInfo)) { }

		template <typename TReturn, typename... Args>
		TReturn InvokeStaticMethod(const std::string_view methodName, Args&&... args)
		{
			TReturn result{};
			const auto methodHash = GetID() ^ ComputeMethodHash<TReturn, Args...>(methodName);

			if constexpr (sizeof...(args) == 0)
			{
				Dotnet::InvokeStaticMethod_(methodHash, &result, nullptr);
			}
			else
			{
				const void* argsArray[] = { (&args)... };
				Dotnet::InvokeStaticMethod_(methodHash, &result, argsArray);
			}

			return result;
		}

		template <typename... Args>
		void InvokeStaticMethod(const std::string_view methodName, Args&&... args) const
		{
			const int32_t methodHash = GetID() ^ ComputeMethodHash<void, Args...>(methodName);

			if constexpr (sizeof...(args) == 0)
			{
				Dotnet::InvokeStaticMethod_(methodHash, nullptr, nullptr);
			}
			else
			{
				const void* argsArray[] = { (&args)... };
				Dotnet::InvokeStaticMethod_(methodHash, nullptr, argsArray);
			}
		}

		constexpr int32_t GetID() const noexcept { return info_.GetID(); }
		
		constexpr const std::string_view GetName() const noexcept { return info_.GetName(); }
		
		constexpr const std::string_view GetFullName() const noexcept { return info_.GetFullName(); }
		
		constexpr const std::string_view GetAssemblyQualifiedName() const noexcept { return info_.GetAssemblyQualifiedName(); }

		constexpr DotnetType& operator=(DotnetType&& other) noexcept = default;

	private:
		TypeInfo info_;

		static constexpr int32_t ComputeStringHash(const char* string) noexcept
		{
			int32_t hash = 5381;
			while (*string)
			{
				hash = *string + 33 * hash;
				string++;
			}

			return hash;
		}

		template <typename T>
		static constexpr int32_t ComputeTypeHash() noexcept
		{
			if constexpr (std::is_same_v<T, int8_t>)
				return 3;
			if constexpr (std::is_same_v<T, uint8_t>)
				return 7;
			if constexpr (std::is_same_v<T, int16_t>)
				return 11;
			if constexpr (std::is_same_v<T, uint16_t>)
				return 13;
			if constexpr (std::is_same_v<T, int32_t>)
				return 17;
			if constexpr (std::is_same_v<T, uint32_t>)
				return 19;
			if constexpr (std::is_same_v<T, int64_t>)
				return 23;
			if constexpr (std::is_same_v<T, uint64_t>)
				return 29;
			if constexpr (std::is_same_v<T, bool>)
				return 31;
			if constexpr (std::is_same_v<T, float>)
				return 37;
			if constexpr (std::is_same_v<T, double>)
				return 41;
			if constexpr (std::is_same_v<typename std::decay<T>::type, const char*>)
				return 47;

			return 0;
		}

		template <typename TReturn, typename... Args>
		static constexpr int32_t ComputeMethodHash(const std::string_view methodName) noexcept
		{
			int32_t hash = ComputeStringHash(methodName.data()) ^ ComputeTypeHash<TReturn>();

			if constexpr (sizeof...(Args) != 0)
				hash ^= (ComputeTypeHash<Args>() ^ ...);

			return hash;
		}
	};
}
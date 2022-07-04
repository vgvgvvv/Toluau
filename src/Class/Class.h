#pragma once
#include <cstdint>
#include <utility>

namespace ToLuau
{
	namespace ClassDetail
	{
		/* ========================================================================= */
		/* Hash                                                                      */
		/* ========================================================================= */
		static constexpr uint64_t kFNV1aValue = 0xcbf29ce484222325;
		static constexpr uint64_t kFNV1aPrime = 0x100000001b3;

		inline constexpr uint64_t
		Hash(char const* const str, uint64_t const value = kFNV1aValue) noexcept
		{
			return (str[0] == '\0')
			       ? value
			       : Hash(&str[1], (value ^ uint64_t(str[0])) * kFNV1aPrime);
		}
	}


	class Type
	{

	public:
		virtual ~Type() = default;

		Type(
				int size,
				char const* name) noexcept
				: TypeSize(size)
				, TypeHash(ClassDetail::Hash(name))
				, TypeName(name)
		{

		}

		/* --------------------------------------------------------------------- */
		/* Identifier                                                            */
		/* --------------------------------------------------------------------- */
		virtual bool IsClass() const noexcept { return false; }

		virtual bool IsEnum() const noexcept { return false; }

		virtual bool IsPointer() const noexcept { return false; }

		/* --------------------------------------------------------------------- */
		/* Access                                                                */
		/* --------------------------------------------------------------------- */
		uint64_t
		Size() const noexcept
		{
			return TypeSize;
		}

		uint64_t
		Hash() const noexcept
		{
			return TypeHash;
		}

		char const*
		Name() const noexcept
		{
			return TypeName;
		}

		/* --------------------------------------------------------------------- */
		/* Operator                                                              */
		/* --------------------------------------------------------------------- */
		bool
		operator==(Type const& other) const noexcept
		{
			return TypeHash == other.TypeHash;
		}

		bool
		operator!=(Type const& other) const noexcept
		{
			return !(*this == other);
		}


	protected:
		uint64_t TypeSize;
		uint64_t TypeHash;
		char const* TypeName;
	};


	class Class : public Type
	{

	public:
		Class(
				int InSize,
				const Class* InBaseClass,
				char const* InName) noexcept
				: Type(InSize, InName)
				, BaseClass(InBaseClass)
		{
			Defined = true;
		}


		/* --------------------------------------------------------------------- */
		/* Identifier                                                            */
		/* --------------------------------------------------------------------- */
		virtual bool IsClass() const noexcept override { return true; };


		/* --------------------------------------------------------------------- */
		/* Access                                                                */
		/* --------------------------------------------------------------------- */
		Class const* GetBaseClass() const noexcept
		{
			return BaseClass;
		}

		bool IsA(const Class* targetClass) const
		{
			if(targetClass == nullptr)
			{
				return false;
			}
			if(*this == *targetClass)
			{
				return true;
			}
			const auto baseClass = GetBaseClass();
			if (baseClass == nullptr)
			{
				return false;
			}
			return baseClass->IsA(targetClass);
		}


	protected:
		const Class* BaseClass;
		bool Defined;
	};


}



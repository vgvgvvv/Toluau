#pragma once
#include "IToLuauRegister.h"
#include "Toluau/ToLuau.h"
#include "Toluau/API/StackAPI.h"
#include "Toluau/Class/ClassName.h"
#include "Toluau/Containers/ToLuauVar.h"
#include "Toluau/Containers/ToLuauDelegate.h"
#include "Toluau/Containers/ToLuauFunction.h"
#include "Toluau/Containers/ToLuauArray.h"
#include "Toluau/Containers/ToLuauMap.h"
#include "Toluau/API/CppBinding.h"

#define __STATIC_REGISTER_LUAU_BEGIN(Name) \
namespace REGISTER_LUAU_INNTER_NAME_SPACE\
{ \
	class Name##Register : public ToLuau::ILuauStaticRegister \
	{ \
	public: \
		void LuaRegister(ToLuau::IToLuauRegister *Register) override \
		{

#define __STATIC_REGISTER_LUAU_END(Name) \
		} \
	}; \
	Name##Register _##Name##RegisterInstance; \
}

#define DEFINE_FRIEND_REGISTER_LUAU_BEGIN(Name)\
	

#define LUAU_BEGIN_CLASS(ClassName) \
	__STATIC_REGISTER_LUAU_BEGIN(ClassName) \
	Register->GetMetaData().PushClass(ToLuau::GetClassName<ClassName>(nullptr), ""); \
	Register->BeginClass<ClassName>();

#define LUAU_BEGIN_CLASS_WITH_SUPER(ClassName, SuperClass) \
	__STATIC_REGISTER_LUAU_BEGIN(ClassName)  \
	Register->GetMetaData().PushClass(ToLuau::GetClassName<ClassName>(nullptr), ToLuau::GetClassName<SuperClass>(nullptr)); \
	Register->BeginClass<ClassName, SuperClass>();

// #define LUAU_BEGIN_CLASS_BY_NAME(ClassRegName, ClassNameStr) \
// 	__STATIC_REGISTER_LUAU_BEGIN(ClassRegName) \
// 	Register->GetMetaData().PushClass(ClassNameStr, ""); \
// 	Register->BeginClass(ClassNameStr);
//
// #define LUAU_BEGIN_CLASS_WITH_SUPER_BY_NAME(ClassRegName, ClassNameStr, SuperClassNameStr) \
// 	__STATIC_REGISTER_LUAU_BEGIN(ClassRegName) \
// 	Register->GetMetaData().PushClass(ClassNameStr, SuperClassNameStr); \
// 	Register->BeginClass(ClassNameStr, SuperClassNameStr);

#define LUAU_REG_NEW_FUNC(ClassName, FuncName) \
	Register->GetMetaData().RegFunction<decltype(&ClassName::FuncName)>("new"); \
	Register->RegFunction("new", &ToLuau::LuaCppBinding<decltype(&ClassName::FuncName), &ClassName::FuncName>::LuaCFunction);

#define LUAU_REG_LAMBDA_NEW_FUNC(ClassName, ...) \
	{ \
		static auto Lambda = __VA_ARGS__; \
		using BindingType = LuaCallableBinding<decltype(Lambda)>::Prototype; \
		BindingType::Func = &Lambda; \
		Register->RegFunction("new", &BindingType::LuaCFunction); \
	}

#define LUAU_REG_LUA_NEW_FUNC(ClassName, FuncName) \
	Register->RegFunction("new", &ClassName::FuncName);

#define LUAU_REG_FUNC(ClassName, FuncName) \
	Register->GetMetaData().RegFunction<decltype(&ClassName::FuncName)>(#FuncName); \
	Register->RegFunction(#FuncName, &ToLuau::LuaCppBinding<decltype(&ClassName::FuncName), &ClassName::FuncName>::LuaCFunction);

#define LUAU_REG_FUNC_BY_NAME(ClassName, FuncName, Alias) \
	Register->GetMetaData().RegFunction<decltype(&ClassName::FuncName)>(#Alias); \
	Register->RegFunction(#Alias, &ToLuau::LuaCppBinding<decltype(&ClassName::FuncName), &ClassName::FuncName>::LuaCFunction);

#define LUAU_REG_LAMBDA_FUNC(FuncName, ...) \
	{ \
    	static auto Lambda = __VA_ARGS__; \
    	using BindingType = LuaCallableBinding<decltype(Lambda)>::Prototype; \
    	BindingType::Func = &Lambda; \
    	Register->RegFunction(#FuncName, &BindingType::LuaCFunction); \
    }

#define LUAU_REG_LAMBDA_FUNC_BY_META_TAG(Tag, ...) \
	{ \
		static auto Lambda = __VA_ARGS__; \
		using BindingType = LuaCallableBinding<decltype(Lambda)>::Prototype; \
		BindingType::Func = &Lambda; \
		Register->RegMetaFunction(MetatableEvent::Tag, &BindingType::LuaCFunction); \
	}

#define LUAU_REG_LUA_FUNC(ClassName, FuncName) \
	Register->RegFunction(#FuncName, &ClassName::FuncName);

#define LUAU_REG_LUA_FUNC_WITH_FUNC_TYPE(ClassName, FuncName, FuncType) \
	Register->GetMetaData().RegFunction<FuncType>(#FuncName); \
	Register->RegFunction(#FuncName, &ClassName::FuncName);	

#define LUAU_REG_VAR_WITH_GETTER_SETTER(ClassName, VarName) \
	Register->GetMetaData().RegVar(false, ToLuau::GetClassName<decltype(ClassName::VarName)>(nullptr), #VarName);\
	Register->RegVar(#VarName, \
		&ToLuau::LuaCppBinding<decltype(&ClassName::Set##VarName), &ClassName::Set##VarName>::LuaCFunction, \
		&ToLuau::LuaCppBinding<decltype(&ClassName::Get##VarName), &ClassName::Get##VarName>::LuaCFunction);

#define LUAU_REG_VAR(ClassName, VarName) \
	static_assert(std::is_member_object_pointer<decltype(&ClassName::VarName)>::value, "you must register non static member");\
	Register->GetMetaData().RegVar(false, ToLuau::GetClassName<decltype(ClassName::VarName)>(nullptr), #VarName);\
	Register->RegVar(#VarName, \
		[](lua_State* L) \
		{ \
			auto Owner = ToLuau::StackAPI::Check<ClassName*>(L, -2); \
			auto Value = ToLuau::StackAPI::Check<decltype(ClassName::VarName)>(L, -1); \
			Owner->VarName = Value; \
			return 0;\
		}, \
		[](lua_State* L) \
		{ \
			auto Owner = ToLuau::StackAPI::Check<ClassName*>(L, -1); \
			ToLuau::StackAPI::Push<decltype(ClassName::VarName)>(L, Owner->VarName); \
			return 1;\
		} \
		);

#define LUAU_REG_STATIC_VAR(ClassName, VarName) \
	static_assert(!std::is_member_object_pointer<decltype(&ClassName::VarName)>::value, "you must register static member");\
	Register->GetMetaData().RegVar(false, ToLuau::GetClassName<decltype(ClassName::VarName)>(nullptr), #VarName);\
	Register->RegVar(#VarName, \
	[](lua_State* L) \
	{ \
		ClassName::VarName = ToLuau::StackAPI::Check<decltype(ClassName::VarName)>(L, -1); \
		return 0;\
	}, \
	[](lua_State* L) \
	{ \
		ToLuau::StackAPI::Push<decltype(ClassName::VarName)>(L, ClassName::VarName); \
		return 1;\
	} \
	);

#define LUAU_REG_CONST_STATIC_VAR(ClassName, VarName) \
	static_assert(!std::is_member_object_pointer<decltype(&ClassName::VarName)>::value, "you must register static member");\
	Register->GetMetaData().RegVar(false, ToLuau::GetClassName<decltype(ClassName::VarName)>(nullptr), #VarName);\
	Register->RegVar(#VarName, \
	[](lua_State* L) \
	{ \
		return 0;\
	}, \
	[](lua_State* L) \
	{ \
		ToLuau::StackAPI::PushValue<std::remove_const<decltype(ClassName::VarName)>::type>(L, ClassName::VarName); \
		return 1;\
	} \
	);

#define LUAU_END_CLASS(ClassName) \
	Register->EndClass();\
	Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(ClassName)

#define LUAU_BEGIN_ENUM(EnumName) \
	__STATIC_REGISTER_LUAU_BEGIN(EnumName) \
	Register->GetMetaData().PushEnum(#EnumName); \
	Register->BeginEnum(#EnumName);

#define LUAU_REG_CLASS_ENUM_VAR(EnumName, Name) \
	Register->GetMetaData().RegVar(true, ToLuau::GetClassName<int32_t>(nullptr), #Name);\
	Register->RegVar(#Name, nullptr, [](lua_State* L){ return ToLuau::StackAPI::Push(L, EnumName::Name); });

#define LUAU_REG_ENUM_VAR(EnumName, Name) \
	Register->GetMetaData().RegVar(true, ToLuau::GetClassName<int32_t>(nullptr), #Name);\
	Register->RegVar(#Name, nullptr, [](lua_State* L){ return ToLuau::StackAPI::Push(L, Name); });

#define LUAU_END_ENUM(EnumName) \
	Register->EndEnum();        \
	Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(EnumName)

#define LUAU_BEGIN_STATIC_LIB(LibName) \
	__STATIC_REGISTER_LUAU_BEGIN(LibName) \
	Register->GetMetaData().PushClass(#LibName, ""); \
	Register->BeginStaticLib(#LibName);

#define LUAU_END_STATIC_LIB(LibName) \
	Register->EndStaticLib(); \
	Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(LibName)

#ifdef TOLUAUUNREAL_API

#define LUAU_REG_UCLASS(ClassName) \
	__STATIC_REGISTER_LUAU_BEGIN(ClassName) \
	Register->RegUClass(ClassName::StaticClass()); \
	__STATIC_REGISTER_LUAU_END(ClassName)

#define LUAU_BEGIN_UCLASS(ClassName) \
	__STATIC_REGISTER_LUAU_BEGIN(ClassName) \
	Register->RegUClass(ClassName::StaticClass()->GetSuperClass()); \
	Register->GetMetaData().PushUEClass(ClassName::StaticClass()); \
	Register->BeginUEClass(ClassName::StaticClass()); \

#define LUAU_END_UCLASS(ClassName) \
	Register->EndUEClass(); \
	Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(ClassName)

#define LUAU_REG_UENUM(EnumName) \
	__STATIC_REGISTER_LUAU_BEGIN(EnumName) \
	Register->RegUEnum(StaticEnum<EnumName>()); \
	__STATIC_REGISTER_LUAU_END(EnumName)

#define LUAU_REG_USTRUCT(StructName) \
	__STATIC_REGISTER_LUAU_BEGIN(StructName) \
	Register->RegUStruct(StructName::StaticStruct()); \
	__STATIC_REGISTER_LUAU_END(StructName)

#define LUAU_BEGIN_USTRUCT(StructName) \
	__STATIC_REGISTER_LUAU_BEGIN(StructName) \
	Register->RegUStruct(StructName::StaticStruct()->GetSuperStruct()); \
	Register->GetMetaData().PushUEStruct(StructName::StaticStruct()); \
	Register->BeginUEStruct(StructName::StaticStruct());

#define LUAU_END_USTRUCT(StructName) \
	Register->EndUEStruct(); \
	Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(StructName)

#endif

#define LUAU_CUSTOM_REG(ClassName, FuncName) \
	auto L = Register->GetOwner()->GetState(); \
	ClassName::FuncName(L);
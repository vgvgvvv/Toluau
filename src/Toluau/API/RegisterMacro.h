#pragma once
#include "IToLuauRegister.h"
#include "Toluau/ToLuau.h"

#define __STATIC_REGISTER_LUAU_BEGIN(Name) \
namespace \
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
#define LUAU_BEGIN_CLASS(ClassName) \
    __STATIC_REGISTER_LUAU_BEGIN(ClassName) \
    Register->GetMetaData().PushClass(#ClassName, ""); \
    Register->BeginClass(ClassName::StaticLuaClass());

#define LUAU_BEGIN_CLASS_WITH_SUPER(ClassName, SuperClass) \
    __STATIC_REGISTER_LUAU_BEGIN(ClassName, SuperClass)  \
	Register->GetMetaData().PushClass(#ClassName, #SuperClass); \
	Register->BeginClass(ClassName::StaticLuaClass(), SuperClass::StaticLuaClass());

#define LUAU_BEGIN_CLASS_BY_NAME(ClassRegName, ClassNameStr) \
 	__STATIC_REGISTER_LUAU_BEGIN(ClassRegName) \
    Register->GetMetaData().PushClass(ClassNameStr, ""); \
    Register->BeginClass(ClassNameStr);

#define LUAU_BEGIN_CLASS_WITH_SUPER_BY_NAME(ClassRegName, ClassNameStr, SuperClassNameStr) \
 	__STATIC_REGISTER_LUAU_BEGIN(ClassRegName) \
    Register->GetMetaData().PushClass(ClassNameStr, SuperClassNameStr); \
    Register->BeginClass(ClassNameStr, SuperClassNameStr);

#define LUAU_REG_NEW_FUNC(ClassName, FuncName) \
    Register->RegFunction("new", &ToLuau::LuaCppBinding<decltype(&ClassName::FuncName), &ClassName::FuncName>::LuaCFunction);

#define LUAU_REG_LUA_NEW_FUNC(ClassName, FuncName) \
    Register->RegFunction("new", &ClassName::FuncName);

#define LUAU_REG_FUNC(ClassName, FuncName) \
    Register->RegFunction(#FuncName, &ToLuau::LuaCppBinding<decltype(&ClassName::FuncName), &ClassName::FuncName>::LuaCFunction);

#define LUAU_REG_LUA_FUNC(ClassName, FuncName) \
    Register->RegFunction(#FuncName, &ClassName::FuncName);

#define LUAU_REG_VAR(ClassName, VarName) \
     Register->RegVar(#VarName, \
        &ToLuau::LuaCppBinding<decltype(&ClassName::Set##VarName), &ClassName::Set##VarName>::LuaCFunction, \
        &ToLuau::LuaCppBinding<decltype(&ClassName::Get##VarName), &ClassName::Get##VarName>::LuaCFunction);

#define LUAU_END_CLASS(ClassName) \
    Register->EndClass();         \
    Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(ClassName)

#define LUAU_BEGIN_ENUM(EnumName) \
    __STATIC_REGISTER_LUAU_BEGIN(EnumName) \
    Register->GetMetaData().PushEnum(#EnumName); \
	Register->BeginEnum(#EnumName);

#define LUAU_REG_CLASS_ENUM_VAR(EnumName, Name) \
    Register->RegVar(#Name, nullptr, [](lua_State* L){ return ToLuau::StackAPI::Push(L, EnumName::Name); });

#define LUAU_END_ENUM(EnumName) \
    Register->EndEnum();        \
    Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(EnumName)

#define LUAU_BEGIN_STATIC_LIB(LibName) \
    __STATIC_REGISTER_LUAU_BEGIN(LibName) \
    Register->GetMetaData().PushNamespace(#LibName); \
	Register->BeginStaticLib(#LibName);

#define LUAU_END_STATIC_LIB(LibName) \
    Register->EndStaticLib(); \
    Register->GetMetaData().Pop(); \
	__STATIC_REGISTER_LUAU_END(LibName)

#define LUAU_REG_UCLASS(ClassName) \
	__STATIC_REGISTER_LUAU_BEGIN(ClassName) \
	Register->RegUClass(ClassName::StaticClass()); \
	__STATIC_REGISTER_LUAU_END(ClassName)

#define LUAU_REG_UENUM(EnumName) \
	__STATIC_REGISTER_LUAU_BEGIN(EnumName) \
	Register->RegUEnum(StaticEnum<EnumName>()); \
	__STATIC_REGISTER_LUAU_END(EnumName)

#define LUAU_REG_USTRUCT(StructName) \
	__STATIC_REGISTER_LUAU_BEGIN(StructName) \
	Register->RegUStruct(StructName::StaticStruct()); \
	__STATIC_REGISTER_LUAU_END(StructName)

#define LUAU_CUSTOM_REG(ClassName, FuncName) \
    auto L = Register->GetOwner()->GetState(); \
    ClassName::FuncName(L);
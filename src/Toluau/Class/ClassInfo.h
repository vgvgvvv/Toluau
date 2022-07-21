#pragma once
#include <string>
#include "Class.h"


#define DEFINE_LUA_CLASS(className) \
public:\
	typedef className LuaClassType; \
	typedef void LuaSuperClass; \
	static const ToLuau::Class* StaticLuaClass() { return &GetSelfLuaClass(); }\
	virtual const ToLuau::Class* GetLuaClass() const { return className::StaticLuaClass(); }\
private:\
	static ToLuau::Class& GetSelfLuaClass(){               \
        static ToLuau::Class Instance(sizeof(className), \
		[](){ return nullptr; }, \
		#className);                      \
        return Instance;                                \
	};

#define DEFINE_LUA_DERIVED_CLASS(className, baseClassName) \
public: \
	typedef className LuaClassType; \
	typedef baseClassName LuaSuperClass; \
	static const ToLuau::Class* StaticLuaClass() { return &GetSelfLuaClass(); } \
	virtual const ToLuau::Class* GetLuaClass() const override { return className::StaticLuaClass(); } \
private:    \
	static ToLuau::Class& GetSelfLuaClass(){ \
        static ToLuau::Class Instance( sizeof(className), \
		[](){ return baseClassName::StaticLuaClass(); }, \
		#className);  \
		return Instance;\
	};



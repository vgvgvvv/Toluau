//
// Created by 35207 on 2022/6/26 0026.
//

#include "StackAPI.h"

#include "lua.h"

#include "Toluau/ToLuau.h"
#include "Toluau/Util/Util.h"
#include "IToLuauRegister.h"
#include "ToLuauLib.h"
#include "Toluau/Containers/ToLuauArray.h"
#include "Toluau/Containers/ToLuauMap.h"
#include "Toluau/Containers/ToLuauDelegate.h"

#define UD_TAG(Tag) static_cast<int32_t>(UserDataType::Tag)

namespace ToLuau
{
	
	
	namespace StackAPI
	{
#if LUAU_SUPPORT_HOYO_CLASS
		namespace HoYoClass
		{
			TMap<FHoYoClass, PushFunc> HoYoClassRegisterPushFuncMap;
			TMap<FHoYoClass, CheckFunc> HoYoClassRegisterCheckFuncMap;
		}
#endif
		
		int InstanceIndexSelf(lua_State* L)
		{
			lua_getmetatable(L,1);
			auto name = StackAPI::Check<std::string>(L, 2);
		
			lua_getfield(L,-1,name.c_str());
			lua_remove(L,-2); // remove mt of ud
			return 1;
		}
		
		int32_t SetupMetaTableCommon(lua_State* L)
		{
			lua_pushcfunction(L, InstanceIndexSelf, "InstanceIndexSelf");
			lua_setfield(L, -2, "__index");
			return 0;
		}
		
		int32_t SetClassMetaTable(lua_State *L, std::string& ClassName, lua_CFunction SetupMetaTable) // ud
		{
			auto Owner = ILuauState::GetByRawState(L);
			auto ClassRef = Owner->GetRegister().GetClassMetaRef(ClassName);
			if(ClassRef > 0)
			{
				lua_getref(L, ClassRef); // ud meta
				lua_setmetatable(L, -2); // ud
			}
			else if(SetupMetaTable != nullptr)
			{
				lua_newtable(L); // ud table
				
				SetupMetaTable(L);

				lua_pushstring(L, "__type"); // ud table __type
				lua_pushstring(L, ClassName.c_str()); // ud table __type name
				lua_rawset(L, -3); // ud table
				
				auto Ref = lua_ref(L, -1);
				Owner->GetRegister().SetClassMetaRef(ClassName, Ref);
				lua_setmetatable(L, -2); // ud
			}
			else
			{
				luaL_errorL(L, "cannot find class meta ref %s", ClassName.c_str());
			}
			return 1;
		}

		bool HasMetaTable(lua_State* L, std::string& ClassName)
		{
			auto Owner = ILuauState::GetByRawState(L);
			auto ClassRef = Owner->GetRegister().GetClassMetaRef(ClassName);
			return ClassRef > 0;
		}

#ifdef TOLUAUUNREAL_API
		bool RegClass(lua_State* L, UClass* Class)
		{
			const ILuauState* Owner = ILuauState::GetByRawState(L);
			auto& Register = Owner->GetRegister();
			Register.BeginModule("");
			Register.RegUClass(Class);
			Register.EndModule();
			return true;
		}

		bool RegStruct(lua_State* L, UScriptStruct* Struct)
		{
			const ILuauState* Owner = ILuauState::GetByRawState(L);
			auto& Register = Owner->GetRegister();
			Register.BeginModule("");
			Register.RegUStruct(Struct);
			Register.EndModule();
			return true;
		}
#endif


		void InitStackAPI()
		{
#ifdef TOLUAUUNREAL_API
			UE::InitProperty();
#endif
		}

		

		namespace Int64Detail
		{
			int gInt64MT = -1;

			static int64_t getInt64(lua_State* L, int idx)
			{
				if (void* p = lua_touserdatatagged(L, idx, static_cast<int32_t>(UserDataType::Int64)))
					return *static_cast<int64_t*>(p);

				if (lua_isnumber(L, idx))
					return lua_tointeger(L, idx);

				luaL_typeerror(L, idx, "int64");
			}

			static void pushInt64(lua_State* L, int64_t value)
			{
				void* p = lua_newuserdatatagged(L, sizeof(int64_t), static_cast<int32_t>(UserDataType::Int64));

				lua_getref(L, gInt64MT);
				lua_setmetatable(L, -2);

				*static_cast<int64_t*>(p) = value;
			}
		}
		
		int32_t RegisterInt64(lua_State* L)
		{
			using namespace Int64Detail;
			// create metatable with all the metamethods
			lua_newtable(L);
			gInt64MT = lua_ref(L, -1);

			// __index
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					void* p = lua_touserdatatagged(L, 1, UD_TAG(Int64));
					if (!p)
						luaL_typeerror(L, 1, "int64");

					const char* name = luaL_checkstring(L, 2);

					if (strcmp(name, "value") == 0)
					{
						lua_pushnumber(L, double(*static_cast<int64_t*>(p)));
						return 1;
					}

					luaL_error(L, "unknown field %s", name);
				},
				nullptr);
			lua_setfield(L, -2, "__index");

			// __newindex
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					void* p = lua_touserdatatagged(L, 1, UD_TAG(Int64));
					if (!p)
						luaL_typeerror(L, 1, "int64");

					const char* name = luaL_checkstring(L, 2);

					if (strcmp(name, "value") == 0)
					{
						double value = luaL_checknumber(L, 3);
						*static_cast<int64_t*>(p) = int64_t(value);
						return 0;
					}

					luaL_error(L, "unknown field %s", name);
				},
				nullptr);
			lua_setfield(L, -2, "__newindex");

			// __eq
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					lua_pushboolean(L, getInt64(L, 1) == getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__eq");

			// __lt
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					lua_pushboolean(L, getInt64(L, 1) < getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__lt");

			// __le
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					lua_pushboolean(L, getInt64(L, 1) <= getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__le");

			// __add
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushInt64(L, getInt64(L, 1) + getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__add");

			// __sub
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushInt64(L, getInt64(L, 1) - getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__sub");

			// __mul
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushInt64(L, getInt64(L, 1) * getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__mul");

			// __div
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					// ideally we'd guard against 0 but it's a test so eh
					pushInt64(L, getInt64(L, 1) / getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__div");

			// __mod
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					// ideally we'd guard against 0 and INT64_MIN but it's a test so eh
					pushInt64(L, getInt64(L, 1) % getInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__mod");

			// __pow
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushInt64(L, int64_t(pow(double(getInt64(L, 1)), double(getInt64(L, 2)))));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__pow");

			// __unm
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushInt64(L, -getInt64(L, 1));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__unm");

			// __tostring
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					int64_t value = getInt64(L, 1);
					std::string str = std::to_string(value);
					lua_pushlstring(L, str.c_str(), str.length());
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__tostring");

			// ctor
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					double v = luaL_checknumber(L, 1);
					pushInt64(L, int64_t(v));
					return 1;
				},
				"int64");
			lua_setglobal(L, "int64");
			return 0;
		}

		namespace UInt64Detail
		{
			int gUInt64MT = -1; 

			static uint64_t getUInt64(lua_State* L, int idx)
			{
				if (void* p = lua_touserdatatagged(L, idx, static_cast<int32_t>(UserDataType::UInt64)))
					return *static_cast<uint64_t*>(p);

				if (lua_isnumber(L, idx))
					return lua_tointeger(L, idx);

				luaL_typeerror(L, idx, "uint64");
			}

			static void pushUInt64(lua_State* L, uint64_t value)
			{
				void* p = lua_newuserdatatagged(L, sizeof(uint64_t), static_cast<int32_t>(UserDataType::UInt64));

				lua_getref(L, gUInt64MT);
				lua_setmetatable(L, -2);

				*static_cast<uint64_t*>(p) = value;
			}
		}
		
		int32_t RegisterUInt64(lua_State* L)
		{
			using namespace UInt64Detail;
			
			// create metatable with all the metamethods
			lua_newtable(L);
			gUInt64MT = lua_ref(L, -1);

			// __index
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					void* p = lua_touserdatatagged(L, 1, UD_TAG(UInt64));
					if (!p)
						luaL_typeerror(L, 1, "uint64");

					const char* name = luaL_checkstring(L, 2);

					if (strcmp(name, "value") == 0)
					{
						lua_pushnumber(L, double(*static_cast<uint64_t*>(p)));
						return 1;
					}

					luaL_error(L, "unknown field %s", name);
				},
				nullptr);
			lua_setfield(L, -2, "__index");

			// __newindex
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					void* p = lua_touserdatatagged(L, 1, UD_TAG(UInt64));
					if (!p)
						luaL_typeerror(L, 1, "uint64");

					const char* name = luaL_checkstring(L, 2);

					if (strcmp(name, "value") == 0)
					{
						double value = luaL_checknumber(L, 3);
						*static_cast<uint64_t*>(p) = uint64_t(value);
						return 0;
					}

					luaL_error(L, "unknown field %s", name);
				},
				nullptr);
			lua_setfield(L, -2, "__newindex");

			// __eq
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					lua_pushboolean(L, getUInt64(L, 1) == getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__eq");

			// __lt
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					lua_pushboolean(L, getUInt64(L, 1) < getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__lt");

			// __le
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					lua_pushboolean(L, getUInt64(L, 1) <= getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__le");

			// __add
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushUInt64(L, getUInt64(L, 1) + getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__add");

			// __sub
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushUInt64(L, getUInt64(L, 1) - getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__sub");

			// __mul
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushUInt64(L, getUInt64(L, 1) * getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__mul");

			// __div
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					// ideally we'd guard against 0 but it's a test so eh
					pushUInt64(L, getUInt64(L, 1) / getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__div");

			// __mod
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					// ideally we'd guard against 0 and INT64_MIN but it's a test so eh
					pushUInt64(L, getUInt64(L, 1) % getUInt64(L, 2));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__mod");

			// __pow
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushUInt64(L, uint64_t(pow(double(getUInt64(L, 1)), double(getUInt64(L, 2)))));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__pow");

			// __unm
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					pushUInt64(L, getUInt64(L, 1));
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__unm");

			// __tostring
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					uint64_t value = getUInt64(L, 1);
					std::string str = std::to_string(value);
					lua_pushlstring(L, str.c_str(), str.length());
					return 1;
				},
				nullptr);
			lua_setfield(L, -2, "__tostring");

			// ctor
			lua_pushcfunction(
				L,
				[](lua_State* L) {
					double v = luaL_checknumber(L, 1);
					pushUInt64(L, uint64_t(v));
					return 1;
				},
				"uint64");
			lua_setglobal(L, "uint64");
			return 0;
		}

		int32_t FindTable(lua_State* L, const std::string& FullName, bool bCreateIfNil)
		{
			std::vector<std::string> Names = StringEx::Split(FullName, ".");
			lua_getglobal(L, Names[0].c_str()); // globalvalue

			if(lua_isnil(L, -1)) // nil
			{
				lua_pop(L, 1);
				
				lua_getref(L, TOLUAU_LOADED_REF); // ref
				lua_getfield(L, -1, Names[0].c_str()); // ref field
				if(lua_isnil(L, -1)) // ref nil
				{
					lua_pop(L, 2);
					if(bCreateIfNil)
					{
						lua_newtable(L); // newvalue
						lua_pushvalue(L, -1); // newvalue newvalue
						lua_setglobal(L, Names[0].c_str()); // newvalue
					}
					else
					{
						lua_pushnil(L);
						LUAU_ERROR_F("cannot find table %s", Names[0].c_str())
						return 1;
					}
				}
				else
				{
					lua_remove(L, -2); // field
				}
			}

			for(size_t i = 1; i < Names.size(); ++i)
			{
				if(!lua_istable(L, -1)) // field
				{
					lua_pop(L, 1); //
					LUAU_ERROR_F("%s is not a table", Names[i].c_str())
					lua_pushnil(L);
					return 1;
				}

				lua_getfield(L, -1, Names[i].c_str()); // field1 field2
				if(lua_isnil(L, -1))
				{
					// field1 nil
					lua_pop(L, 1); // field1
					if(bCreateIfNil)
					{
						lua_newtable(L); // field1 field2
						lua_pushvalue(L, -1); // field1 field2 field2
						lua_setfield(L, -3, Names[i].c_str()); // field1 field2
					}
					else
					{
						lua_pop(L, 1); //
						LUAU_ERROR_F("cannot find table %s", Names[i].c_str())
						lua_pushnil(L);
						return 1;
					}
				}

				lua_remove(L, -2);// field2
			}

			return 1;
		}

#ifdef TOLUAUUNREAL_API
		namespace UE
		{
			
#pragma region push & check propertys

			template<typename T>
			int32 PushUProperty(lua_State* L,FProperty* Prop,uint8* Params)
			{
				auto P=CastField<T>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				typename T::TCppType Value = P->GetPropertyValue(Params);
				return StackAPI::Push<typename T::TCppType>(L,static_cast<typename T::TCppType>(Value));
			}

			template<typename T>
			int32 CheckUProperty(lua_State* L,FProperty* Prop,uint8* Params, int32 i)
			{
				auto P = CastField<T>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				P->SetPropertyValue(Params, StackAPI::Check<typename T::TCppType>(L, i));
				return 0;
			}
			
			int32 PushEnumProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FEnumProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto P2 = P->GetUnderlyingProperty();
				ensure(P2);
				int32 IntValue = P2->GetSignedIntPropertyValue(Params);
				return StackAPI::Push<int32>(L, IntValue); 	
			}
			
			int32 CheckEnumProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				auto P = CastField<FEnumProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Enum = StackAPI::Check<UEnum*>(L, i);
				P->SetEnum(Enum);
				return 0;
			}
			
			int32 PushStructProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FStructProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				UScriptStruct* ScriptStruct = P->Struct;
				void* NewStruct = lua_newuserdatatagged(L, sizeof(ScriptStruct->GetStructureSize()), UD_TAG(UEStruct));
				ScriptStruct->InitializeStruct(NewStruct);
				ScriptStruct->CopyScriptStruct(NewStruct, Params);

				auto ClassName = StringEx::FStringToStdString(ScriptStruct->GetName());
				// set metatable
				if(!HasMetaTable(L, ClassName))
				{
					RegStruct(L, ScriptStruct);
				}
				if(HasMetaTable(L, ClassName))
				{
					SetClassMetaTable(L, ClassName);
				}
				
				return 1;
			}

			int32 CheckStructProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				auto P = CastField<FStructProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				UScriptStruct* ScriptStruct = P->Struct;
				if (void* Ptr = lua_touserdatatagged(L, i, UD_TAG(UEStruct)))
				{
					ScriptStruct->CopyScriptStruct(Params, Ptr);
				}
				else
				{
					luaL_typeerror(L, i, "UEStruct");
				}
				return 0;
			}
			
			int32 PushClassProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FClassProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Value = P->GetPropertyValue(Params);
				StackAPI::Push(L, Value);
				return 1;
			}

			int32 CheckClassProperty(lua_State* L, FProperty* Prop, uint8* Params, int32_t i)
			{
				auto P = CastField<FClassProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Obj = StackAPI::Check<UObject*>(L, i);
				P->SetPropertyValue(Params, Obj);
				return 0;
			}
			
			int32 PushObjectProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FObjectProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				UObject* Obj = P->GetPropertyValue(Params);
				StackAPI::Push(L, Obj);
				return 1;
			}

			int32 CheckObjectProperty(lua_State* L, FProperty* Prop, uint8* Params, int32_t i)
			{
				auto P = CastField<FObjectProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Obj = StackAPI::Check<UObject*>(L, i);
				if(!IsValid(Obj))
				{
					luaL_errorL(L, "uobject at %d is invalid!!", i);
				}
				if(!Obj->IsA(P->PropertyClass))
				{
					luaL_errorL(L, "uobject at %d is %s instead of %s!!", i,
						StringEx::FStringToStdString(Obj->GetClass()->GetName()).c_str(),
						StringEx::FStringToStdString(P->PropertyClass->GetName()).c_str());
				}
				P->SetPropertyValue(Params, Obj);
				return 0;
			}

			int32 PushWeakObjectProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FWeakObjectProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Value = P->GetPropertyValue(Params);
				StackAPI::Push(L, Value);
				return 1;
			}

			int32 CheckWeakObjectProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				auto P = CastField<FWeakObjectProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Result = StackAPI::Check<FWeakObjectPtr>(L, i);
				P->SetPropertyValue(Params, Result);
				return 0;
			}

			int32 PushSoftObjectProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FSoftObjectProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				FSoftObjectPtr Value = P->GetPropertyValue(Params);
				StackAPI::Push(L, Value);
				return 1;
			}

			int32 CheckSoftObjectProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				auto P = CastField<FSoftObjectProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Result = StackAPI::Check<FSoftObjectPtr>(L, i);
				P->SetPropertyValue(Params, Result);
				return 0;
			}
			
			int32 PushArrayProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FArrayProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				FScriptArray* Value = P->GetPropertyValuePtr(Params);
				return ToLuauArray::Push(L, P->Inner, Value);
			}

			int32 CheckArrayProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				auto P = CastField<FArrayProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				if (lua_istable(L, i))
				{
					int ArraySize = lua_objlen(L, i);
					if (ArraySize <= 0)
					{
						return 0;
					}
					int TableIndex = i;
					if (i < 0 && i > LUA_REGISTRYINDEX)
					{
						TableIndex = i - 1;
					}
					FScriptArrayHelper ArrayHelper(P, Params);
					ArrayHelper.AddValues(ArraySize);

					int index = 0;

					lua_pushnil(L);
					while (index < ArraySize && lua_next(L, TableIndex) != 0) {
						StackAPI::UE::CheckProperty(L, P->Inner, ArrayHelper.GetRawPtr(index++), -1);
						lua_pop(L, 1);
					}
					return 0;
				}

				auto UD = Check<ToLuauArray*>(L, i);
				ToLuauArray::Clone((FScriptArray*)Params, P->Inner, UD->Get());
				return 0;
			}
			
			int32 PushMapProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FMapProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				FScriptMap* v = P->GetPropertyValuePtr(Params);
				return ToLuauMap::Push(L, P->KeyProp, P->ValueProp, v);
			}

			int32 CheckMapProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				auto P = CastField<FMapProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);

				if(lua_istable(L, i))
				{
					int32 TableIndex = i;
					if(i < 0 && i > LUA_REGISTRYINDEX)
					{
						TableIndex = i -1;
					}

					FScriptMapHelper MapHelper(P, Params);

					lua_pushnil(L);
					while (lua_next(L, TableIndex) != 0)
					{
						FDefaultConstructedPropertyElement TempKey(P->KeyProp);
						FDefaultConstructedPropertyElement TempValue(P->ValueProp);
						auto KeyPtr = TempKey.GetObjAddress();
						auto ValuePtr = TempValue.GetObjAddress();
						StackAPI::UE::CheckProperty(L, P->KeyProp, (uint8*)KeyPtr, -2);
						StackAPI::UE::CheckProperty(L, P->ValueProp, (uint8*)ValuePtr, -1);
						MapHelper.AddPair(KeyPtr, ValuePtr);
						lua_pop(L, 1);
					}
					return 0;
				}

				auto UD = StackAPI::Check<ToLuauMap*>(L, i);
				ToLuauMap::Clone((FScriptMap*)Params, P->KeyProp, P->ValueProp, UD->Get());
				
				return 0;
			}
			
			int32 PushDelegateProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FDelegateProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				FScriptDelegate* Delegate = P->GetPropertyValuePtr(Params);
				return ToLuauDelegate::Push(L, Delegate, P->SignatureFunction, Prop->GetNameCPP());
			}

			int32 CheckDelegateProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				auto P = CastField<FDelegateProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				auto Obj = StackAPI::Check<UObject*>(L, i);
				if(auto LuaDelegate = Cast<UToLuauDelegate>(Obj))
				{
					LuaDelegate->BindFunction(P->SignatureFunction);
				}
				else
				{
					luaL_typeerrorL(L, i, "UToLuauDelegate");
				}

				FScriptDelegate Delegate;
				Delegate.BindUFunction(Obj, TEXT("EventTrigger"));

				P->SetPropertyValue(Params, Delegate);
				return 0;
			}
			
			int32 PushMulticastDelegateProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FMulticastDelegateProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				FMulticastScriptDelegate* Delegate = const_cast<FMulticastScriptDelegate*>(P->GetMulticastDelegate(Params));
				return ToLuauMultiDelegate::Push(L, Delegate, P->SignatureFunction, P->GetNameCPP());
			}

			int32 CheckMulticastDelegateProperty(lua_State* L, FProperty* Prop, uint8* Params, int32 i)
			{
				return 0;
			}
			
			int32 PushMulticastInlineDelegateProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FMulticastInlineDelegateProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				FMulticastScriptDelegate* Delegate = const_cast<FMulticastScriptDelegate*>(P->GetMulticastDelegate(Params));
				return ToLuauMultiDelegate::Push(L, Delegate, P->SignatureFunction, P->GetNameCPP());
			}

			int32 PushMulticastSparseDelegateProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto P = CastField<FMulticastSparseDelegateProperty>(Prop);
				TOLUAU_ASSERT(P != nullptr);
				FMulticastScriptDelegate* Delegate = const_cast<FMulticastScriptDelegate*>(P->GetMulticastDelegate(Params));
				return ToLuauMultiDelegate::Push(L, Delegate, P->SignatureFunction, P->GetNameCPP());
			}


#pragma endregion

#pragma region Pusher & Checker Functions
			using PushPropertyFunction = int32 (*)(lua_State* L, FProperty* prop, uint8* parms);
			using CheckPropertyFunction = int32 (*)(lua_State* L, FProperty* prop, uint8* parms, int32 i);
		
			TMap<FFieldClass*, PushPropertyFunction> pusherMap;
			TMap<FFieldClass*, CheckPropertyFunction> checkerMap;

			CheckPropertyFunction GetChecker(FFieldClass* cls)
			{
				auto it = checkerMap.Find(cls);
				if(it!=nullptr)
					return *it;
				return nullptr;
			}
		
			PushPropertyFunction GetPusher(FFieldClass* cls)
			{
				auto it = pusherMap.Find(cls);
				if(it!=nullptr)
					return *it;
				return nullptr;
			}

			PushPropertyFunction GetPusher(FProperty* Prop)
			{
				return GetPusher(Prop->GetClass());
			}
		
			CheckPropertyFunction GetChecker(FProperty* Prop)
			{  
				return GetChecker(Prop->GetClass());
			}

			void RegPusher(FFieldClass* Cls, const PushPropertyFunction Func) {
				pusherMap.Add(Cls, Func);
			}

			void RegChecker(FFieldClass* cls, const CheckPropertyFunction Func) {
				checkerMap.Add(cls, Func);
			}

			void RegPusherAndChecker(FFieldClass* Cls, const PushPropertyFunction PushFunc, const CheckPropertyFunction CheckFunc)
			{
				RegPusher(Cls, PushFunc);
				RegChecker(Cls, CheckFunc);
			}
		
			template<typename T>
			inline void RegPropertyPusher() {
				pusherMap.Add(T::StaticClass(), PushUProperty<T>);
			}

			template<typename T>
			inline void RegPropertyChecker() {
				checkerMap.Add(T::StaticClass(), CheckUProperty<T>);
			}

			template<typename T>
			inline void RegPropertyPusherAndChecker()
			{
				RegPropertyPusher<T>();
				RegPropertyChecker<T>();
			}

#pragma endregion
			
			void InitProperty()
			{
#pragma region Pusher & Checker Reg
				RegPropertyPusherAndChecker<FBoolProperty>();
				RegPropertyPusherAndChecker<FInt8Property>();
				RegPropertyPusherAndChecker<FInt16Property>();
				RegPropertyPusherAndChecker<FUInt16Property>();
				RegPropertyPusherAndChecker<FIntProperty>();
				RegPropertyPusherAndChecker<FUInt32Property>();
				RegPropertyPusherAndChecker<FInt64Property>();
				RegPropertyPusherAndChecker<FUInt64Property>();
				RegPropertyPusherAndChecker<FByteProperty>();
				RegPropertyPusherAndChecker<FFloatProperty>();
				RegPropertyPusherAndChecker<FDoubleProperty>();
				RegPropertyPusherAndChecker<FTextProperty>();
				RegPropertyPusherAndChecker<FStrProperty>();
				RegPropertyPusherAndChecker<FNameProperty>();

				RegPusherAndChecker(FEnumProperty::StaticClass(), &PushEnumProperty, &CheckEnumProperty);
				RegPusherAndChecker(FDelegateProperty::StaticClass(), &PushDelegateProperty, &CheckDelegateProperty);
				RegPusher(FMulticastDelegateProperty::StaticClass(), &PushMulticastDelegateProperty);
				RegPusher(FMulticastInlineDelegateProperty::StaticClass(), &PushMulticastInlineDelegateProperty);
				RegPusher(FMulticastSparseDelegateProperty::StaticClass(), &PushMulticastSparseDelegateProperty);
				RegPusherAndChecker(FArrayProperty::StaticClass(), &PushArrayProperty, &CheckArrayProperty);
				RegPusherAndChecker(FMapProperty::StaticClass(), &PushMapProperty, &CheckMapProperty);

				RegPusherAndChecker(FStructProperty::StaticClass(), &PushStructProperty, &CheckStructProperty);
				RegPusherAndChecker(FClassProperty::StaticClass(), &PushClassProperty, &CheckClassProperty);
				RegPusherAndChecker(FObjectProperty::StaticClass(), &PushObjectProperty, &CheckObjectProperty);
				RegPusherAndChecker(FWeakObjectProperty::StaticClass(), &PushWeakObjectProperty, &CheckWeakObjectProperty);
				// RegPusherAndChecker(FLazyObjectProperty::StaticClass(), &PushLazyObjectProperty, &CheckLazyObjectProperty);
				RegPusherAndChecker(FSoftObjectProperty::StaticClass(), &PushSoftObjectProperty, &CheckSoftObjectProperty);

#pragma endregion 

			}

			int32_t PushProperty(lua_State* L, FProperty* Prop, uint8* Params)
			{
				auto Pusher = GetPusher(Prop);
				if (Pusher)
				{
					return Pusher(L,Prop,Params);
				}
				else
				{
					FString name = Prop->GetClass()->GetName();
					LUAU_LOG_F("unsupport type %s to push", TCHAR_TO_UTF8(*name));
					return 0;
				}
			}

			int32_t PushProperty(lua_State* L, FProperty* Prop, UObject* Obj)
			{
				return PushProperty(L, Prop, Prop->ContainerPtrToValuePtr<uint8>(Obj));
			}

			int32_t CheckProperty(lua_State* L, FProperty* Prop, uint8* Params, int32_t Pos)
			{
				auto Checker = GetChecker(Prop);
				if(Checker)
				{
					return Checker(L, Prop, Params, Pos);
				}
				else
				{
					FString name = Prop->GetClass()->GetName();
					LUAU_LOG_F("unsupport type %s to check", TCHAR_TO_UTF8(*name));
					return 0;
				}
			}

			int32_t CheckProperty(lua_State* L, FProperty* Prop, UObject* Obj, int32_t Pos)
			{
				return CheckProperty(L, Prop, Prop->ContainerPtrToValuePtr<uint8>(Obj), Pos);
			}

#pragma region UFunction

			namespace UEFunc
			{
				static const FName NAME_LatentInfo = TEXT("LatentInfo");

				int32 FillParamFromState(lua_State* L, FProperty* Prop, uint8* Params,int32 Pos)
				{
					uint64 PropFlag = Prop->GetPropertyFlags();
					if(PropFlag & CPF_OutParm && lua_isnil(L, Pos))
					{
						return Prop->GetSize();
					}

					auto Checker = GetChecker(Prop);
					if(Checker)
					{
						Checker(L, Prop, Params, Pos);
						return Prop->GetSize();
					}
					else
					{
						FString TypeName = Prop->GetClass()->GetName();
						luaL_error(L, "unsupport param type %s at %d", TCHAR_TO_UTF8(*TypeName), Pos);
						return 0;
					}
				}

				static auto IsRealOutParam = [](const uint64 PropFlag)
				{
					return (PropFlag&CPF_OutParm) && !(PropFlag&CPF_ConstParm) && !(PropFlag&CPF_BlueprintReadOnly);
				};

				void FillParam(lua_State* L,int Pos ,UFunction* Func,uint8* Params)
				{
					auto FuncFlag = Func->FunctionFlags;
					for(TFieldIterator<FProperty> It(Func); It && (It->PropertyFlags & CPF_Parm); ++It)
					{
						FProperty* Prop = *It;
						uint64 PropFlag = Prop->GetPropertyFlags();

						if(FuncFlag & EFunctionFlags::FUNC_Native)
						{
							if(PropFlag & CPF_ReturnParm)
							{
								continue;
							}
						}
						else if(IsRealOutParam(PropFlag))
						{
							continue;
						}

						check(Prop->GetFName() != NAME_LatentInfo);

						FillParamFromState(L, Prop, Params + Prop->GetOffset_ForInternal(), Pos);
						Pos++;
					}
				}
				
				void CallUFunction(lua_State* L, UObject* Obj, UFunction* Func, uint8* Params)
				{
					auto Flag = Func->FunctionFlags;
					if(Flag & FUNC_Net)
					{
						checkf(false, TEXT("not support rpc function"))
					}
					Obj->ProcessEvent(Func, Params);
				}
				
				int ReturnValue(lua_State* L, UFunction* Func, uint8* Params)
				{
					// check is function has return value
					const bool bHasReturnParam = Func->ReturnValueOffset != MAX_uint16;

					int Ret = 0;
					if(bHasReturnParam)
					{
						FProperty* P = Func->GetReturnProperty();
						Ret += PushProperty(L, P, Params + P->GetOffset_ForInternal());
					}

					for(TFieldIterator<FProperty> It(Func); It; ++It)
					{
						FProperty* P = *It;
						uint64 PropFlag = P->GetPropertyFlags();
						if(PropFlag & CPF_ReturnParm)
						{
							continue;
						}
						
						if(P->GetFName() == NAME_LatentInfo)
						{
							checkf(false, TEXT("not support latent ufunction"));	
						}
						else if(IsRealOutParam(PropFlag))
						{
							Ret += PushProperty(L, P, Params + P->GetOffset_ForInternal());
						}
					}

					return Ret;
				}
				
				int32_t UFunctionClosure(lua_State* L)
				{
					lua_pushvalue(L, lua_upvalueindex(1));
					void* FuncUD = lua_touserdata(L, -1);
					lua_pop(L, 1);
					
					lua_pushvalue(L, lua_upvalueindex(2));
					UClass* Class = static_cast<UClass*>(lua_touserdata(L, -1));
					lua_pop(L, 1);

					bool IsStatic = Class != nullptr;
					
					if(!FuncUD)
					{
						luaL_error(L, "call ufunction error");
					}

					int Offset=1;
					UObject* Obj;
					if(IsStatic)
					{
						Obj = Class->ClassDefaultObject;
					}
					else
					{
						Obj = StackAPI::Check<UObject*>(L, 1);
						Offset++;
					}
					UFunction* Func = static_cast<UFunction*>(FuncUD);

					uint8* Params = (uint8*)FMemory_Alloca(Func->ParmsSize);
					FMemory::Memzero(Params, Func->ParmsSize);
					for(TFieldIterator<FProperty> It(Func); It && It->HasAllPropertyFlags(CPF_Parm); ++It)
					{
						FProperty* LocalProp = *It;
						checkSlow(LocalProp);
						if(!LocalProp->HasAnyPropertyFlags(CPF_ZeroConstructor))
						{
							LocalProp->InitializeValue_InContainer(Params);
						}
					}

					FillParam(L, Offset, Func, Params);

					CallUFunction(L, Obj, Func, Params);

					int32_t OutParamCount = ReturnValue(L, Func, Params);

					for(TFieldIterator<FProperty> It(Func); It && It->HasAllPropertyFlags(CPF_Parm); ++It)
					{
						It->DestroyValue_InContainer(Params);
					}
				
					return OutParamCount;
				}
			}
			
			int32_t PushUFunction(lua_State* L, UFunction* Function, UClass* Class)
			{
				lua_pushlightuserdata(L, Function);
				if(Class)
				{
					lua_pushlightuserdata(L, Class);
				}
				else
				{
					lua_pushnil(L);
				}
				lua_pushcclosure(L, UEFunc::UFunctionClosure,
					StringEx::FStringToStdString(Function->GetName()).c_str(), 2);
				return 1;
			}

#pragma endregion 
		}

#endif
		
	}
}

#undef UD_TAG
//
// Created by 35207 on 2022/6/26 0026.
//

#include "StackAPI.h"

#include "lua.h"

#include "Toluau/ToLuau.h"
#include "Toluau/Util/Util.h"
#include "IToLuauRegister.h"

#define UD_TAG(Tag) static_cast<int32_t>(UserDataType::Tag)

namespace ToLuau
{
    namespace StackAPI
    {
    	
        int32_t SetClassMetaTable(lua_State *L, std::string& ClassName) // ud
        {
            auto Owner = ILuauState::GetByRawState(L);
            auto ClassRef = Owner->GetRegister().GetClassMetaRef(ClassName);
            if(ClassRef > 0)
            {
                lua_getref(L, ClassRef); // ud meta
                lua_setmetatable(L, -2); // ud
            }
            else
            {
                luaL_errorL(L, "cannot find class meta ref %s", ClassName.c_str());
            }
            return 1;
        }

       

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

        		luaL_typeerror(L, 1, "int64");
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

        		luaL_typeerror(L, 1, "uint64");
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

#ifdef TOLUAUUNREAL_API
    	namespace UE
        {
        	
#pragma region push & check propertys

    		template<typename T>
			int32 PushUProperty(lua_State* L,FProperty* Prop,uint8* Params)
	        {
        		auto p=CastField<T>(Prop);
        		ensure(p);
        		return StackAPI::Push<typename T::TCppType>(L,static_cast<typename T::TCppType>(p->GetPropertyValue(Params)));
	        }

        	int32 PushEnumProperty(lua_State* L, FProperty* Prop, uint8* Params)
    		{
    			auto p = CastField<FEnumProperty>(Prop);
    			ensure(p);
    			auto p2 = p->GetUnderlyingProperty();
    			ensure(p2);
    			int32 IntValue = p2->GetSignedIntPropertyValue(Params);
    			return StackAPI::Push<int32>(L, IntValue); 	
    		}
        	
	        template<typename T>
			int32 CheckUProperty(lua_State* L,FProperty* Prop,uint8* Params, int32 i)
	        {
    			auto p = CastField<T>(Prop);
    			ensure(p);
    			p->SetPropertyValue(Params, StackAPI::Check<typename T::TCppType>(L, i));
    			return 0;
    		}

        	// TODO check and push with map and array

#pragma endregion

	   
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
    	
        	template<typename T>
			inline void RegPropertyPusher() {
        		pusherMap.Add(T::StaticClass(), PushUProperty<T>);
        	}

        	template<typename T>
			inline void RegPropertyChecker() {
        		checkerMap.Add(T::StaticClass(), CheckUProperty<T>);
        	}

        	void InitProperty()
        	{
#pragma region Pusher
        		RegPropertyPusher<FInt8Property>();
        		RegPropertyPusher<FInt16Property>();
        		RegPropertyPusher<FUInt16Property>();        		
        		RegPropertyPusher<FIntProperty>();
        		RegPropertyPusher<FUInt32Property>();
        		RegPropertyPusher<FInt64Property>();
        		RegPropertyPusher<FUInt64Property>();

        		RegPropertyPusher<FByteProperty>();
        		RegPropertyPusher<FFloatProperty>();
        		RegPropertyPusher<FDoubleProperty>();
        		RegPropertyPusher<FTextProperty>();
        		RegPropertyPusher<FStrProperty>();
        		RegPropertyPusher<FNameProperty>();
#pragma endregion 
        		
#pragma region Checkers
        		RegPropertyChecker<FInt8Property>();
        		RegPropertyChecker<FInt16Property>();
				RegPropertyChecker<FUInt16Property>();        		
        		RegPropertyChecker<FIntProperty>();
        		RegPropertyChecker<FUInt32Property>();
        		RegPropertyChecker<FInt64Property>();
        		RegPropertyChecker<FUInt64Property>();

        		RegPropertyChecker<FByteProperty>();
        		RegPropertyChecker<FFloatProperty>();
        		RegPropertyChecker<FDoubleProperty>();
        		RegPropertyChecker<FTextProperty>();
        		RegPropertyChecker<FStrProperty>();
        		RegPropertyChecker<FNameProperty>();
#pragma endregion
        		
        		/// TODO Propertys
        		/// UDelegateProperty
        		/// UMulticastDelegateProperty
        		/// UMulticastInlineDelegateProperty
        		/// UMulticastSparseDelegateProperty
        		/// UObjectProperty
        		/// UArrayProperty
        		/// UMapProperty
        		/// UStructProperty
        		/// UEnumProperty
        		/// UClassProperty
        		/// UWeakObjectProperty

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

        		int FillParamFromState(lua_State* L, FProperty* Prop, uint8* Params,int i)
        		{
        			uint64 PropFlag = Prop->GetPropertyFlags();
        			if(PropFlag & CPF_OutParm && lua_isnil(L, i))
        			{
        				return Prop->GetSize();
        			}

        			auto Checker = GetChecker(Prop);
        			if(Checker)
        			{
        				Checker(L, Prop, Params, i);
        				return Prop->GetSize();
        			}
        			else
        			{
        				FString TypeName = Prop->GetClass()->GetName();
        				luaL_error(L, "unsupport param type %s at %d", TCHAR_TO_UTF8(*TypeName), i);
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

        				check(Prop->GetFName() == NAME_LatentInfo);

        				
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

        			if(!FuncUD)
        			{
        				luaL_error(L, "call ufunction error");
        			}

        			int Offset=1;
        			auto Obj = StackAPI::Check<UObject*>(L, 1);
        			Offset++;
        			UFunction* Func = reinterpret_cast<UFunction*>(FuncUD);

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
        	
            int32_t PushUFunction(lua_State* L, UFunction* Function)
            {
        		lua_pushlightuserdata(L, Function);
        		lua_pushcclosure(L, UEFunc::UFunctionClosure,
        			StringEx::FStringToStdString(Function->GetName()).c_str(), 1);
        		return 1;
            }

#pragma endregion 
        }

#endif
    	
    }
}

#undef UD_TAG
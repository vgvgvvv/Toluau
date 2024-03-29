//
// Created by 35207 on 2022/6/26 0026.
//

#include "IToLuauRegister.h"
#include <memory>
#include <map>

#include "lua.h"
#include "lualib.h"

#include "Toluau/ToLuau.h"
#include "IToLuauAPI.h"
#include "Toluau/Util/Util.h"
#include "ToLuauLib.h"

namespace ToLuau
{

	static const char* GlobalLuaRegisterName = "_GLOBAL_REGISTER_";

	class GlobalRegisterManager
	{
		friend class ILuauStaticRegister;
	public:
		static GlobalRegisterManager& Get()
		{
			static GlobalRegisterManager StaticInstance;
			return StaticInstance;
		}

		void DoRegister(IToLuauRegister* Owner)
		{
			if(!Owner)
			{
				return;
			}
			for (const auto &Register: Registers)
			{
				if(!Register)
				{
					continue;
				}
				Owner->BeginModule("");
				Register->LuaRegister(Owner);
				Owner->EndModule();
			}
			// Owner->GetMetaData().RegAllUEClass();
			// Owner->GetMetaData().RegAllUEEnum();
		}

	private:
		std::vector<ILuauStaticRegister*> Registers;
	};

	ILuauStaticRegister::ILuauStaticRegister()
	{
		GlobalRegisterManager::Get().Registers.push_back(this);
	}

	ILuauStaticRegister::~ILuauStaticRegister()
	{
		auto& Registers = GlobalRegisterManager::Get().Registers;
		auto removeIt = std::remove(Registers.begin(), Registers.end(), this);
	}


	class ToLuaRegister : public IToLuauRegister
	{
	public:
		explicit ToLuaRegister(ILuauState* InOwner) : IToLuauRegister(InOwner)
		{
			MetaData = ILuaMetaData::CreateMetaData();
		}

		void RegisterAll() override;

		void BeginModule(const std::string& ModuleName) override;
		void EndModule() override;

		void PushModuleName(const std::string& Name);
		static int32_t ModuleIndexEvent(lua_State* L);

		void BeginClass(const std::string& ClassName, const std::string& SuperClassName, bool Append) override;

		void RegClass(const Class* LuaClass) override;
		void BeginClass(const Class* LuaClass) override;
		void BeginClass(const Class* LuaClass, const Class* SuperLuaClass, bool Append) override;

#if LUAU_SUPPORT_HOYO_CLASS
		void RegClass(const FHoYoClass* LuaClass) override;
		void BeginClass(const FHoYoClass* LuaClass) override; 
		void BeginClass(const FHoYoClass* LuaClass, const FHoYoClass* SuperLuaClass, bool Append = true) override;
#endif
		
		void EndClass() override;

		static int32_t ClassNewEvent(lua_State* L);
		static int32_t ClassIndexEvent(lua_State* L);
		static int32_t ClassNewIndexEvent(lua_State* L);

		void BeginEnum(const std::string& EnumName) override;
		void EndEnum() override;

		static int32_t EnumIndexEvent(lua_State* L);
		static int32_t EnumNewIndexEvent(lua_State* L);

		void BeginStaticLib(const std::string &StaticLibName) override;
		void EndStaticLib() override;

		static int32_t StaticLibIndexEvent(lua_State* L);
		static int32_t StaticLibNewIndexEvent(lua_State* L);

		const LuaClassInfo* GetClassInfo(const std::string& Name) override;

		void RegFunction(const std::string& FuncName, LuaFunc Func) override;
		void RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter) override;
		void RegMetaFunction(const MetatableEvent& FuncName, LuaFunc Func) override;

		ILuaMetaData& GetMetaData() override
		{
			return *MetaData;
		}
		
#ifdef TOLUAUUNREAL_API
		void RegUClass(UClass* Class, bool Append) override;
		void BeginUEClass(UClass* Class, bool Append) override;
		void EndUEClass() override;

		static int32_t UEClassNewEvent(lua_State* L);
		static int32_t UEGetNativeClassEvent(lua_State* L); 
		static int32_t UEClassIndexEvent(lua_State* L);
		static int32_t UEClassNewIndexEvent(lua_State* L);
		
		void RegUEClassProperties(lua_State* L, FProperty* StartProperty);
		static bool CacheUEClassFunction(lua_State* L, UClass* Class);

		static int32_t UEClassGetProperty(lua_State* L);
		static int32_t UEClassSetProperty(lua_State* L);
		
		void RegUEnum(UEnum* Enum) override;
		
		void RegUStruct(UStruct* Struct, bool Append) override;
		void BeginUEStruct(UStruct* Struct, bool Append) override;
		void EndUEStruct() override;

		static int32_t UEStructNewEvent(lua_State* L);
		static int32_t UEGetNativeStructEvent(lua_State* L);
		static int32_t UEStructIndexEvent(lua_State* L);
		static int32_t UEStructNewIndexEvent(lua_State* L);

		void RegUEStructProperties(lua_State* L, FProperty* StartProperty);
		static int32_t UEStructGetProperty(lua_State* L);
		static int32_t UEStructSetProperty(lua_State* L);
		
#endif

		int32_t GetEnumRef(const std::string& EnumName) const override;
		void SetEnumRef(const std::string& EnumName, int32_t Ref) override;
		
		int32_t GetStaticLibRef(const std::string& StaticLibName) const override;
		void SetStaticLibRef(const std::string& StaticLibName, int32_t Ref) override;
		
		int32_t GetClassMetaRef(const std::string& ClassName) const override;
		void SetClassMetaRef(const std::string& ClassName, int32_t Ref) override;

#ifdef TOLUAUUNREAL_API
		int32_t GetClassMetaRefByUStruct(const UStruct* Struct) const override;
		int32_t GetClassMetaRefByUClass(const UClass* Class) const override;
#endif

		int32_t GetClassMetaRefByLuaClass(const ToLuau::Class* Class) const override;

#if LUAU_SUPPORT_HOYO_CLASS
		int32_t GetClassMetaRefByHoYoClass(const FHoYoClass* Class) const override;
#endif
		
	private:

		template<int32_t Value>
		void RegEnumValue(const std::string& VarName)
		{
			RegVar(VarName, nullptr, [](lua_State* L)
			{
				return ToLuau::StackAPI::Push(L, static_cast<int32_t>(Value)); 
			});
		}

		static void PushGetTable(lua_State* L);
		static void PushSetTable(lua_State* L);
		
		static void PushFullName(const ToLuaRegister* Owner, int32_t Pos);

		static void AddToLoaded(const ToLuaRegister* Owner);

		static bool GetFromPreload(lua_State* L);

		std::map<std::string, int32_t> EnumRefDict;
		std::map<std::string, int32_t> StaticLibRefDict;
		std::map<std::string, int32_t> ClassMetaRefDict;

		std::map<uint64_t, std::shared_ptr<LuaClassInfo>> ClassInfoDict;

		std::string CurrentModuleName;
		
		std::shared_ptr<ILuaMetaData> MetaData;
	};

	std::shared_ptr<IToLuauRegister> IToLuauRegister::Create(ILuauState *InOwner)
	{
		auto Result = std::make_shared<ToLuaRegister>(InOwner);
		// register all static registered types
		Result->RegisterAll();
		return Result;
	}


	void ToLuaRegister::RegisterAll()
	{
		StackAPI::StackChecker AutoStack(Owner->GetState());
		GlobalRegisterManager::Get().DoRegister(this);
	}

	void ToLuaRegister::BeginModule(const std::string& ModuleName)
	{
		auto L = Owner->GetState();
		if(!ModuleName.empty())
		{
			if(lua_type(L, -1) != LUA_TTABLE)
			{
				LUAU_ERROR_F("begin module %s", ModuleName.c_str())
				return;
			}

			lua_pushstring(L, ModuleName.c_str()); // stack name
			lua_rawget(L, -2); // stack value

			if(lua_isnil(L, -1))
			{
				lua_pop(L, 1); // stack
				lua_newtable(L); // stack table

				lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index)); // stack table __index
				lua_pushcfunction(L, ModuleIndexEvent, "ModuleIndexEvent"); // stack table __index function
				lua_rawset(L, -3); // stack table

				lua_pushstring(L, ModuleName.c_str()); // stack table name
				lua_pushstring(L, "__type"); // stack table name ".name"
				PushModuleName(ModuleName); //stack table name ".name" fullname

				lua_rawset(L, -4); // stack table name
				lua_pushvalue(L, -2); // stack table name table
				lua_rawset(L, -4); // stack table

				lua_pushvalue(L, -1); // stack table table
				lua_setmetatable(L, -2); // stack table
				return;
			}

		}
		else
		{
			luaL_findtable(L, LUA_REGISTRYINDEX, GlobalLuaRegisterName, 1);
			lua_pushstring(L, GlobalLuaRegisterName);
			lua_setfield(L, -2, "__type");
		}
	}

	void ToLuaRegister::EndModule()
	{
		auto L = Owner->GetState();
		lua_pop(L, 1);
		auto index = find(CurrentModuleName.begin(), CurrentModuleName.end(), '.');
		if(index != CurrentModuleName.end())
		{
			CurrentModuleName = CurrentModuleName.substr(0, index - CurrentModuleName.begin());
		}
		else
		{
			CurrentModuleName = "";
		}
	}

	void ToLuaRegister::PushModuleName(const std::string &Name)
	{
		std::string NewModuleName = CurrentModuleName.empty() ? Name : CurrentModuleName + "::" + Name;
		lua_pushstring(Owner->GetState(), NewModuleName.c_str());
	}

	int ToLuaRegister::ModuleIndexEvent(lua_State *L) // table key
	{
		StackAPI::StackChecker Checker(L, 2, 1);
		lua_pushvalue(L, 2); // table key key
		lua_rawget(L, 1); // table key value
		if(lua_isnil(L, -1))
		{
			return 1;
		}

		lua_pop(L, 1); // table key
		lua_pushstring(L, "__type"); // table key ".name"
		lua_rawget(L, 1); // table key namespace

		if(lua_isnil(L, -1))
		{
			lua_getref(L, TOLUAU_PRELOAD_REF); // table key space preload
			lua_pushvalue(L, -2); // table key space preload space
			lua_pushstring(L, "::"); // table key space preload space "::"
			lua_pushvalue(L, 2);  // table key space preload space "::" key
			lua_concat(L, 3); // table key space preload fullname
			lua_pushvalue(L, -1); // table key space preload fullname fullname
			lua_rawget(L, -3); // table key space preload fullname value

			if(!lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_getref(L, TOLUAU_REQUIRE_REF);// table key space preload fullname require
				lua_pushvalue(L, -2); // table key space preload fullname reuqire fullname
				lua_call(L, 1, 1);
			}
			else
			{
				lua_pushnil(L);
			}
		}

		return 1;
	}

	// table -> table name classtable mt
	void ToLuaRegister::BeginClass(const std::string& ClassName, const std::string& SuperClassName, bool Append)
	{
		auto L = Owner->GetState();
		StackAPI::StackChecker Checker(L, +3);

		lua_pushstring(L, ClassName.c_str()); // table name
		lua_newtable(L); // table name classtable

		auto FullName = CurrentModuleName.empty() ? ClassName.c_str() : CurrentModuleName + "::" + ClassName.c_str();


		auto ClassMetaRef_It = ClassMetaRefDict.find(FullName);
		int32_t ClassMetaRef = 0;
		if(ClassMetaRef_It == ClassMetaRefDict.end())
		{
			lua_newtable(L); // t k classtable mt
			ClassMetaRef = lua_ref(L, -1); // t k classtable mt
			ClassMetaRefDict.insert(std::make_pair(FullName, ClassMetaRef));
		}
		else
		{
			ClassMetaRef = ClassMetaRef_It->second;
			lua_getref(L, ClassMetaRef); // t k classtable mt
			lua_rawgetfield(L, -1, ".registed"); // t k classtable mt bool
			auto Registered = lua_toboolean(L, -1);
			lua_pop(L, 1); // t k classtable mt
			if(Registered && !Append) 
			{
				lua_pop(L, 1); // t k classtable
				// make fake metatable
				lua_newtable(L); // t k classtable fakemt
				lua_pushstring(L, ".skip");
				lua_pushboolean(L, true);
				lua_rawset(L, -3);
				return;
			}
			// t k classtable mt
		}

		// t k classtable mt

		if(!SuperClassName.empty())
		{
			auto BaseClassRef_It = ClassMetaRefDict.find(SuperClassName.c_str());
			if(BaseClassRef_It == ClassMetaRefDict.end())
			{
				lua_newtable(L);     // t k classtable mt bmt
				auto BaseClassRef = lua_ref(L, -1); // t k class mt bmt
				lua_pushstring(L, ".registed");
				lua_pushboolean(L, false);
				lua_rawset(L, -3);
				ClassMetaRefDict.insert(std::make_pair(SuperClassName.c_str(), BaseClassRef));
				lua_setmetatable(L, -2); // t k class table mt
			}
			else
			{
				lua_getref(L, BaseClassRef_It->second);
				lua_setmetatable(L, -2);
			}
		}

		//table name classtable mt

		lua_pushstring(L, "__type"); // table name classtable mt .name
		PushFullName(this, -4); // table name classtable mt .name fullname
		lua_rawset(L, -3); // table name classtable mt

		lua_pushstring(L, ".ref");
		lua_pushinteger(L, ClassMetaRef);
		lua_rawset(L, -3);

		lua_pushstring(L, ".registed");
		lua_pushboolean(L, true);
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Call));
		lua_pushcfunction(L, ClassNewEvent, (FullName + "_Call").c_str());
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
		lua_pushcfunction(L, ClassIndexEvent, (FullName + "_Index").c_str());
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
		lua_pushcfunction(L, ClassNewIndexEvent, (FullName + "_NewIndex").c_str());
		lua_rawset(L, -3);

	}

	void ToLuaRegister::RegClass(const Class* LuaClass)
	{
		if(!LuaClass)
		{
			return;
		}
		auto L = Owner->GetState();
		StackAPI::StackChecker StackHelper(L);
		RegClass(LuaClass->GetBaseClass());
		BeginClass(LuaClass);
		EndClass();
	}

	void ToLuaRegister::BeginClass(const Class* LuaClass)
	{
		BeginClass(LuaClass, LuaClass->GetBaseClass(), true);
	}

	void ToLuaRegister::BeginClass(const Class* LuaClass, const Class* SuperLuaClass, bool Append)
	{
		TOLUAU_ASSERT(LuaClass->GetBaseClass() == SuperLuaClass || *LuaClass->GetBaseClass() == *SuperLuaClass)
		BeginClass(LuaClass->Name(), SuperLuaClass == nullptr ? "" : SuperLuaClass->Name(), Append);
	}

#if LUAU_SUPPORT_HOYO_CLASS
	void ToLuaRegister::RegClass(const FHoYoClass* LuaClass)
	{
		if(!LuaClass)
		{
			return;
		}
		auto L = Owner->GetState();
		StackAPI::StackChecker StackHelper(L);
		RegClass(LuaClass->GetBaseClass());
		BeginClass(LuaClass);
		EndClass();
	}

	void ToLuaRegister::BeginClass(const FHoYoClass* LuaClass)
	{
		BeginClass(LuaClass, LuaClass->GetBaseClass(), true);
	}

	void ToLuaRegister::BeginClass(const FHoYoClass* LuaClass, const FHoYoClass* SuperLuaClass, bool Append)
	{
		auto Name = StringEx::FStringToStdString(LuaClass->GetName().ToString());
		auto SuperName = SuperLuaClass == nullptr ? "" : StringEx::FStringToStdString(SuperLuaClass->GetName().ToString());
		TOLUAU_ASSERT(LuaClass->GetBaseClass() == SuperLuaClass || *LuaClass->GetBaseClass() == *SuperLuaClass)
		BeginClass(Name, SuperName, Append);
	}
#endif

	// table name classtable mt -> table
	void ToLuaRegister::EndClass()
	{
		auto L = Owner->GetState();
		StackAPI::StackChecker Checker(L, -3);

		// table name classtable mt
		lua_rawgetfield(L, -1, ".skip"); // table name classtable mt bool
		bool bSkip = !!lua_toboolean(L, -1);
		lua_pop(L, 1); // table name classtable mt
		if(bSkip)
		{
			lua_pop(L, 3); // table
			return;
		}
		
		lua_setmetatable(L, -2); //table name classtable
		AddToLoaded(this);
		lua_rawset(L, -3); //table
	}

	int32_t ToLuaRegister::ClassNewEvent(lua_State *L)
	{
		if(!lua_istable(L, 1))
		{
			Lua::DumpStack(L, "type error");
			luaL_typeerror(L, 1, "table");
			return 0;
		}

		int32_t count = lua_gettop(L); // table args...
		lua_pushvalue(L, 1); // table args... table

		if(lua_getmetatable(L, -1)) // table args... table mt
		{
			lua_remove(L, -2);
			lua_pushstring(L, "new"); // find newfunc // table args... mt new
			lua_rawget(L, -2); // table args... mt newfunc

			if(lua_isfunction(L, -1))
			{
				for(int32_t i = 2; i <= count; i++)
				{
					lua_pushvalue(L, i);
				}
				// table args... mt newfunc args...
				lua_call(L, count-1, 1); // table args... mt result
			}
		}

		while(lua_gettop(L) != 2)
		{
			lua_remove(L, -2);
		}

		// table result

		return 1;
	}

	int32_t ToLuaRegister::ClassIndexEvent(lua_State *L)
	{
		// t k
		auto type = lua_type(L, 1);

		if(type == LUA_TUSERDATA)
		{

			lua_pushvalue(L, 1); // t k t
			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2); // t k mt v

				if(!lua_isnil(L, -1))
				{
					lua_remove(L, -2); // t k v
					return 1;
				}

				lua_pop(L, 1);
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func table
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k value
						return 1;
					}
				}

				lua_settop(L, 3);
			}

			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;
		}
		else if(type == LUA_TTABLE)
		{
			lua_pushvalue(L, 1); // t k t

			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2);

				if(!lua_isnil(L,-1))
				{
					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 2); // t k m func k
						lua_pushvalue(L, -2); // t k m func k func
						lua_rawset(L, 1); // t k mt func
					}

					lua_remove(L, -2); // t k func

					return 1;
				}

				lua_pop(L, 1); // t k mt
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func t
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k v
						return 1;
					}
				}

				lua_settop(L, 3);

			}

			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;

		}

		lua_pushnil(L);
		return 1;
	}

	int32_t ToLuaRegister::ClassNewIndexEvent(lua_State *L)
	{
		auto type = lua_type(L, 1);

		if(type == LUA_TUSERDATA)
		{
			lua_getmetatable(L, 1);
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2);

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2);
					lua_rawget(L, -2);

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1);
						lua_pushvalue(L, 3);
						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}
		else if(type == LUA_TTABLE)
		{
			lua_getmetatable(L, 1);
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2);

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2);
					lua_rawget(L, -2);

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1);
						lua_pushvalue(L, 3);
						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}

		lua_settop(L, 3);
		luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
		return 0;
	}

	void ToLuaRegister::BeginEnum(const std::string& EnumName)
	{
		auto L = Owner->GetState();
		auto FullName = CurrentModuleName.empty() ? EnumName : CurrentModuleName + "::" + EnumName;
		lua_pushstring(L, EnumName.c_str()); // enumname
		lua_newtable(L); // enumname table

		auto Ref = lua_ref(L, -1); // enumname table
		EnumRefDict.insert(std::make_pair(FullName, Ref));

		AddToLoaded(this); // enumname table
		lua_newtable(L); // enumname table metatable

		lua_pushstring(L, "__type"); // enumname table metatable .name
		PushFullName(this, -4); // enumname table metatable .name fullname
		lua_rawset(L, -3); // enumname table metatable

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
		lua_pushcfunction(L, EnumIndexEvent, (FullName + " Index").c_str());
		lua_rawset(L, -3); // enumname table metatable

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
		lua_pushcfunction(L, EnumNewIndexEvent, (FullName + " NewIndex").c_str());
		lua_rawset(L, -3); // enumname table metatable

	}

	void ToLuaRegister::EndEnum()
	{
		auto L = Owner->GetState();
		lua_setmetatable(L, -2);
		lua_rawset(L, -3);
	}

	int32_t ToLuaRegister::EnumIndexEvent(lua_State *L)
	{
		lua_getmetatable(L, 1); // table key meta

		if(lua_istable(L, -1))
		{
			lua_pushvalue(L, -2); // table key meta key
			lua_rawget(L, -2); // table key meta value

			if(!lua_isnil(L, -1))
			{
				lua_remove(L, -2); // table key value
				return 1;
			}

			lua_pop(L, 1);

			lua_pushstring(L, ".get"); // table key meta .get
			lua_rawget(L, -2);       // table key meta gettable

			if(lua_istable(L, -1))
			{
				lua_pushvalue(L, -3); // table key meta gettable key
				lua_rawget(L, -2); // table key meta gettable getfunc

				if(lua_isfunction(L, -1))
				{
					lua_call(L, 0, 1); // table key meta gettable value
					lua_pushvalue(L, -2); // table key meta gettable value key
					lua_pushvalue(L, -2); // table key meta gettable value key value
					lua_rawset(L, 3); // table key meta gettable value
					lua_remove(L, -2); // table key meta value
					lua_remove(L, -2); // table key value
					return 1;
				}

				lua_pop(L, 1);
			}
		}

		lua_settop(L, 2);
		lua_pushnil(L);
		
		return 1;
	}

	int32_t ToLuaRegister::EnumNewIndexEvent(lua_State *L)
	{
		luaL_errorL(L, "the left-hand side of an assignment must be a variable, a property or an indexer");
		return 0;
	}

	void ToLuaRegister::BeginStaticLib(const std::string &StaticLibName)
	{
		auto L = Owner->GetState();

		lua_pushstring(L, StaticLibName.c_str()); // name
		lua_newtable(L);                            // name table

		auto Ref = lua_ref(L, -1);
		StaticLibRefDict.insert(std::pair(StaticLibName, Ref));

		AddToLoaded(this);
		lua_pushvalue(L, -1); // name table table

		lua_pushstring(L, "__type"); // name table table ".name"
		PushFullName(this, -4); // name table table ".name" fullname
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
		lua_pushcfunction(L, StaticLibIndexEvent, (StaticLibName + "_Index").c_str());
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
		lua_pushcfunction(L, StaticLibNewIndexEvent, (StaticLibName + "_NewIndex").c_str());
		lua_rawset(L, -3);

	}

	void ToLuaRegister::EndStaticLib()
	{
		auto L = Owner->GetState();
		lua_setmetatable(L, -2);
		lua_rawset(L, -3);
	}

	int32_t ToLuaRegister::StaticLibIndexEvent(lua_State *L)
	{
		lua_pushvalue(L, 2); // t k k
		lua_rawget(L, 1); // t k v

		if(!lua_isnil(L, -1))
		{
			return 1;
		}

		lua_pop(L, 1);
		lua_pushstring(L, ".get");
		lua_rawget(L, 1); // t k gettable

		if(lua_istable(L, -1))
		{
			lua_pushvalue(L, 2); // t k tget k
			lua_rawget(L, -2); // t k tget func

			if(lua_isfunction(L, -1))
			{
				lua_call(L, 0, 1); // t k tget value
				lua_remove(L, -2); // t k value
				return 1;
			}
		}

		lua_settop(L, 2);

		if(ToLuaRegister::GetFromPreload(L))
		{
			return 1;
		}

		luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
		return 0;
	}

	int32_t ToLuaRegister::StaticLibNewIndexEvent(lua_State *L)
	{
		lua_pushstring(L, ".set"); // t k v .set
		lua_rawget(L, 1); // t k v table

		if(lua_istable(L, -1))
		{
			lua_pushvalue(L, 2); // t k v table k
			lua_rawget(L, -2); // t k v table func

			if(lua_isfunction(L, -1))
			{
				lua_pushvalue(L, 1); // t k v table func t
				lua_pushvalue(L, 3); // t k v table func t v
				lua_call(L, 2, 0);
				return 0;
			}
		}

		lua_settop(L, 3);

		luaL_errorL(L, "field or property %s does not exist", lua_tostring(L, 2));
		return 0;
	}

	const LuaClassInfo* ToLuaRegister::GetClassInfo(const std::string& Name)
	{
		auto Hash = ClassDetail::Hash(Name.c_str());
		auto ClassInfoDict_It = ClassInfoDict.find(Hash);
		if(ClassInfoDict_It == ClassInfoDict.end())
		{
			return nullptr;
		}
		if(!ClassInfoDict_It->second.get())
		{
			return nullptr;
		}
		return ClassInfoDict_It->second.get();
	}

	void ToLuaRegister::RegFunction(const std::string& FuncName, LuaFunc Func)
	{
		auto L = Owner->GetState();
		lua_rawgetfield(L, -1, ".skip"); // table name classtable mt bool
		bool bSkip = !!lua_toboolean(L, -1);
		lua_pop(L, 1); // table name classtable mt
		if(bSkip)
		{
			return;
		}
		lua_pushstring(L, FuncName.c_str());
		lua_pushcfunction(L, Func, FuncName.c_str());
		lua_rawset(L, -3);
	}

	void ToLuaRegister::RegVar(const std::string& VarName, LuaFunc Setter, LuaFunc Getter)
	{
		auto L = Owner->GetState();
		lua_rawgetfield(L, -1, ".skip"); // table name classtable mt bool
		bool bSkip = !!lua_toboolean(L, -1);
		lua_pop(L, 1); // table name classtable mt
		if(bSkip)
		{
			return;
		}
		if(Getter != nullptr)
		{
			PushGetTable(L);

			lua_pushstring(L, VarName.c_str()); // table gettable "varname"
			lua_pushcfunction(L, Getter, (VarName + "Getter").c_str()); // table gettable "varname" cunction
			lua_rawset(L, -3);// table gettable
			lua_pop(L, 1); //table
		}

		if(Setter != nullptr)
		{
			PushSetTable(L);

			lua_pushstring(L, VarName.c_str());
			lua_pushcfunction(L, Setter, (VarName + "Setter").c_str());
			lua_rawset(L, -3);
			lua_pop(L, 1);
		}
	}

	void ToLuaRegister::RegMetaFunction(const MetatableEvent& FuncName, LuaFunc Func)
	{
		auto MtName = GetOwner()->GetAPI().GetMtName(FuncName);
		RegFunction(MtName, Func);
	}

#ifdef TOLUAUUNREAL_API
	void ToLuaRegister::RegUClass(UClass* Class, bool Append)
	{
		auto L = Owner->GetState();
		StackAPI::StackChecker StackHelper(L);
		
		if(Class == nullptr)
		{
			return;
		}
		auto SuperClass = Class->GetSuperClass();
		if(SuperClass)
		{
			RegUClass(SuperClass, false);
		}
		GetMetaData().PushUEClass(Class);
		BeginUEClass(Class, Append);
		EndUEClass();
		GetMetaData().Pop();
	}

	// table name classtable mt
	void ToLuaRegister::BeginUEClass(UClass* Class, bool Append)
	{
		TOLUAU_ASSERT(Class != nullptr);
		auto L = Owner->GetState();
		auto SuperClass = Class->GetSuperClass();
		FString Prefix = Class->IsNative() ? Class->GetPrefixCPP() : TEXT("");
		auto ClassName = StringEx::FStringToStdString(Prefix + Class->GetName());
		FString SuperPrefix = SuperClass && SuperClass->IsNative() ? SuperClass->GetPrefixCPP() : TEXT("");
		auto SuperClassName = SuperClass ? StringEx::FStringToStdString(SuperPrefix + SuperClass->GetName()) : "";
		BeginClass(ClassName, SuperClassName, Append);
		
		lua_rawgetfield(L, -1, ".skip"); // table name classtable mt bool
		bool bSkip = !!lua_toboolean(L, -1);
		lua_pop(L, 1); // table name classtable mt
		if(bSkip)
		{
			return;
		}

		GetMetaData().ExportClassProperties(Class);
		GetMetaData().ExportClassFunctions(Class);
		
		// table name classtable mt
		RegUEClassProperties(L, Class->PropertyLink);
		
		lua_pushstring(L, "new"); 	// table name classtable mt new
		lua_pushlightuserdata(L, Class); 
		lua_pushcclosure(L, UEClassNewEvent, (ClassName + "_New").c_str(), 1);
		lua_rawset(L, -3);

		lua_pushstring(L, "GetNativeClass"); 	// table name classtable mt new
		lua_pushlightuserdata(L, Class); 
		lua_pushcclosure(L, UEGetNativeClassEvent, (ClassName + "_GetNativeClass").c_str(), 1);
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
		lua_pushlightuserdata(L, Class); 
		lua_pushcclosure(L, UEClassIndexEvent, (ClassName + "_Index").c_str(), 1);
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
		lua_pushcfunction(L, UEClassNewIndexEvent, (ClassName + "_NewIndex").c_str());
		lua_rawset(L, -3);
	}

	void ToLuaRegister::EndUEClass()
	{
		EndClass();
	}


	int32_t ToLuaRegister::UEClassNewEvent(lua_State* L)
	{
		lua_pushvalue(L, lua_upvalueindex(1));
		auto ClassPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		UClass* Class = static_cast<UClass*>(ClassPtr);
		if(!Class)
		{
			luaL_errorL(L, "new class event error !!");
			return 0;
		}
		
		UObject* Outer = StackAPI::CheckOptional<UObject*>(L, 1, (UObject*)GetTransientPackage());
		if(!Outer)
		{
			Outer = (UObject*)GetTransientPackage();
		}
		if (Class && !Outer->IsA(Class->ClassWithin))
		{
			luaL_error(L, "Can't create object in %s", TCHAR_TO_UTF8(*Outer->GetClass()->GetName()));
		}
		FName Name = StackAPI::CheckOptional<FName>(L, 2, FName(NAME_None));
		if(Class) {
			UObject* Obj = NewObject<UObject>(Outer,Class,Name);
			if(Obj) {
				StackAPI::Push<TStrongObjectPtr<UObject>>(L,TStrongObjectPtr<UObject>(Obj));
				return 1;
			}
		}
		return 0;
	}

	int32_t ToLuaRegister::UEGetNativeClassEvent(lua_State* L)
	{
		lua_pushvalue(L, lua_upvalueindex(1));
		auto ClassPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		UClass* Class = static_cast<UClass*>(ClassPtr);
		if(!Class)
		{
			luaL_errorL(L, "get class failed error !!");
			return 0;
		}

		return StackAPI::Push<UClass>(L, Class);
	}

	int32_t ToLuaRegister::UEClassIndexEvent(lua_State* L)
	{
		// t k
		auto type = lua_type(L, 1);
		
		lua_pushvalue(L, lua_upvalueindex(1));
		auto ClassPtr = static_cast<UClass*>(lua_tolightuserdata(L, -1));
		lua_pop(L, 1);
		
		if(type == LUA_TUSERDATA)
		{
			lua_pushvalue(L, 1); // t k t
			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2); // t k mt v

				if(!lua_isnil(L, -1))
				{
					lua_remove(L, -2); // t k v
					return 1;
				}

				lua_pop(L, 1);
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func table
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k value
						return 1;
					}
				}

				lua_settop(L, 3);
			}
			
			lua_settop(L, 2);

			if(CacheUEClassFunction(L, ClassPtr)) // t k
			{
				lua_getmetatable(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2); // t k mt v
				
				return 1;
			}

			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;
		}
		else if(type == LUA_TTABLE)
		{
			lua_pushvalue(L, 1); // t k t

			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2);

				if(!lua_isnil(L,-1))
				{
					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 2); // t k m func k
						lua_pushvalue(L, -2); // t k m func k func
						lua_rawset(L, 1); // t k mt func
					}

					lua_remove(L, -2); // t k func

					return 1;
				}

				lua_pop(L, 1); // t k mt
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func t
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k v
						return 1;
					}
				}

				lua_settop(L, 3);

			}

			lua_settop(L, 2);
			
			if(CacheUEClassFunction(L, ClassPtr)) // t k
			{
				lua_getmetatable(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2); // t k mt v
				
				return 1;
			}

			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;
		}

		lua_pushnil(L);
		return 1;
	}

	int32_t ToLuaRegister::UEClassNewIndexEvent(lua_State* L)
	{
		// table key value
		auto type = lua_type(L, 1);

		auto Obj = StackAPI::Check<UObject*>(L, 1);
		if(!Obj)
		{
			luaL_error(L, "try to index nil uobject");
		}

		if(type == LUA_TUSERDATA)
		{
			lua_getmetatable(L, 1); // table key value mt
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2); // table key value mt settable

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // table key value mt settable key
					lua_rawget(L, -2); // table key value mt settable func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // table key value mt settable func table
						lua_pushvalue(L, 3); // table key value mt settable func table value

						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}
		else if(type == LUA_TTABLE)
		{
			lua_getmetatable(L, 1);
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2);

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2);
					lua_rawget(L, -2);

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1);
						lua_pushvalue(L, 3);
						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}

		lua_settop(L, 3);
		luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
		return 0;
	}
	
	void ToLuaRegister::RegUEnum(UEnum* Enum)
	{
		auto EnumName = Enum->GetName();
		bool HasNameSpace = EnumName.Contains(TEXT("::"));
		TArray<FString> Splited;
		if(HasNameSpace)
		{
			EnumName.ParseIntoArray(Splited, TEXT("::"));
			for(int i = 0; i < Splited.Num() - 1; i ++)
			{
				BeginModule(StringEx::FStringToStdString(Splited[i]));
			}
			GetMetaData().PushEnum(StringEx::FStringToStdString(Splited.Last()));
			BeginEnum(StringEx::FStringToStdString(Splited.Last()));
		}
		else
		{
			GetMetaData().PushEnum(StringEx::FStringToStdString(EnumName));
			BeginEnum(StringEx::FStringToStdString(EnumName));
		}
		

		auto L = Owner->GetState();
		// enumname table metatable
		auto Num = Enum->NumEnums();
		for(int32_t i = 0; i < Num; i ++)
		{
			auto Name = Enum->GetNameByIndex(i);
			auto Value = Enum->GetValueByIndex(i);

			FString SubEnumName = Name.ToString();
			FString Left;
			FString Right;
			if(SubEnumName.Split(TEXT("::"), &Left, &Right))
			{
				lua_pushstring(L, TCHAR_TO_ANSI(*Right)); // enumname table metatable name
				GetMetaData().RegVar(true, GetClassName<int32>(), TCHAR_TO_ANSI(*Right));
			}
			else
			{
				lua_pushstring(L, TCHAR_TO_ANSI(*SubEnumName)); // enumname table metatable name
				GetMetaData().RegVar(true, GetClassName<int32>(), TCHAR_TO_ANSI(*SubEnumName));
			}
			lua_pushinteger(L, static_cast<int32_t>(Value)); // enumname table metatable name value
			lua_rawset(L, -3); // enumname table metatable
		}
		
		EndEnum();
		GetMetaData().Pop();
		if(HasNameSpace)
		{
			for(int i = 0; i < Splited.Num() - 1; i ++)
			{
				EndModule();
			}
		}
	}

	void ToLuaRegister::RegUStruct(UStruct* Struct, bool Append)
	{
		if(!Struct)
		{
			return;
		}
		auto SuperStruct = Cast<UScriptStruct>(Struct->GetSuperStruct());
		if(SuperStruct)
		{
			RegUStruct(SuperStruct, false);
		}
		GetMetaData().PushUEStruct(Struct);
		GetMetaData().ExportClassProperties(Struct);
		GetMetaData().ExportClassFunctions(Struct);
		BeginUEStruct(Struct, Append);
		EndUEStruct();
		GetMetaData().Pop();
	}

	void ToLuaRegister::BeginUEStruct(UStruct* Struct, bool Append)
	{
		TOLUAU_ASSERT(Struct != nullptr)
		auto L = Owner->GetState();
		auto SuperStruct = Struct->GetSuperStruct();
		FString Prefix = Struct->IsNative() ? Struct->GetPrefixCPP() : TEXT("");
		auto StructName = StringEx::FStringToStdString(Prefix + Struct->GetName());
		auto SuperStructName = SuperStruct ? StringEx::FStringToStdString(Prefix + SuperStruct->GetName()) : "";
		BeginClass(StructName, SuperStructName, Append);

		lua_rawgetfield(L, -1, ".skip"); // table name classtable mt bool
		bool bSkip = !!lua_toboolean(L, -1);
		lua_pop(L, 1); // table name classtable mt
		if(bSkip)
		{
			return;
		}

		RegUEStructProperties(L, Struct->PropertyLink);
		
		// table name classtable mt

		lua_pushstring(L, "new");
		lua_pushlightuserdata(L, Struct);
		lua_pushcclosure(L, UEStructNewEvent, (StructName + "_New").c_str(), 1);
		lua_rawset(L, -3);

		lua_pushstring(L, "GetNativeClass"); 	// table name classtable mt new
		lua_pushlightuserdata(L, Struct); 
		lua_pushcclosure(L, UEGetNativeStructEvent, (StructName + "_GetNativeClass").c_str(), 1);
		lua_rawset(L, -3);
		
		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::Index));
		lua_pushlightuserdata(L, Struct);
		lua_pushcclosure(L, UEStructIndexEvent, (StructName + "_Index").c_str(), 1);
		lua_rawset(L, -3);

		lua_pushstring(L, IToLuauAPI::GetMtName(MetatableEvent::NewIndex));
		lua_pushlightuserdata(L, Struct);
		lua_pushcclosure(L, UEStructNewIndexEvent, (StructName + "_NewIndex").c_str(), 1);
		lua_rawset(L, -3);
	}

	void ToLuaRegister::EndUEStruct()
	{
		EndClass();
	}
	
	int32_t ToLuaRegister::UEStructNewEvent(lua_State* L)
	{
		lua_pushvalue(L, lua_upvalueindex(1));
		auto ScriptStructPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		UScriptStruct* Struct = static_cast<UScriptStruct*>(ScriptStructPtr);
		if(!Struct)
		{
			luaL_errorL(L, "new struct event error !!");
			return 0;
		}

		return StackAPI::UE::PushNewUEStruct(L, Struct);

	}

	int32_t ToLuaRegister::UEGetNativeStructEvent(lua_State* L)
	{
		lua_pushvalue(L, lua_upvalueindex(1));
		auto ClassPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		UScriptStruct* Struct = static_cast<UScriptStruct*>(ClassPtr);
		if(!Struct)
		{
			luaL_errorL(L, "get struct failed error !!");
			return 0;
		}

		return StackAPI::Push<UScriptStruct>(L, Struct);
	}

	int32_t ToLuaRegister::UEStructIndexEvent(lua_State* L)
	{
		auto type = lua_type(L, 1);

		lua_pushvalue(L, lua_upvalueindex(1)); // table key upvalue
		auto ScriptStructPtr = static_cast<UStruct*>(lua_tolightuserdata(L, -1)); 
		lua_pop(L, 1); // table key
		
		// table key
		if(type == LUA_TUSERDATA)
		{
			lua_pushvalue(L, 1); // t k t
			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2); // t k mt v

				if(!lua_isnil(L, -1))
				{
					lua_remove(L, -2); // t k v
					return 1;
				}

				lua_pop(L, 1);
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func table
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k value
						return 1;
					}
				}

				lua_settop(L, 3);
			}
			
			lua_settop(L, 2);

			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;
		}
		else if(type == LUA_TTABLE)
		{
			lua_pushvalue(L, 1); // t k t

			while(lua_getmetatable(L, -1)) // t k t mt
			{
				lua_remove(L, -2); // t k mt
				lua_pushvalue(L, 2); // t k mt k
				lua_rawget(L, -2);

				if(!lua_isnil(L,-1))
				{
					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 2); // t k m func k
						lua_pushvalue(L, -2); // t k m func k func
						lua_rawset(L, 1); // t k mt func
					}

					lua_remove(L, -2); // t k func

					return 1;
				}

				lua_pop(L, 1); // t k mt
				lua_pushstring(L, ".get"); // t k mt .get
				lua_rawget(L, -2); // t k mt tget

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // t k mt tget k
					lua_rawget(L, -2); // t k mt tget func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // t k mt tget func t
						lua_call(L, 1, 1); // t k mt tget func v
						lua_remove(L, -2);
						lua_remove(L, -2);
						lua_remove(L, -2); // t k v
						return 1;
					}
				}

				lua_settop(L, 3);

			}

			lua_settop(L, 2);
			
			if(GetFromPreload(L))
			{
				return 1;
			}

			luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
			return 0;
		}
		else
		{
			Lua::DumpStack(L, "type error");
			luaL_typeerrorL(L, -2, "UEStruct");
			return 0;
		}
	}

	int32_t ToLuaRegister::UEStructNewIndexEvent(lua_State* L)
	{

		// table key value
		auto type = lua_type(L, 1);

		if(type == LUA_TUSERDATA)
		{
			lua_getmetatable(L, 1); // table key value mt
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2); // table key value mt settable

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2); // table key value mt settable key
					lua_rawget(L, -2); // table key value mt settable func

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1); // table key value mt settable func table
						lua_pushvalue(L, 3); // table key value mt settable func table value

						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}
		else if(type == LUA_TTABLE)
		{
			lua_getmetatable(L, 1);
			while(lua_istable(L, -1))
			{
				lua_pushstring(L, ".set");
				lua_rawget(L, -2);

				if(lua_istable(L, -1))
				{
					lua_pushvalue(L, 2);
					lua_rawget(L, -2);

					if(lua_isfunction(L, -1))
					{
						lua_pushvalue(L, 1);
						lua_pushvalue(L, 3);
						lua_call(L, 2, 0);
						return 0;
					}

					lua_pop(L, 1);
				}

				lua_pop(L, 1);

				if(!lua_getmetatable(L, -1))
				{
					lua_pushnil(L);
				}

				lua_remove(L, -2);
			}
		}

		lua_settop(L, 3);
		luaL_error(L, "field or property %s does not exist", lua_tostring(L, 2));
		return 0;
	}

	void ToLuaRegister::RegUEStructProperties(lua_State* L, FProperty* StartProperty)
	{
		PushGetTable(L);
		
		auto PropertyLink = StartProperty;
		for (FProperty* Property = PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			auto PropertyName = StringEx::FStringToStdString(Property->GetName());
			lua_pushstring(L, PropertyName.c_str()); // gettable propertyname
			lua_pushlightuserdata(L, Property); // gettable lightuserdata
			lua_pushcclosure(L, &ToLuaRegister::UEStructGetProperty, "", 1); //gettable propertyname closuer
			lua_rawset(L, -3); // gettable
		}
		lua_pop(L, 1); // 

		PushSetTable(L);

		PropertyLink = StartProperty;
		for (FProperty* Property = PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			auto PropertyName = StringEx::FStringToStdString(Property->GetName());
			lua_pushstring(L, PropertyName.c_str()); // settable propertyname
			lua_pushlightuserdata(L, Property); // settable lightuserdata
			lua_pushcclosure(L, &ToLuaRegister::UEStructSetProperty, "", 1); //gettable propertyname closuer
			lua_rawset(L, -3); // settable
		}
		lua_pop(L, 1); // 
	}

	int32_t ToLuaRegister::UEStructGetProperty(lua_State* L)
	{
		auto upvalueIndex = lua_upvalueindex(1);
		lua_pushvalue(L, upvalueIndex); // prop
		void* PropPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		if(!PropPtr)
		{
			luaL_error(L, "Get UProperty Error !!");
		}
		
		FProperty* Prop = static_cast<FProperty*>(PropPtr);
		if (void* p = StackAPI::UE::CheckUEStruct(L, -1))
		{
			StackAPI::UE::PushProperty(L, Prop, static_cast<uint8*>(p) + Prop->GetOffset_ForInternal());
			return 1;
		}
		else
		{
			lua_pushnil(L);
		}
		
		return 1;
	}

	int32_t ToLuaRegister::UEStructSetProperty(lua_State* L)
	{
		// obj value func
		auto upvalueIndex = lua_upvalueindex(1);
		lua_pushvalue(L, upvalueIndex); // prop
		void* PropPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		if(!PropPtr)
		{
			luaL_error(L, "Get UProperty Error !!");
		}
		
		FProperty* Prop = static_cast<FProperty*>(PropPtr);
		if (void* p = StackAPI::UE::CheckUEStruct(L, -2))
		{
			StackAPI::UE::CheckProperty(L, Prop, static_cast<uint8*>(p) + Prop->GetOffset_ForInternal(), -1);
			return 1;
		}

		return 0;
	}

	void ToLuaRegister::RegUEClassProperties(lua_State* L, FProperty* StartProperty)
	{
		PushGetTable(L);
		
		auto PropertyLink = StartProperty;
		for (FProperty* Property = PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			auto PropertyName = StringEx::FStringToStdString(Property->GetName());
			lua_pushstring(L, PropertyName.c_str()); // gettable propertyname
			lua_pushlightuserdata(L, Property); // gettable lightuserdata
			lua_pushcclosure(L, &ToLuaRegister::UEClassGetProperty, "", 1); //gettable propertyname closuer
			lua_rawset(L, -3); // gettable
		}
		lua_pop(L, 1); // 

		PushSetTable(L);

		PropertyLink = StartProperty;
		for (FProperty* Property = PropertyLink; Property != nullptr; Property = Property->PropertyLinkNext)
		{
			auto PropertyName = StringEx::FStringToStdString(Property->GetName());
			lua_pushstring(L, PropertyName.c_str()); // settable propertyname
			lua_pushlightuserdata(L, Property); // settable lightuserdata
			lua_pushcclosure(L, &ToLuaRegister::UEClassSetProperty, "", 1); //gettable propertyname closuer
			lua_rawset(L, -3); // settable
		}
		lua_pop(L, 1); // 
	}

	bool ToLuaRegister::CacheUEClassFunction(lua_State* L, UClass* Class)
	{
		// t k
		auto Name = lua_tostring(L, -1);

		auto Function = Class->FindFunctionByName(FName(Name));

		if(!Function)
		{
			return false;
		}

		lua_getmetatable(L, -2); // t k mt
		lua_pushvalue(L, -2); // t k mt k
		if(Function->HasAllFunctionFlags(EFunctionFlags::FUNC_Static))
		{
			StackAPI::UE::PushUFunction(L, Function, Class); // t k mt k func
		}
		else
		{
			StackAPI::UE::PushUFunction(L, Function); // t k mt k func
		}
		lua_rawset(L, -3); // t k mt

		lua_pop(L, 1); // t k
		
		return true;
	}

	int32_t ToLuaRegister::UEClassGetProperty(lua_State* L)
	{
		auto upvalueIndex = lua_upvalueindex(1);
		lua_pushvalue(L, upvalueIndex); // prop
		void* PropPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		if(!PropPtr)
		{
			luaL_error(L, "Get UProperty Error !!");
		}
		
		FProperty* Prop = static_cast<FProperty*>(PropPtr);

		auto Obj = StackAPI::Check<UObject*>(L, -1);

		if(Obj)
		{
			StackAPI::UE::PushProperty(L, Prop, Obj);
		}
		else
		{
			lua_pushnil(L);
		}
		
		return 1;
	}

	int32_t ToLuaRegister::UEClassSetProperty(lua_State* L)
	{
		// obj value func
		auto upvalueIndex = lua_upvalueindex(1);
		lua_pushvalue(L, upvalueIndex); // prop
		void* PropPtr = lua_tolightuserdata(L, -1);
		lua_pop(L, 1);

		if(!PropPtr)
		{
			luaL_error(L, "Get UProperty Error !!");
		}
		
		FProperty* Prop = reinterpret_cast<FProperty*>(PropPtr);
		auto Obj = StackAPI::Check<UObject*>(L, -2);
		if(Obj)
		{
			StackAPI::UE::CheckProperty(L, Prop, Obj, -1);
		}
		
		return 0;
	}

#endif

	int32_t ToLuaRegister::GetEnumRef(const std::string &EnumName) const
	{
		auto It = EnumRefDict.find(EnumName);
		if(It != EnumRefDict.end())
		{
			return It->second;
		}
		return -1;
	}

	void ToLuaRegister::SetEnumRef(const std::string& EnumName, int32_t Ref)
	{
		auto It = EnumRefDict.find(EnumName);
		if(It != EnumRefDict.end())
		{
			LUAU_ERROR_F("enum ref has been registered %s", EnumName.c_str());
		}
		else
		{
			EnumRefDict.insert(std::make_pair(EnumName, Ref));
		}
	}

	int32_t ToLuaRegister::GetStaticLibRef(const std::string &StaticLibName) const
	{
		auto It = StaticLibRefDict.find(StaticLibName);
		if(It != StaticLibRefDict.end())
		{
			return It->second;
		}
		return -1;
	}

	void ToLuaRegister::SetStaticLibRef(const std::string& StaticLibName, int32_t Ref)
	{
		auto It = StaticLibRefDict.find(StaticLibName);
		if(It != StaticLibRefDict.end())
		{
			LUAU_ERROR_F("staticlib ref has been registered %s", StaticLibName.c_str());
		}
		else
		{
			StaticLibRefDict.insert(std::make_pair(StaticLibName, Ref));
		}
	}

	int32_t ToLuaRegister::GetClassMetaRef(const std::string &ClassName) const
	{
		auto It = ClassMetaRefDict.find(ClassName);
		if(It != ClassMetaRefDict.end())
		{
			return It->second;
		}
		return -1;
	}

	void ToLuaRegister::SetClassMetaRef(const std::string& ClassName, int32_t Ref)
	{
		auto It = ClassMetaRefDict.find(ClassName);
		if(It != ClassMetaRefDict.end())
		{
			LUAU_ERROR_F("class meta ref has been registered %s", ClassName.c_str());
		}
		else
		{
			ClassMetaRefDict.insert(std::make_pair(ClassName, Ref));
		}
	}

#ifdef TOLUAUUNREAL_API
	int32_t ToLuaRegister::GetClassMetaRefByUStruct(const UStruct* Struct) const
	{
		const UStruct* CurrentClass = Struct;
		do
		{
			auto ClassName = GetUStructName(CurrentClass);
			auto Ref = GetClassMetaRef(ClassName);
			if(Ref > 0)
			{
				return Ref;
			}
			CurrentClass = CurrentClass->GetSuperStruct();
		} while (CurrentClass);
		return -1;
	}

	int32_t ToLuaRegister::GetClassMetaRefByUClass(const UClass* Class) const
	{
		const UClass* CurrentClass = Class;
		do
		{
			auto ClassName = GetUClassName(CurrentClass);
			auto Ref = GetClassMetaRef(ClassName);
			if(Ref > 0)
			{
				return Ref;
			}
			CurrentClass = CurrentClass->GetSuperClass();
		} while (CurrentClass);
		return -1;
	}
#endif

	int32_t ToLuaRegister::GetClassMetaRefByLuaClass(const ToLuau::Class* Class) const
	{
		const ToLuau::Class* CurrentClass = Class;
		do
		{
			auto ClassName = CurrentClass->Name();
			auto Ref = GetClassMetaRef(ClassName);
			if(Ref > 0)
			{
				return Ref;
			}
			CurrentClass = CurrentClass->GetBaseClass();
		} while (CurrentClass);
		return -1;
	}

#if LUAU_SUPPORT_HOYO_CLASS
	int32_t ToLuaRegister::GetClassMetaRefByHoYoClass(const FHoYoClass* Class) const
	{
		const FHoYoClass* CurrentClass = Class;
		do
		{
			auto ClassName = StringEx::FStringToStdString(CurrentClass->GetName().ToString());
			auto Ref = GetClassMetaRef(ClassName);
			if(Ref > 0)
			{
				return Ref;
			}
			CurrentClass = CurrentClass->GetBaseClass();
		} while (CurrentClass);
		return -1;
	}
#endif


	void ToLuaRegister::PushGetTable(lua_State* L)
	{
		lua_pushstring(L, ".get"); // table ".get"
		lua_rawget(L, -2); // table gettable

		if(!lua_istable(L, -1))
		{
			lua_pop(L, 1);  // table
			lua_newtable(L); // table gettable
			lua_pushstring(L, ".get"); // table gettable ".get"
			lua_pushvalue(L, -2); // table gettable ".get" gettable
			lua_rawset(L, -4); // table gettable
		}
	}

	void ToLuaRegister::PushSetTable(lua_State* L)
	{
		lua_pushstring(L, ".set"); // table ".set"
		lua_rawget(L, -2);

		if(!lua_istable(L, -1))
		{
			lua_pop(L, 1);
			lua_newtable(L);
			lua_pushstring(L, ".set");
			lua_pushvalue(L, -2);
			lua_rawset(L, -4);
		}
	}

	void ToLuaRegister::PushFullName(const ToLuaRegister* Owner, int32_t Pos)
	{
		auto L = Owner->GetOwner()->GetState();
		if(!Owner->CurrentModuleName.empty())
		{
			lua_pushstring(L, Owner->CurrentModuleName.c_str());
			lua_pushstring(L, "::");
			lua_pushvalue(L, Pos < 0 ? Pos - 2 : Pos + 2);
			lua_concat(L, 3);
		}
		else
		{
			lua_pushvalue(L, Pos);
		}
	}

	void ToLuaRegister::AddToLoaded(const ToLuaRegister* Owner)
	{
		auto L = Owner->GetOwner()->GetState();
		lua_getref(L, TOLUAU_LOADED_REF); // name table preload
		PushFullName(Owner, -3); // name table preload fullname
		lua_pushvalue(L, -3); // name table preload fullname table
		lua_rawset(L, -3); // name table preload
		lua_pop(L, 1);
	}

	bool ToLuaRegister::GetFromPreload(lua_State* L)
	{
		lua_settop(L, 2); // table key
		lua_getmetatable(L, 1); // table key meta
		lua_pushstring(L, "__type"); // table key meta ".name"
		lua_rawget(L, -2); // table key meta space

		if(!lua_isnil(L, -1))
		{
			lua_getref(L, TOLUAU_PRELOAD_REF); // table key meta space preload
			lua_pushvalue(L, -2); // table key meta space preload space
			lua_pushstring(L, "::"); // table key meta space preload space "::"
			lua_pushvalue(L, 2); // table key meta space preload space "::" key
			lua_concat(L, 3); // table key meta space preload fullname
			lua_pushvalue(L, -1); // table key meta space preload fullname fullname
			lua_rawget(L, -3); // table key meta space preload fullname value

			if(!lua_isnil(L, -1))
			{
				lua_pop(L, 1);
				lua_getref(L, TOLUAU_REQUIRE_REF);       // table key meta space preload fullname require
				lua_pushvalue(L, -2);                // table key meta space preload fullname require fullname
				lua_call(L, 1, 1);
				return true;
			}
		}

		return false;
	}



}
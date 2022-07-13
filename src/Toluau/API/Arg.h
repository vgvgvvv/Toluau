#pragma once

namespace ToLuau
{
	struct IArg
	{
		virtual int32_t Count() const = 0;

		virtual void PushLua(lua_State *L) const = 0;

		virtual void PushLuaTable(lua_State *L) const = 0;
	};

	struct Arg0 : public IArg
	{
		int32_t Count() const override { return 0; }

		void PushLua(lua_State *L) const override
		{
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
		}
	};

	template<typename TArg1>
	struct Arg1 : public IArg
	{
		TArg1 A1;

		explicit Arg1(TArg1 InA1) : A1(InA1) {}

		int32_t Count() const override { return 1; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
		}
	};

	template<typename TArg1,
			typename TArg2>
	struct Arg2 : public IArg
	{
		TArg1 A1;
		TArg2 A2;

		explicit Arg2(TArg1 InA1,
		              TArg2 InA2)
				: A1(InA1)
				, A2(InA2)
		{}

		int32_t Count() const override { return 2; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
		}
	};

	template<typename TArg1,
			typename TArg2,
			typename TArg3>
	struct Arg3 : public IArg
	{
		TArg1 A1;
		TArg2 A2;
		TArg3 A3;

		explicit Arg3(TArg1 InA1,
		              TArg2 InA2,
		              TArg3 InA3)
				: A1(InA1)
				, A2(InA2)
				, A3(InA3)
		{}

		int32_t Count() const override { return 3; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
			StackAPI::Push(L, A3);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
			StackAPI::PushTableItem(L, 3, A3);
		}
	};

	template<typename TArg1,
			typename TArg2,
			typename TArg3,
			typename TArg4>
	struct Arg4 : public IArg
	{
		TArg1 A1;
		TArg2 A2;
		TArg3 A3;
		TArg4 A4;

		explicit Arg4(TArg1 InA1,
		              TArg2 InA2,
		              TArg3 InA3,
		              TArg4 InA4)
				: A1(InA1)
				, A2(InA2)
				, A3(InA3)
				, A4(InA4)
		{}

		int32_t Count() const override { return 4; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
			StackAPI::Push(L, A3);
			StackAPI::Push(L, A4);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
			StackAPI::PushTableItem(L, 3, A3);
			StackAPI::PushTableItem(L, 4, A4);
		}
	};

	template<typename TArg1,
			typename TArg2,
			typename TArg3,
			typename TArg4,
			typename TArg5>
	struct Arg5 : public IArg
	{
		TArg1 A1;
		TArg2 A2;
		TArg3 A3;
		TArg4 A4;
		TArg5 A5;

		explicit Arg5(TArg1 InA1,
		              TArg2 InA2,
		              TArg3 InA3,
		              TArg4 InA4,
		              TArg5 InA5)
				: A1(InA1)
				, A2(InA2)
				, A3(InA3)
				, A4(InA4)
				, A5(InA5)
		{}

		int32_t Count() const override { return 5; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
			StackAPI::Push(L, A3);
			StackAPI::Push(L, A4);
			StackAPI::Push(L, A5);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
			StackAPI::PushTableItem(L, 3, A3);
			StackAPI::PushTableItem(L, 4, A4);
			StackAPI::PushTableItem(L, 5, A5);
		}
	};

	template<typename TArg1,
			typename TArg2,
			typename TArg3,
			typename TArg4,
			typename TArg5,
			typename TArg6>
	struct Arg6 : public IArg
	{
		TArg1 A1;
		TArg2 A2;
		TArg3 A3;
		TArg4 A4;
		TArg5 A5;
		TArg6 A6;

		explicit Arg6(TArg1 InA1,
		              TArg2 InA2,
		              TArg3 InA3,
		              TArg4 InA4,
		              TArg5 InA5,
		              TArg6 InA6)
				: A1(InA1)
				, A2(InA2)
				, A3(InA3)
				, A4(InA4)
				, A5(InA5)
				, A6(InA6)
		{}

		int32_t Count() const override { return 6; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
			StackAPI::Push(L, A3);
			StackAPI::Push(L, A4);
			StackAPI::Push(L, A5);
			StackAPI::Push(L, A6);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
			StackAPI::PushTableItem(L, 3, A3);
			StackAPI::PushTableItem(L, 4, A4);
			StackAPI::PushTableItem(L, 5, A5);
			StackAPI::PushTableItem(L, 6, A6);
		}
	};

	template<typename TArg1,
			typename TArg2,
			typename TArg3,
			typename TArg4,
			typename TArg5,
			typename TArg6,
			typename TArg7>
	struct Arg7 : public IArg
	{
		TArg1 A1;
		TArg2 A2;
		TArg3 A3;
		TArg4 A4;
		TArg5 A5;
		TArg6 A6;
		TArg7 A7;

		explicit Arg7(TArg1 InA1,
		              TArg2 InA2,
		              TArg3 InA3,
		              TArg4 InA4,
		              TArg5 InA5,
		              TArg6 InA6,
		              TArg7 InA7)
				: A1(InA1)
				, A2(InA2)
				, A3(InA3)
				, A4(InA4)
				, A5(InA5)
				, A6(InA6)
				, A7(InA7)
		{}

		int32_t Count() const override { return 7; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
			StackAPI::Push(L, A3);
			StackAPI::Push(L, A4);
			StackAPI::Push(L, A5);
			StackAPI::Push(L, A6);
			StackAPI::Push(L, A7);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
			StackAPI::PushTableItem(L, 3, A3);
			StackAPI::PushTableItem(L, 4, A4);
			StackAPI::PushTableItem(L, 5, A5);
			StackAPI::PushTableItem(L, 6, A6);
			StackAPI::PushTableItem(L, 7, A7);
		}
	};

	template<typename TArg1,
			typename TArg2,
			typename TArg3,
			typename TArg4,
			typename TArg5,
			typename TArg6,
			typename TArg7,
			typename TArg8>
	struct Arg8 : public IArg
	{
		TArg1 A1;
		TArg2 A2;
		TArg3 A3;
		TArg4 A4;
		TArg5 A5;
		TArg6 A6;
		TArg7 A7;
		TArg8 A8;

		explicit Arg8(TArg1 InA1,
		              TArg2 InA2,
		              TArg3 InA3,
		              TArg4 InA4,
		              TArg5 InA5,
		              TArg6 InA6,
		              TArg7 InA7,
		              TArg8 InA8)
				: A1(InA1)
				, A2(InA2)
				, A3(InA3)
				, A4(InA4)
				, A5(InA5)
				, A6(InA6)
				, A7(InA7)
				, A8(InA8)
		{}

		int32_t Count() const override { return 8; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
			StackAPI::Push(L, A3);
			StackAPI::Push(L, A4);
			StackAPI::Push(L, A5);
			StackAPI::Push(L, A6);
			StackAPI::Push(L, A7);
			StackAPI::Push(L, A8);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
			StackAPI::PushTableItem(L, 3, A3);
			StackAPI::PushTableItem(L, 4, A4);
			StackAPI::PushTableItem(L, 5, A5);
			StackAPI::PushTableItem(L, 6, A6);
			StackAPI::PushTableItem(L, 7, A7);
			StackAPI::PushTableItem(L, 8, A8);
		}
	};

	template<typename TArg1,
			typename TArg2,
			typename TArg3,
			typename TArg4,
			typename TArg5,
			typename TArg6,
			typename TArg7,
			typename TArg8,
			typename TArg9>
	struct Arg9 : public IArg
	{
		TArg1 A1;
		TArg2 A2;
		TArg3 A3;
		TArg4 A4;
		TArg5 A5;
		TArg6 A6;
		TArg7 A7;
		TArg8 A8;
		TArg9 A9;

		explicit Arg9(TArg1 InA1,
		              TArg2 InA2,
		              TArg3 InA3,
		              TArg4 InA4,
		              TArg5 InA5,
		              TArg6 InA6,
		              TArg7 InA7,
		              TArg8 InA8,
		              TArg9 InA9)
					    : A1(InA1)
					    , A2(InA2)
						, A3(InA3)
						, A4(InA4)
						, A5(InA5)
						, A6(InA6)
						, A7(InA7)
						, A8(InA8)
						, A9(InA9)
					  {}

		int32_t Count() const override { return 9; }

		void PushLua(lua_State *L) const override
		{
			StackAPI::Push(L, A1);
			StackAPI::Push(L, A2);
			StackAPI::Push(L, A3);
			StackAPI::Push(L, A4);
			StackAPI::Push(L, A5);
			StackAPI::Push(L, A6);
			StackAPI::Push(L, A7);
			StackAPI::Push(L, A8);
			StackAPI::Push(L, A9);
		}

		void PushLuaTable(lua_State *L) const override
		{
			lua_newtable(L);
			StackAPI::PushTableItem(L, 1, A1);
			StackAPI::PushTableItem(L, 2, A2);
			StackAPI::PushTableItem(L, 3, A3);
			StackAPI::PushTableItem(L, 4, A4);
			StackAPI::PushTableItem(L, 5, A5);
			StackAPI::PushTableItem(L, 6, A6);
			StackAPI::PushTableItem(L, 7, A7);
			StackAPI::PushTableItem(L, 8, A8);
			StackAPI::PushTableItem(L, 9, A9);
		}
	};
}

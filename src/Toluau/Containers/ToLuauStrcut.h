#pragma once

#ifdef TOLUAUUNREAL_API

namespace ToLuau
{
	struct ToLuauStruct
	{
	public:
		ToLuauStruct(UScriptStruct* structType);
		
		ToLuauStruct(uint8* buf,uint32 size,UScriptStruct* uss);
		~ToLuauStruct();

		UScriptStruct* GetType() const;
			
		void* Get() const;

		template<typename T>
		T* GetT()
		{
			if(StructType->IsChildOf(T::StaticStruct))
			{
				return static_cast<T*>(Buf);
			}
			return nullptr;
		}
		
		void InitializeStruct() const;

		void CopyScriptStruct(void* Source);

	private:
		uint8* Buf;
		uint32 Size;
		UScriptStruct* StructType;
	
	};
}

#endif
#include "ToLuauStrcut.h"

#ifdef TOLUAUUNREAL_API

namespace ToLuau
{
	ToLuauStruct::ToLuauStruct(UScriptStruct* structType)
	{
		StructType = structType;
		Size = structType->GetStructureSize();
		Buf = (uint8*)FMemory::Malloc(Size);
	}

	ToLuauStruct::ToLuauStruct(uint8* b,uint32 s,UScriptStruct* u)
		: Buf(b), Size(s), StructType(u) {
	}

	ToLuauStruct::~ToLuauStruct() {
		if (Buf) {
			StructType->DestroyStruct(Buf);
			FMemory::Free(Buf);
			Buf = nullptr;
		}
	}

	UScriptStruct* ToLuauStruct::GetType() const
	{
		return StructType;
	}

	void* ToLuauStruct::Get() const
	{
		return Buf;
	}

	

	void ToLuauStruct::InitializeStruct() const
	{
		StructType->InitializeStruct(Buf);
	}

	void ToLuauStruct::CopyScriptStruct(void* Source)
	{
		StructType->CopyScriptStruct(Buf, Source);
	}
}

#endif
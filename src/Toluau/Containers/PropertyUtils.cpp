#include "PropertyUtils.h"

#ifdef TOLUAUUNREAL_API

#include "API/RegisterMacro.h"
#include "UObject/StrongObjectPtr.h"

LUAU_BEGIN_ENUM(PropertyType)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Byte)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Int8)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Int16)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Int)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Int64)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, UInt16)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, UInt32)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, UInt64)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, UnsizedInt)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, UnsizedUInt)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Float)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Double)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Bool)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, SoftClass)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, WeakObject)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, LazyObject)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, SoftObject)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Class)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Object)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Interface)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Name)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Str)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Array)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Map)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Set)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Struct)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Delegate)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, MulticastDelegate)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Text)
LUAU_REG_CLASS_ENUM_VAR(PropertyType, Enum)
LUAU_END_ENUM(PropertyType)

namespace ToLuau
{
	const FName DefaultLuaPropertyName = "TransientLuaProperty";
	
	UObject* GetPropertyOuter()
	{
		static TStrongObjectPtr<UStruct> PropOuter;
		if (!PropOuter.IsValid()) PropOuter.Reset(NewObject<UStruct>((UObject*)GetTransientPackage()));
		return PropOuter.Get();
	}

	template<typename FP = FProperty>
	FP* NewProperty(UObject* Outer, FFieldClass* Class)
	{
		FP* Prop = CastFieldChecked<FP>(Class->Construct(Outer, DefaultLuaPropertyName, RF_NoFlags));
		if(Prop)
		{
			Prop->ArrayDim = 1;
			return Prop;
		}
		return nullptr;
	}
	
	FProperty* PropertyProto::CreateProperty(const PropertyProto& Proto)
	{
		FProperty* Prop = nullptr;
		UObject* Outer = GetPropertyOuter();
		switch (Proto.Type)
		{
		case PropertyType::Byte:
			Prop = NewProperty(Outer, FByteProperty::StaticClass());
			break;
		case PropertyType::Int8:
			Prop = NewProperty(Outer, FInt8Property::StaticClass());
			break;
		case PropertyType::Int16:
			Prop = NewProperty(Outer, FInt16Property::StaticClass());
			break;
		case PropertyType::Int:
			Prop = NewProperty(Outer, FIntProperty::StaticClass());
			break;
		case PropertyType::Int64: 
			Prop = NewProperty(Outer, FInt64Property::StaticClass());
			break;
		case PropertyType::UInt16:
			Prop = NewProperty(Outer, FUInt16Property::StaticClass());
			break;
		case PropertyType::UInt32:
			Prop = NewProperty(Outer, FUInt32Property::StaticClass());
			break;
		case PropertyType::UInt64:
			Prop = NewProperty(Outer, FUInt64Property::StaticClass());
			break;
		case PropertyType::UnsizedInt:
			Prop = NewProperty(Outer, FUInt64Property::StaticClass());
			break;
		case PropertyType::UnsizedUInt:
			Prop = NewProperty(Outer, FUInt64Property::StaticClass());
			break;
		case PropertyType::Float:
			Prop = NewProperty(Outer, FFloatProperty::StaticClass());
			break;
		case PropertyType::Double:
			Prop = NewProperty(Outer, FDoubleProperty::StaticClass());
			break;
		case PropertyType::Bool:
			Prop = NewProperty(Outer, FBoolProperty::StaticClass());
			break;
		case PropertyType::Object:
			{
				auto NewProp = NewProperty<FObjectProperty>(Outer, FObjectProperty::StaticClass());
				NewProp->SetPropertyClass(Proto.Class);
				Prop = NewProp;
				break;
			}
		case PropertyType::Str:
			Prop = NewProperty(Outer, FStrProperty::StaticClass());
			break;
		default: ;
		}

		if(Prop)
		{
			FArchive Ar;
			Prop->LinkWithoutChangingOffset(Ar);
		}
		return Prop;
	}
}

#endif
#pragma once

#ifdef TOLUAUUNREAL_API
#include "Toluau/ToLuau_API.h"
#else
#include "ToLuau_API.h"
#endif

#ifdef TOLUAUUNREAL_API

enum class PropertyType
{
	Byte,
	Int8,
	Int16,
	Int,
	Int64,
	UInt16,
	UInt32,
	UInt64,
	UnsizedInt,
	UnsizedUInt,
	Float,
	Double,
	Bool,
	SoftClass,
	WeakObject,
	LazyObject,
	SoftObject,
	Class,
	Object,
	Interface,
	Name,
	Str,
	Array,
	Map,
	Set,
	Struct,
	Delegate,
	MulticastDelegate,
	Text,
	Enum,
};

namespace ToLuau
{
	
	template<typename T>
	struct TypeToProp;

	#define TypeToProp(A,B) \
	template<> struct TypeToProp<A> { \
		static const PropertyType Value = PropertyType::B; \
	};\

	TypeToProp(uint8, Byte);
	TypeToProp(int8, Int8);
	TypeToProp(int16, Int16);
	TypeToProp(int, Int);
	TypeToProp(int64, Int64);
	TypeToProp(uint16, UInt16);
	TypeToProp(uint32, UInt32);
	TypeToProp(uint64, UInt64);
	TypeToProp(float, Float);
	TypeToProp(double, Double);
	TypeToProp(bool, Bool);
	TypeToProp(UObject*, Object);
	TypeToProp(FString, Str);

	// definition of property
	struct ToLuau_API PropertyProto {
		PropertyProto(PropertyType InType) :Type(InType), Class(nullptr) {}
		PropertyProto(PropertyType InType, UClass* InClass) :Type(InType), Class(InClass) {}

		template<typename T>
		static PropertyProto get() {
			return PropertyProto(TypeToProp<T>::Value);
		}

		PropertyType Type;
		UClass* Class;

		// create UProperty by PropertyProto
		// returned UProperty should be collect by yourself
		static FProperty* CreateProperty(const PropertyProto& Proto);
	}; 
	
}

#endif
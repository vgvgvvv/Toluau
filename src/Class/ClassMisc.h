#pragma once
#include "Class.h"

namespace ToLuau
{
	namespace ClassLib
	{
		inline bool IsA(const Class* self, const Class* targetClass)
		{
			if(self == nullptr)
			{
				return false;
			}
			if(targetClass == nullptr)
			{
				return true;
			}
			return self->IsA(targetClass);
		}
	}

}

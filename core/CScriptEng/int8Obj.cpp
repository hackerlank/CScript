#include "StdAfx.h"
#include "vm.h"

using namespace runtime;

///////////////////////////////////////////////////////////////////////////////

charObject::charObject()
	: mVal(0)
{
}

uint32_t charObject::GetObjectTypeId() const
{
	return DT_int8;
}

runtimeObjectBase* charObject::Add(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* charObject::Sub(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* charObject::Mul(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* charObject::Div(const runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* charObject::SetValue(runtimeObjectBase *obj)
{
	return nullptr;
}

runtimeObjectBase* charObject::GetMember(const char *memName)
{
	return __super::GetMember(memName);
}

runtimeObjectBase* charObject::doCall(doCallContext *context)
{
	return nullptr;
}

runtimeObjectBase* charObject::getIndex(int i)
{
	return nullptr;
}

stringObject* charObject::toString()
{
	stringObject *s = new ObjectModule<stringObject>;
	StringHelper::Format(*s->mVal, "%c", mVal);
	return s;
}

///////////////////////////////////////////////////////////////////////////////

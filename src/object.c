#include "object.h"

/* A null structure that Object is the base type of. */
ObjectInfo O_GlobalObjectInfoStruct(NULLType);

DEFINE_OBJECT(Object, NULLType, NULL, NULL);

/* Mostly just wrapping the macros defined previously. */
void O_IncRef(void *obj) 
{
    O_INCREF( (Object*)obj); 
}

void O_DecRef(void *obj) 
{ 
    O_DECREF( (Object*)obj); 
}

size_t O_RefCount(const void *obj) 
{
    return O_REF_COUNT( (const Object*)obj); 
}

/* Declare a given object as value-based. */
void O_DeclareObjectAsValue(void *obj)
{
    Object *o = (Object*)obj;
    assert(o != NULL);
    assert(o->_obj_ref_count == 0);
    
    o->_obj_ref_count = -1;
}


/* Manually call the destructor. */
void O_DestroyValueObject(void *obj)
{
    Object *o = (Object*)obj;
    assert(o != NULL);
    assert(o->_obj_ref_count == -1);

    if(o->_obj_type_info->delete_function != NULL)
	(*o->_obj_type_info->delete_function)(o);
}
    
/*  True if a given object is value-based. */
bool O_ObjectIsValue(void *obj)
{
    Object *o = (Object*)obj;
    assert(o != NULL);
    return o->_obj_ref_count == -1;
}

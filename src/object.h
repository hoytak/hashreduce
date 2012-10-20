/*******************************************************************************
 *
 *  Defines the base structure constructs for all program objects.
 *
 *  This file provides a macro-based implementation of an object pool
 *  that allows for efficient handling of large numbers of small
 *  objects (e.g. nodes in a hash table or graph or any other specific
 *  hash objects).  It also provides functions to deal with reference
 *  counting and other memory management routines.
 *
 *  All structs taking advantage of object pools need to include
 *  OBJECT_ITEMS as a macro within 
 *
 *  This header file provides two macros, DECLARE_OBJECT(name)
 *  and DEFINE_OBJECT(name); these should be placed in the header
 *  and source files respectively.
 *
 *******************************************************************************/

#ifndef OBJECT_H
#define OBJECT_H

#include "bitops.h"
#include "memorypool.h"
#include "debugging.h"
#include "optimizations.h"
#include "errorhandling.h"
#include <stdbool.h>

/**********************************************************************
 * 
 *  The basic object pointer, for which casts can be done.  This is a
 *  void*, then casts work by simply checking for type info.  
 *
 **********************************************************************/

typedef void*         obj_ptr;
typedef const void*   cobj_ptr;

/**********************************************************************
 * 
 *  Macros for declaring and defining the wrappers around an existing
 *  object structure type definition, declaring a structure as an
 *  object.  DECLARE_OBJECT() should be in exactly one header file,
 *  and DEFINE_OBJECT() should be in exactly one source file.  The
 *  following functions are then available for and object named
 *  ObjectName:
 *
 * NewObjectName -- returns a new instance of the object, calling the
 * constructor if the constructor function is not NULL.
 *
 * AllocateObjectName -- returns a new instance of the object, without
 * calling the constructor.  This is intended for functions that
 * initialize a batch of objects together.  The structure is zeroed.
 *
 * DeleteObjectName -- deletes the given object, calling the destroy
 * function registered in the type structure before returning it to
 * the object pool.
 *
 * DeallocateObjectName -- returns the object to the pool without
 * calling the destroy function.
 *
 **********************************************************************/


/********************************************************************************
 *
 *  A base structure that contains all the information for each object
 *  type.  This contains a set of function pointers which define
 *  certain operations of the objects, e.g. construction and
 *  destruction.  It also contains a memory pool for creating and
 *  destroying the objects.
 */

typedef void* (*voidobjectfunc)();
typedef void* (*unaryobjectfunc)(void *);
typedef void* (*binaryobjectfunc)(void *, void *);
typedef void (*nullunaryobjectfunc)(void *);
typedef void (*nullbinaryobjectfunc)(void *, void *);

//typedef struct ObjectInfo ObjectInfo;

typedef struct ObjectInfo {
    /* Holds the name of the type. */
    const char *type_name;

    /* Holds the base type if this one extends another, e.g. */
    struct ObjectInfo *base_type;

    /* Functions called when certain events occur.  These must be
     * either set to NULL or set to specific functions upon
     * initialization. */

    /* If not NULL, called immediately after the object is created. */
    nullunaryobjectfunc construction_function;

    /* If not NULL, called before the object is garbage collected. */
    nullunaryobjectfunc delete_function;

    nullunaryobjectfunc deallocate_function;

    /*****************************************
     * Memory management operations. 
     *****************************************/

} ObjectInfo;

/***********************************************************************
 *
 *  OBJECT_ITEMS should be placed at the beginning of all object
 *  structures.  It contains reference counting, memory management
 *  information, and type information.
 *
 **********************************************************************/ 
#ifndef NDEBUG

#define _OBJECT_MAGIC_NUMBER           0x38d829efull
#define _DECLARE_OBJECT_MAGIC_NUMBER   uint32_t _magic_number;
#define _SET_OBJECT_MAGIC_NUMBER(obj)  (obj)->_magic_number = _OBJECT_MAGIC_NUMBER;
#define _OBJECT_MAGIC_NUMBER_MATCHES(obj) ((obj)->_magic_number == _OBJECT_MAGIC_NUMBER)
#else
#define _DECLARE_OBJECT_MAGIC_NUMBER
#define _SET_OBJECT_MAGIC_NUMBER(obj)  
#define _OBJECT_MAGIC_NUMBER_MATCHES(obj) (true)
#endif

#define OBJECT_ITEMS							\
									\
    /* Include memory pool items, as we use them here. */		\
    MEMORY_POOL_ITEMS;							\
    _DECLARE_OBJECT_MAGIC_NUMBER					\
									\
    /* Type information -- points to singleton type structure. */	\
    struct ObjectInfo *_obj_type_info;					\
									\
    /* Reference counting. */						\
    signed long int _obj_ref_count;

/* A single global object info structure is defined for each type. */
#define O_GlobalObjectInfoStruct(ObjectType) (ObjectType##_objinfo)

/* A null structure that Object is the base type of. */
extern ObjectInfo O_GlobalObjectInfoStruct(NULLType);

#define DECLARE_OBJECT(ObjectType)					\
    									\
    extern struct ObjectInfo O_GlobalObjectInfoStruct(ObjectType);	\
									\
    DECLARE_GLOBAL_MEMORY_POOL(ObjectType);				\
    									\
    static inline ObjectType* ALLOCATE##ObjectType()			\
    {									\
	ObjectType *obj = Mp_New##ObjectType();				\
									\
	_SET_OBJECT_MAGIC_NUMBER(obj);					\
	obj->_obj_type_info = &O_GlobalObjectInfoStruct(ObjectType);	\
	assert(obj->_obj_type_info != NULL);				\
	obj->_obj_ref_count = 1;					\
									\
	assert(O_IsType(Object, obj));					\
									\
	return obj;							\
    }									\
									\
    ObjectType* Allocate##ObjectType();					\
    ObjectType* Construct##ObjectType();				\
    void Delete##ObjectType(ObjectType*);				\
    void Deallocate##ObjectType(ObjectType*);				\
									\
    /* Type checking methods. */					\
    void _Tc_##ObjectType##_NullErrorMessage(const char* file, const char* funcname, unsigned long linenumber); \
    void _Tc_##ObjectType##_CastErrorMessage(const Object *_h, const char* file, const char* funcname, unsigned long linenumber); \
									\
    static inline void _Tc_##ObjectType##_TypeCheck(			\
	cobj_ptr _h, const char* file, const char* funcname,		\
	unsigned long linenumber, bool check_null)			\
    {									\
	if(IN_DEBUG_MODE)						\
	{								\
	    const Object *h = (const Object*)_h;			\
									\
	    if(unlikely(h == NULL))					\
	    {								\
		if(check_null)						\
		    _Tc_##ObjectType##_NullErrorMessage(file, funcname, linenumber); \
	    }								\
	    else if(unlikely((!_OBJECT_MAGIC_NUMBER_MATCHES(h))		\
			     || !O_IsType(ObjectType, h)))		\
		_Tc_##ObjectType##_CastErrorMessage(h, file, funcname, linenumber); \
	}								\
    } 


#define DEFINE_OBJECT(ObjectType,					\
		      BaseType,						\
		      construction_function,				\
		      delete_function)					\
									\
    /* Functions that explicitly do the casts for what is stored in	\
     * the construction function to stay within the standard.		\
     * (Technically, casts between functions with different signatures  \
     * is not supported in this case (a struct* to void* difference).   \
     */									\
									\
    extern ObjectInfo O_GlobalObjectInfoStruct(BaseType);		\
    									\
    DEFINE_GLOBAL_MEMORY_POOL(ObjectType);				\
									\
    /* Initializing the object info structure. */			\
    ObjectInfo O_GlobalObjectInfoStruct(ObjectType) = {			\
	/*  *type_name  */    #ObjectType,				\
	/* Base Type */	      &O_GlobalObjectInfoStruct(BaseType),	\
	/* constructor */     (nullunaryobjectfunc)construction_function, \
        /* destructor */      (nullunaryobjectfunc)delete_function,	\
	/* memory_pool */     (nullunaryobjectfunc)Mp_Free##ObjectType##_NonStatic \
    };									\
									\
    /* Functions for reporting casting errors. */			\
    void _Tc_##ObjectType##_NullErrorMessage(				\
	const char* file, const char* funcname,				\
	unsigned long linenumber)					\
    {									\
	fprintf(stderr, "\n\nWARNING: Type-checked cast attempted on NULL pointer."); \
	fprintf(stderr, "\n\nLocation: %s, function %s, line %lu. \n",	\
		file, funcname, linenumber);				\
    }									\
									\
    void _Tc_##ObjectType##_CastErrorMessage(				\
	const Object *h, const char* file,				\
	const char* funcname, unsigned long linenumber)			\
    {									\
	fprintf(stderr, "\n\nERROR: Invalid upcast attempted from type "); \
	if(unlikely(h->_obj_type_info == NULL))				\
	    fprintf(stderr, "NULL TYPE");				\
	else								\
	    fprintf(stderr, "%s", h->_obj_type_info->type_name);	\
									\
	fprintf(stderr, " to type %s.", #ObjectType);			\
									\
	fprintf(stderr, "\nLocation: %s, function %s, line %lu: ",	\
		file, funcname, linenumber);				\
	abort();							\
    }									\
									\
    ObjectType* Construct##ObjectType()					\
    {									\
	ObjectType *obj = ALLOCATE##ObjectType();			\
									\
	if(construction_function != NULL)				\
	    O_GlobalObjectInfoStruct(ObjectType).construction_functio##n(obj); \
									\
	return obj;							\
    }									\
									\
    ObjectType* Allocate##ObjectType()					\
    {									\
	return ALLOCATE##ObjectType();					\
    }									

/*********************************************************************************
 *
 *  Reference counting and memory management.
 *
 ********************************************************************************/

/* Acquire a reference to a hashkey structure. */
void O_IncRef(void *obj);

/* Give back a reference to the hashkey structure; may cause arbitrary
 * code to be executed. */
void O_DecRef(void *obj);

/* Get the current reference count of an object. */
size_t O_RefCount(const void *obj);

/****************************************
 * Stuff to handle value-based object structures (usually a by-value
 * member of another object). 
 ****************************************/

/* Declare a given object as value-based. */
void O_DeclareObjectAsValue(void *obj);

/* Manually call the destructor. */
void O_DestroyValueObject(void *obj);

/*  True if a given object is value-based. */
bool O_ObjectIsValue(void *obj);

/*************************************************************
 *
 *  Type checking routines macros.
 *
 ************************************************************/

#define O_TypeCheckNoNull(ObjectType, obj)			\
  _Tc_##ObjectType##_TypeCheck(obj,__FILE__, __func__, __LINE__, false)

#define O_TypeCheck(ObjectType, obj)					\
  _Tc_##ObjectType##_TypeCheck(obj,__FILE__, __func__, __LINE__, true)

#ifndef NDEBUG

#define O_Cast(ObjectType, obj)					\
  (O_TypeCheck(ObjectType, (obj)), (ObjectType*)(obj))

#define O_CastPtr(ObjectType, obj)				\
  (O_TypeCheckNoNull(ObjectType, (*(Object**)obj)), (ObjectType**)(obj))

#define O_CastC(ObjectType, obj)				\
  (O_TypeCheck(ObjectType, (obj)), (const ObjectType*)(obj))

#define O_CastCPtr(ObjectType, obj)					\
  (O_TypeCheckNoNull(ObjectType, (*(Object**)obj)), (const ObjectType*)(obj))

#else

#define O_Cast(ObjectType, obj)	        ((ObjectType*)(obj))
#define O_CastPtr(ObjectType, obj)	((ObjectType**)(obj))
#define O_CastC(ObjectType, obj)        ((const ObjectType*)(obj))
#define O_CastCPtr(ObjectType, obj)     ((const ObjectType**)(obj))

#endif

extern ObjectInfo O_GlobalObjectInfoStruct(Object);

/* Type checking. */
static inline bool __O_IsTypeCheck(const ObjectInfo *oi, const ObjectInfo *objinfo)
{
#ifdef DEBUG_MODE    
    if(objinfo == NULL)
    {
	ERROR(objinfo != NULL, "Error verifying new type is anscestor of Object; was BaseType specified correctly?");
	assert(objinfo != NULL);
    }
#endif
    assert(oi != NULL);

    while(1)
    {
	assert(objinfo != NULL);
	assert(oi != NULL);

	if(oi == objinfo)
	    return true;
	else if (objinfo == &O_GlobalObjectInfoStruct(Object))
	    return false;
	else
	{
#ifdef DEBUG_MODE	    
	    return __O_IsTypeCheck(oi, objinfo->base_type);
#else
	    objinfo = objinfo->base_type;
#endif
	}
    }
}

/* Returns true if type of cast is type or subtype of object. */
#define O_IsType(ObjectType, obj)					\
    (__O_IsTypeCheck(&O_GlobalObjectInfoStruct(ObjectType), ((const Object*)obj)->_obj_type_info))

#define O_IsExactType(ObjectType, obj)					\
    ( ((const Object*)obj)->_obj_type_info == &O_GlobalObjectInfoStruct(ObjectType))


/************************************************************
 *
 *  The base object type.
 *
 ************************************************************/

typedef struct {
    OBJECT_ITEMS;
}Object;

DECLARE_OBJECT(Object);

/**********************************************************************
 *
 *  Functions for invoking the generic operations.
 *
 **********************************************************************/

/*****************************************
 *
 *  Macros implementing the above.
 *
 ****************************************/

#define O_INCREF(obj)							\
    do {								\
	assert(O_IsType(Object, obj));					\
									\
	Object *objp = (Object*)(obj);					\
									\
	if(likely(objp->_obj_ref_count >= 0))				\
	    ++objp->_obj_ref_count;					\
    } while(0)

#define O_DECREF(obj)							\
    do{									\
	Object *objp = (Object*)(obj);					\
	assert(O_IsType(Object, objp));					\
	assert(objp->_obj_ref_count > 0);				\
    									\
	--objp->_obj_ref_count;						\
    									\
	if(objp->_obj_ref_count == 0)					\
	{								\
	    if(objp->_obj_type_info->delete_function != NULL)		\
		(*(objp->_obj_type_info->delete_function))(objp);	\
									\
	    assert((*(objp->_obj_type_info->deallocate_function)) != NULL); \
	    (*(objp->_obj_type_info->deallocate_function))(objp);	\
	}								\
    }while(0)

#define O_REF_COUNT(obj) ((obj)->_obj_ref_count)

#endif



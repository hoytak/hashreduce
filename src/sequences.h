#ifndef _SEQUENCES_H_
#define _SEQUENCES_H_

#include "object.h"

/* Macros that declare a new sequence object; one that simply supports
   sequences by accumulation and then iteration.  */

#define _REF_COUNT_IN_false(x)
#define _REF_COUNT_OUT_false(x)
#define _REF_COUNT_IN_true(x)   O_INCREF(x)
#define _REF_COUNT_OUT_true(x)  O_DECREF(x)

#define DECLARE_NEW_SEQUENCE_OBJECT(name, base_type, base_type_items,	\
				    item_type, abbreviation, count, do_ref_counting) \
    									\
    typedef struct __##name##_Node_t {					\
	MEMORY_POOL_ITEMS;						\
	size_t size;							\
	struct __##name##_Node_t *next;					\
	item_type items[count];						\
    } __##name##_Node;							\
    									\
    DECLARE_GLOBAL_MEMORY_POOL(__##name##_Node);			\
    									\
    typedef struct {							\
	base_type_items;						\
	__##name##_Node* end_node;					\
	__##name##_Node nodes;						\
	size_t size;							\
    } name;								\
									\
    DECLARE_OBJECT(name);						\
									\
    void _##abbreviation##_Constructor();				\
    void _##abbreviation##_Destructor();				\
									\
    static inline void __##name##__internal_constructor(name *hs)	\
    {									\
	hs->end_node = &(hs->nodes);					\
	hs->size = 0;							\
	hs->nodes.size = 0;						\
    }									\
									\
    static inline name* New##name()					\
    {									\
	name *obj = ALLOCATE##name();					\
      	__##name##__internal_constructor(obj);				\
	return obj;							\
    }									\
									\
    static inline size_t abbreviation##_Size(const name *obj)		\
    {									\
	return obj->size;						\
    }									\
									\
    static inline void abbreviation##_Append(name *hs, item_type h)	\
    {									\
	__##name##_Node *end_node = hs->end_node;			\
	_REF_COUNT_IN_##do_ref_counting(h);				\
									\
	if(unlikely(end_node->size == count))				\
	{								\
	    assert(end_node->next == NULL);				\
	    hs->end_node = end_node = (end_node->next = Mp_New__##name##_Node()); \
	    end_node->size = 1;						\
	    end_node->items[0] = h;					\
	    ++(hs->size);						\
	}								\
	else								\
	{								\
	    assert(end_node->size < _HS_NODE_SIZE);			\
	    end_node->items[end_node->size] = h;			\
	    ++(end_node->size);						\
	    ++(hs->size);						\
	}								\
    }									\
									\
    typedef struct {							\
	MEMORY_POOL_ITEMS;						\
	name *hs;							\
	__##name##_Node *next_unit;					\
	size_t next_index;						\
	item_type current_item;						\
    } name##Iterator;							\
									\
    DECLARE_GLOBAL_MEMORY_POOL(name##Iterator);				\
    									\
    static inline void abbreviation##i_INIT(name *hs, name##Iterator * hsi) \
    {									\
	hsi->hs = hs;							\
	hsi->next_unit  = likely(hs->nodes.size != 0) ? &(hs->nodes) : NULL; \
	hsi->next_index = 0;						\
    }									\
									\
    static inline bool abbreviation##i_NEXT(item_type *hvi, name##Iterator* hsi) \
    {									\
	const __##name##_Node* node = hsi->next_unit;			\
									\
	if(unlikely(node == NULL))					\
	    return false;						\
									\
	assert(hsi->next_index < node->size);				\
									\
	(*hvi)    = node->items[hsi->next_index];			\
	++(hsi->next_index);						\
									\
	if(unlikely(hsi->next_index == node->size))			\
	{								\
	    hsi->next_unit = node->next;				\
	    hsi->next_index = 0;					\
	    								\
	    if(node->next != NULL)					\
		assert(node->next->size >= 1);				\
	}								\
									\
	return true;							\
    }									\
									\
    name##Iterator* abbreviation##i_New(name *hs);			\
    bool abbreviation##i_Next(item_type* hvi, name##Iterator *hsi);	\
    void abbreviation##i_Finish(name##Iterator *hsi);			\

#define DEFINE_NEW_SEQUENCE_OBJECT(name, base_type, base_type_items,	\
				   item_type, abbreviation, count, do_ref_counting) \
    									\
    void _##name##_Constructor(name* hs)				\
    {									\
	__##name##__internal_constructor(hs);				\
    }									\
									\
    void _##name##_Destructor(name* hs)					\
    {									\
	__##name##_Node* dealloc_start = hs->nodes.next;		\
	__##name##_Node* dealloc_x;					\
									\
	while(dealloc_start != NULL) {					\
	    dealloc_x = dealloc_start;					\
	    dealloc_start = dealloc_start->next;			\
	    size_t i;							\
	    for(i = 0; i < dealloc_x->size; ++i) {			\
		_REF_COUNT_OUT_##do_ref_counting(dealloc_x->items[i]);	\
	    }								\
									\
	    Mp_Free__##name##_Node(dealloc_x);				\
	}								\
    }									\
									\
    DEFINE_OBJECT(name, base_type, _##name##_Constructor,		\
		  _##name##_Destructor, NULL);				\
									\
    DEFINE_GLOBAL_MEMORY_POOL(__##name##_Node);				\
    DEFINE_GLOBAL_MEMORY_POOL(name##Iterator);				\
									\
    name##Iterator* abbreviation##i_New(name *hs)			\
    {									\
	name##Iterator *hsi = Mp_New##name##Iterator();			\
	abbreviation##i_INIT(hs, hsi);					\
	O_INCREF(hs);							\
	return hsi;							\
    }									\
									\
    bool abbreviation##i_Next(item_type *hvi, name##Iterator* hsi)	\
    {									\
	return abbreviation##i_NEXT(hvi, hsi);				\
    }									\
									\
    void abbreviation##i_Finish(name##Iterator* hsi)			\
    {									\
	O_DECREF(hsi->hs);						\
	Mp_Free##name##Iterator(hsi);					\
    }									\

#endif /* _ACCUMULATOR_H_ */

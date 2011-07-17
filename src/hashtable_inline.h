#ifndef _HASHTABLE_INLINE_H_
#define _HASHTABLE_INLINE_H_

static inline size_t Ht_SIZE(const HashTable *ht) 
{
    return ht->size;
}

static inline void Ht_SWAP(ht_rptr ht1, ht_rptr ht2)
{
    if(unlikely(ht1 == ht2))
	return;

    size_t a1 = ht2->size;           
    ht2->size = ht1->size; 
    ht1->size = a1;

    _HT_Node *table_buf = ht2->table;  
    ht2->table = ht1->table; 
    ht1->table = table_buf;
    
    size_t a2 = ht2->first_element;  
    ht2->first_element = ht1->first_element;
    ht1->first_element = a2;

    size_t a3 = ht2->_table_size;
    ht2->_table_size = ht1->_table_size;
    ht1->_table_size = a3;

    size_t a4 = ht2->_table_grow_trigger_size;
    ht2->_table_grow_trigger_size = ht1->_table_grow_trigger_size;
    ht1->_table_grow_trigger_size = a4;

    unsigned int i1 = ht2->_table_shift;
    ht2->_table_shift = ht1->_table_shift;
    ht1->_table_shift = i1;

    unsigned int i2 = ht2->_table_log2_size;
    ht2->_table_log2_size = ht1->_table_log2_size;
    ht1->_table_log2_size = i2;

    _HT_MarkerSkipList *msl = ht2->marker_sl;
    ht2->marker_sl = ht1->marker_sl;
    ht1->marker_sl = msl;
}

static inline void _Hti_INIT(ht_crptr ht, _HashTableInternalIterator *hti)
{
    /* Does not deal with ref counting at all to allow constness of ht. */

    hti->number_left = Ht_SIZE(ht);

    hti->current_base_node = ht->table;

    if(likely(hti->number_left != 0))
    {
	while(hti->current_base_node->size == 0)
	    ++(hti->current_base_node);

	hti->next_node = hti->current_base_node;
    }

    hti->next_node = hti->current_base_node;
    hti->next_index = 0;

#ifndef NDEBUG
    hti->ht = ht;
#endif
}

static inline bool _Hti_NEXT(HashObject** h_dest, _HashTableInternalIterator *hti)
{
    if(unlikely(hti->number_left == 0))
    {
#ifndef NDEBUG
	++(hti->current_base_node);

	while(hti->current_base_node != hti->ht->table + hti->ht->_table_size)
	{
	    assert(hti->current_base_node->size == 0);
	    ++(hti->current_base_node);
	}
#endif    
	return false;
    }

    *h_dest = hti->next_node->items[hti->next_index].obj;

    assert(*h_dest != NULL);
    assert(O_IsType(HashObject, *h_dest));

#ifndef NDEBUG
    hti->debug_current_index = hti->next_index;
    hti->debug_current_node = hti->next_node;
#endif
    
    --hti->number_left;

    if(likely(hti->number_left != 0))
    {
	++(hti->next_index);
    
	assert(hti->next_index <= hti->next_node->size);

	if(hti->next_index == hti->next_node->size)
	{
	    if(unlikely(hti->next_node->next_chain != NULL))
	    {
		assert(hti->next_index == _HT_ITEMS_PER_NODE);
		hti->next_node = &(hti->next_node->next_chain->node);
		assert(hti->next_node->size >= 1);
		hti->next_index = 0;
	    }
	    else
	    {
		while((++(hti->current_base_node))->size == 0)
		    assert(hti->current_base_node != hti->ht->table + hti->ht->_table_size);

		hti->next_node = hti->current_base_node;
		hti->next_index = 0;
	    }
	}
    }

    return true;
}

static inline bool Hti_NEXT(HashObject** h_dest, HashTableIterator *hti)
{
    return _Hti_NEXT(h_dest, &(hti->hti));
}

static inline bool Htib_NEXT(HashObject** h_dest, HashTableBufferedIterator *htib)
{
    if(unlikely(htib->left == 0))
	return false;
    
    *h_dest = *htib->next;
    
    ++(htib->next), --(htib->left);

    return true;
}

void _Ht_MSL_Init(HashTable *ht);

static inline void Htmi_INIT(HashTable *ht, HashTableMarkerIterator *htmi)
{
    htmi->ht = ht;

    assert(ht != NULL);

    if(unlikely(ht->marker_sl == NULL))
	_Ht_MSL_Init(ht);

    _HT_MarkerSkipList *msl = ht->marker_sl;

    Hk_CLEAR(&(htmi->current_item.hk));
    htmi->current_item.start = MARKER_MINUS_INFTY;
    htmi->current_item.end   = MARKER_MINUS_INFTY;

    _HT_MSL_Node *leaf = (_HT_MSL_Node *)(msl->first_leaf);
    
    assert(leaf != NULL);
    assert(leaf->marker == MARKER_MINUS_INFTY);

    htmi->next = leaf;
}

static inline bool Htmi_NEXT(HashValidityItem* hvi, HashTableMarkerIterator * htmi)
{
    if(unlikely(htmi->next == NULL))
	return false;
    
    htmi->current_item.start = htmi->current_item.end;
    Hk_REDUCE_UPDATE(&(htmi->current_item.hk), &(htmi->next->hk));

    do {
	htmi->next = (_HT_MSL_Node*)(htmi->next->next);
	
	if(unlikely(htmi->next == NULL))
	{
	    htmi->current_item.end = MARKER_PLUS_INFTY;
	    break;
	}
	
	htmi->current_item.end = htmi->next->marker;

    }while(unlikely(Hk_ISZERO(&(htmi->next->hk))));

    *hvi = htmi->current_item;
    return true;
}

    
static inline void Hsi_INIT(HashSequence *hs, HashSequenceIterator* hsi)
{
    hsi->hs = hs;
    hsi->next_unit  = likely(hs->nodes.size != 0) ? &(hs->nodes) : NULL;
    hsi->next_index = 0;
}

static inline bool Hsi_NEXT(HashValidityItem *hvi, HashSequenceIterator* hsi)
{
    const _HS_Node* node = hsi->next_unit;

    if(unlikely(node == NULL))
	return false;

    assert(hsi->next_index < node->size);

    hvi->hk    = node->items[hsi->next_index].hk;
    hvi->start = node->items[hsi->next_index].marker;

    ++(hsi->next_index);

    if(unlikely(hsi->next_index == node->size))
    {
	hsi->next_unit = node->next;
	hsi->next_index = 0;

	if(node->next != NULL)
	{
	    assert(node->next->size >= 1);
	    assert(node->next->items[0].marker > node->items[node->size - 1].marker);
	}

	hvi->end = likely(node->next != NULL) ? node->next->items[0].marker : MARKER_PLUS_INFTY;
    }
    else
    {
	hvi->end = node->items[hsi->next_index].marker;
    }

    return true;
}

void _Ht_MSL_Drop(HashTable *ht);

/* Clear cache if needed. */
static inline void Ht_ClearMarkerCache(ht_rptr ht)
{
    if(ht->marker_sl != NULL)
	_Ht_MSL_Drop(ht);
}

#endif /* _HASHTABLE_INLINE_H_ */

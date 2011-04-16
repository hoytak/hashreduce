static inline size_t _Ht_Table_Index(const unsigned int shift, uint64_t hk64)
{
    size_t idx = (size_t)( (hk64[HK64I(0)]) >> (shift));
    assert(idx < ht->_table_size);
    return idx;
}

static inline void _Ht_Table_Setup(HashTable *ht, unsigned int log2_size)
{
    ht->_table_log2_size = log2_size;
    ht->_table_size = (1 << (ht->_table_log2_size));
    ht->_table_grow_trigger_size = (1 << (1 + ht->_table_log2_size));
    ht->table = (_HT_Node*)malloc(sizeof(_HT_Node)*(ht->_table_size));
    CHECK_MALLOC(ht->table);
    memset(ht->table, 0, sizeof(_HT_Node)*(ht->_table_size));
}

HashObject* _Ht_Table_InsertIntoOverflowNode(
    _HT_Node * _restrict_ node, const _HT_item hi, const bool overwrite)
    
/* Returns true on success, otherwise, _HT_Item* hi gets punted to next one. */
static inline HashObject* _Ht_Table_InsertIntoNode(
    _HT_Node * _restrict_ node, const _HT_item hi, const bool overwrite)
{
    assert(idx < ht->_table_size);
    assert(node->size <= _HT_ITEMS_PER_NODE);

    /* Get the right location in the node list. */ 
    if(node->size == 0)
    {
	++node->size;
	node->items[0] = hi;
	return;
    }

    unsigned int insert_pos = 0;

    switch(node->size)
    {
    case 4:
	if(unlikely(node->items[3].hk64 < hi.hk64))
	{
	    _Ht_Table_InsertIntoOverflowNode(node, hi, overwrite);
	    return;
	}
    case 3:
	if(node->items[2].hk64 > hi.hk64)
	{
	    insert_pos = 3;
	    break;
	}
    case 2:
	if(node->items[1].hk64 > hi.hk64)
	{
	    insert_pos = 2;
	    break;
	}
    case 1:
	if(node->items[0].hk64 > hi.hk64)
	{
	    insert_pos = 1;
	    break;
	}
    default:
	assert(false);
    }

    /* Now see if it's replacing one or the 64bit hash version conflicts. */
    if(unlikely(hi.hk64 == node->items[insert_pos].hk64))
    {
	if(likely(H_EQUAL(hi.obj, node->items[insert_pos].obj)))
	{
	    if(overwrite)
	    {
		O_INCREF(hi.obj);
		H_CLAIM_LOCK(hi.obj);
		O_DECREF(node->items[insert_pos].obj);
		H_RELEASE_LOCK(node->items[insert_pos].obj);
		node->items[insert_pos] = hi;
	    }

	    return node->items[insert_pos].obj;
	}
	else 
	{
	    /* A definite corner case; occurs naturally with probability ~ 2^-64 */
	    for(;insert_pos < node->size 
		    && Hk_LT(H_Hash_RO(node->items[insert_pos].obj), H_Hash_RO(hi.obj));
		++insert_pos);

	    /* Did we run off the end?  If so, punt this one to the next node. */
	    if(insert_pos == node->size)
	    {
		if(node->size == _HT_ITEMS_PER_NODE)
		    return _Ht_Table_InsertIntoOverflowNode(node, hi, overwrite);
	    }
	    
	    /* Deal with the case where it's equal; deal locally with this node, and we're done */
	    if(H_EQUAL(hi.obj, node->items[insert_pos].obj))
	    {
		if(overwrite)
		{
		    /* incref first, in case they are the same. */
		    O_INCREF(hi.obj);
		    H_CLAIM_LOCK(hi.obj);
		    O_DECREF(node->items[insert_pos].obj);
		    H_RELEASE_LOCK(node->items[insert_pos].obj);
		    node->items[insert_pos] = hi;
		}
		return node->items[insert_pos].obj;
	    }
	}
    }

    if(unlikely(node->size) == _HT_ITEMS_PER_NODE)
	_Ht_Table_InsertIntoOverflowNode(node, node->items[3], overwrite);
    else
	++node->size;

    unsigned int copy_dest = node->size - 1; 
    
    for(;copy_dest != insert_pos; --copy_dest)
	node->items[copy_dest] = node->items[copy_dest - 1];

    node->items[insert_pos] = hi;

    return hi.obj;
}

void _Ht_Table_InsertIntoOverflowNode(
    _HT_Node * _restrict_ node, const _HT_item hi, const bool overwrite)
{
    if(node->next_chain == NULL)
	node->next_chain = MP_New_HT_Independent_Node();
	    
    _Ht_Table_InsertIntoNode(&(node->next_chain->node), hi, overwrite);
}

void _Ht_Table_DeallocateChain(_HT_Independent_Node * _restrict_ inode)
{
    assert(inode != NULL)

    do{
	for(j = 0; j < inode.node.size; ++j)
	    _Ht_Table_Insert(ht, inode.node.items[j]);

	_HT_Independent_Node *next_node = inode.node.next_chain;
	MP_Free_HT_Independent_Node(inode);
	inode = next_node;
    }while(unlikely(inode != NULL));
}

static inline void _Ht_Table_Insert(HashTable *ht, _HT_Item src)
{
    size_t idx = _Ht_Table_Index(ht->_table_shift, src.hk64);

    while(unlikely(!_Ht_Table_InsertIntoNode(&(ht->table[idx]), src, true)))
    {
	++idx; 
	if(unlikely(idx >= ht->_table_size))
	{
	    /* This is really unlikely. */
	    ht->_table_size += _HT_TABLE_ITEM_BUFFER_COUNT;
	    ht->table = (_HT_Node*)realloc(ht->table, sizeof(_HT_Node)*(ht->_table_size));
	}
    }
}

void _Ht_Table_Grow(HashTable *ht)
{
    _HT_Node* _restrict_ src_tbl = ht->table;
    const size_t src_size = ht->_table_size;

    _Ht_Table_Setup(ht, ht->_table_log2_size + 1);

    size_t i;
    unsigned int j;

    for(i = 0; i < src_size; ++i)
    {
	assert(src_table[i].size <= _HT_ITEMS_PER_NODE);

	for(j = 0; j < src_table[i].size; ++j)
	    _Ht_Table_Insert(ht, src_table[i].items[j]);
	
	if(src_table[i].next_chain != NULL)
	{
	    _HT_Independent_Node *inode = src_table[i].next_chain;
	    
	    do{
		for(j = 0; j < inode.node.size; ++j)
		    _Ht_Table_Insert(ht, inode.node.items[j]);

		inode = inode.node.next_chain;
	    }while(unlikely(inode != NULL));

	    _Ht_Table_DeallocateChain(src_table[i].next_chain);
	}
    }

    free(src_tbl);
}

static inline void _Ht_GrowIfNeeded(HashTable *ht)
{
    if(unlikely(ht->size >= ht->_table_grow_trigger_size))
	_Ht_Grow(ht);
}

static inline _HT_Item _Ht_Table_MakeItem(const HashObject *h)
{
    _HT_Item hi;
    hi.hk64 = H_Hash_RO(h)->hk64[HK64I(0)];
    hi.obj = h;
    return hi;
}

static inline HashObject* _Ht_Table_Insert(HashTable *ht, HashObject *h, bool overwrite)
{
    _Ht_GrowIfNeeded(ht);

    _HT_Item hi = _Ht_Table_MakeItem(h);
    size_t idx = _Ht_Table_Index(ht, hi.hk64);

    return _Ht_Table_InsertIntoNode(&(ht->table[idx]), hi, overwrite);
}

/* Now look at simply finding items. */
static inline bool _Ht_Table_Find(_HT_Node **target_node, unsigned int *idx, 
				  _HT_Independent_Node **idp_node, HashObject **target_h, 
				  const HashTable *ht, const HashKey hk)
{
    size_t idx = _Ht_Table_Index(ht, hi.hk64);
    
    _HT_Node * _restrict_ node = ht->table[idx];
    *idp_node = NULL;

    uint64_t hk64 = hk.hk64[HK86I(0)];

HT_TABLE_FIND_RESTART:;

    unsigned int pos = 4;

    switch(node->size)
    {
    case 4:
	if(unlikely(node->items[3].hk64 < hk64))
	{
	    if(node->next_chain != NULL)
	    {
		node = &(node->next_chain.node);
		*idp_node = node->next_chain;
		goto HT_TABLE_FIND_RESTART;
	    }
	    else
		return false;
	}
	if(node->items[3].hk64 == hk64)
	{
	    pos = 3;
	    break;
	}
    case 3:
	if(node->items[2].hk64 == hk64)
	{
	    pos = 2;
	    break;
	}
    case 2:
	if(node->items[1].hk64 == hk64)
	{
	    pos = 1;
	    break;
	}
    case 1:
	if(node->items[0].hk64 == hi.hk64)
	{
	    pos = 0;
	    break;
	}
    default:
	assert(node->size != 0);
	
	/* None of the conditions have caught it; so it's not in this
	 * table. */
	   
	return false;
    }

    /* Now get to test if things are really equal. */
    if(likely(Hk_EQUAL(H_Hash_RO(node->items[pos].obj), &hk)))
    {
	*target_node = node;
	*target_h = node->items[pos].obj;
	*idx = pos;
	return true;
    }
    else
    {
	/* First make sure it's not in any of the other equal nodes. */
	unsigned int s_pos = pos;

	while(s_pos && node->items[--s_pos].hk64 == hk64)
	{
	    if(likely(Hk_EQUAL(H_Hash_RO(node->items[s_pos].obj), &hk)))
	    {
		*target_node = node;
		*target_h = node->items[s_pos].obj;
		*idx = s_pos;
		return true;
	    }
	}

	if(unlikely(pos == 3))
	{
	    /* It may be in the next level. */
	    if(node->next_chain != NULL)
	    {
		node = &(node->next_chain.node);
		*idp_node = node->next_chain;
		goto HT_TABLE_FIND_RESTART;
	    }
	}

	return false;
    }
}

void _Ht_Table_SlideFromChainedNode(_HT_Node * _restrict_ node);

/* Returns true if the node is now empty, otherwise false. */
static inline bool _Ht_Table_ClearFromNode(_HT_Node * _restrict_ node, unsigned int idx)
{
    assert(node->size <= 4);
    assert(node->size >= 1);
    assert(idx < node->size);
    
    unsigned int i;
    for(i = idx; i + 1 < node->size; ++i)
	node->items[i] = node->items[i+1];

    if(unlikely(node->next_chain != NULL))
    {
	_Ht_Table_SlideFromChainedNode(node);
	return false;
    }
    else
    {
	--node->size;
	node->items[node->size].hk64 = 0;
	node->items[node->size].obj = NULL;
	return (node->size == 0);
    }
}

void _Ht_Table_SlideFromChainedNode(_HT_Node * _restrict_ node)
{
    assert(node->size = 4);
    assert(node->next_chain != NULL);
	
    dest->item[3] = node->next_chain.node.item[0];

    if(_Ht_Table_ClearFromNode(&(node->next_chain.node), 0))
    {
	MP_Free_HT_Independent_Node(node->next_chain);
	node->next_chain = NULL;
    }
}


static inline HashObject* _Ht_Table_Pop(HashTable *ht, const HashKey hk)
{
    _HT_Node *node = NULL;
    _HT_Independent_Node *idp_node = NULL;
    unsigned int idx;
    HashObject *h = NULL;

    if(unlikely(!_Ht_Table_Find(&node, &idx, &idp_node, &h, ht, hk)))
	return NULL;

    if(_Ht_Table_ClearFromNode(node, idx) && idp_node != NULL)
	MP_Free_HT_Independent_Node(idp_node);
    
    return h;
}

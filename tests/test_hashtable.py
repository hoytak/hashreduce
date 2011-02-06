#!/usr/bin/env python
import unittest, random
from common import *
from pprint import *

dgl.Mr_Plus_Infinity.restype = c_long
dgl.Mr_Minus_Infinity.restype = c_long

mr_plus_infinity = dgl.Mr_Plus_Infinity()
mr_minus_infinity = dgl.Mr_Minus_Infinity()

dgl.Hti_New.restype = ctypes.c_void_p
dgl.Htib_New.restype = ctypes.c_void_p
dgl.Ht_Summarize_Add.restype = ctypes.c_void_p
dgl.Ht_Summarize_Finish.restype = ctypes.c_void_p

def getHashTreeSet(ht):
    
    hti = dgl.Hti_New(ht)
    
    s = set()

    while True:
        hk = dgl.Hti_Next(hti)
        
        if hk == 0:
            break
        else:
            h = extractHash(hk)
            assert(h not in s)
            s.add(h)

    
    htib = dgl.Htib_New(ht)
    
    sb = set()

    while True:
        hk = dgl.Htib_Next(htib)
        
        if hk == 0:
            break
        else:
            h = extractHash(hk)
            assert(h not in sb)
            sb.add(h)

    assert(s == sb)

    return s

newHT = dgl.NewHashTable
newHT.restype = ctypes.c_void_p

def getHashAtMarkerLoc(ht, m):
    hk = dgl.Ht_HashAtMarkerPoint(0, ht, c_long(m))
    s = extractHash(hk)
    dgl.O_DecRef(hk)
    
    return s

def getHashMList(ht, marker_points):
    
    hl = []

    for m in marker_points:
        hl.append(getHashAtMarkerLoc(ht, m))

    return hl

def allConsistentlyDistinct(*args):

    s = set(args[0])

    if len(s) != 1:
        return False

    for a in args[1:]:
        
        sa = set(a)
        if len(sa) != 1:
            return False
        
        if len(s.intersection(sa)) != 0:
            return False
        
        s = s.union(sa)

    return True

def hashesConsistent(ht, *loc_lists):
    hash_lists = [getHashMList(ht, loc_list) for loc_list in loc_lists]

    # For printing
    
    if not allConsistentlyDistinct(*hash_lists):
        print "\n\nERROR DUMP:"
        pprint([dict(zip(ll, hl)) for ll, hl in zip(loc_lists, hash_lists)])
        return False
    else:
        return True
    


class TestHashTableBasic(unittest.TestCase):
    """
    Just a few hash keys tests that are more sanity checks then full
    tests.  
    """

    def test01a_CreateDelete(self):
        ht = newHT()
        self.assert_(dgl.O_RefCount(ht) == 1, "refcount = %d" % dgl.O_RefCount(ht) )
        dgl.O_DecRef(ht)

    def test01b_SimpleRetrieval(self):
        ht = newHT()
        self.assert_(dgl.O_RefCount(ht) == 1)
        hk = makeHashKey(0)

        dgl.Ht_Give(ht, hk)
        
        self.assert_(dgl.Ht_Get(ht, hk) == hk)

        dgl.O_DecRef(ht)

    def test02_Sizes(self):
        ht = newHT()
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)

        self.assert_(dgl.Ht_Size(ht) == 0)

        dgl.Ht_Give(ht, hk1)

        self.assert_(dgl.Ht_Size(ht) == 1)

        dgl.Ht_Give(ht, hk2)

        self.assert_(dgl.Ht_Size(ht) == 2)

        dgl.Ht_Pop(ht, hk1)

        self.assert_(dgl.Ht_Size(ht) == 1)
        
        dgl.Ht_Pop(ht, hk2)

        self.assert_(dgl.Ht_Size(ht) == 0)

        dgl.O_DecRef(ht)

    def test03_RetrieveOriginal(self):

        ht = newHT()
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(0)

        self.assert_(hk1 != hk2)

        dgl.Ht_Give(ht, hk1)

        self.assert_(dgl.Ht_Get(ht, hk2) == hk1)

        dgl.O_DecRef(ht)

    def test04_SimpleIterator(self):
        ht = newHT()
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)

        dgl.Ht_Give(ht, hk1)
        dgl.Ht_Give(ht, hk2)

        strue = set([extractHash(hk1), extractHash(hk2)])

        s = getHashTreeSet(ht)

        self.assert_(s == strue, '%s != %s (true)' % (str(s), str(strue)))

        dgl.O_DecRef(ht)
        
    def checkHkList(self, hashkeys, null_hash_keys = []):

        ht = newHT()
        
        hashes = [extractHash(hk) for hk in hashkeys]

        count = 0
        
        for i, hk in enumerate(hashkeys):
            for hk2 in hashkeys[i:]:
                self.assert_(not dgl.Ht_Contains(ht, hk2))
                self.assert_(dgl.Ht_Clear(ht, hk2) == 0)

            self.assert_(dgl.Ht_Size(ht) == count, 
                         "%d (given) != %d (count)" % (dgl.Ht_Size(ht), count))

            dgl.Ht_Give(ht, hk)
            
            count += 1
            
            self.assert_(dgl.Ht_Size(ht) == count, 
                         "%d (given) != %d (count)" % (dgl.Ht_Size(ht), count))

        for hk in hashkeys:
            self.assert_(dgl.Ht_Get(ht, hk) == hk)

            self.assert_(dgl.Ht_Size(ht) == count)

            for hk2 in null_hash_keys:
                self.assert_(dgl.Ht_Clear(ht, hk2) == 0)
                self.assert_(dgl.Ht_Size(ht) == count, 
                             "%d (given) != %d (count)" % (dgl.Ht_Size(ht), count))


        s = getHashTreeSet(ht)


        self.assert_(s == set(hashes))

        for hk in hashkeys:
            dgl.Ht_Clear(ht, hk)

            count -= 1
            
            self.assert_(dgl.Ht_Size(ht) == count)

            for hk2 in null_hash_keys:
                self.assert_(dgl.Ht_Clear(ht, hk2) == 0)
                self.assert_(dgl.Ht_Size(ht) == count, 
                             "%d (given) != %d (count)" % (dgl.Ht_Size(ht), count))


        dgl.O_DecRef(ht)

        
    def test05_Corner_01_Close_1(self):
        self.checkHkList(
            [exactHashKey("00000000000000000000000000000000"),
             exactHashKey("00000000000000000000000000000001")])

    def test05_Corner_01_Close_2(self):
        self.checkHkList(
            [exactHashKey("00000000000000000000000000000000"),
             exactHashKey("00000000000000000000000000000001"),
             exactHashKey("00000000000000000000000000000002")])

    def test05_Corner_01_Close_3(self):
        self.checkHkList(
            [exactHashKey("00000000000000000000000000000000"),
             exactHashKey("00000000000000000000000000000001"),
             exactHashKey("00000000000000000000000000000002"),
             exactHashKey("00000000000000000000000000000003"),
             exactHashKey("00000000000000000000000000000004"),
             exactHashKey("00000000000000000000000000000005"),
             exactHashKey("00000000000000000000000000000006")])

    def test05_Corner_02_2LevelClose_1(self):
        self.checkHkList(
            [exactHashKey("01000000000000000000000000000000"),
             exactHashKey("02000000000000000000000000000000")])

    def test05_Corner_02_2LevelClose_2(self):
        self.checkHkList(
            [exactHashKey("01000000000000000000000000000000"),
             exactHashKey("02000000000000000000000000000000"),
             exactHashKey("03000000000000000000000000000000")])

    def test05_Corner_02_2LevelClose_3(self):
        self.checkHkList(
            [exactHashKey("01000000000000000000000000000000"),
             exactHashKey("02000000000000000000000000000000"),
             exactHashKey("03000000000000000000000000000000"),
             exactHashKey("04000000000000000000000000000000"),
             exactHashKey("05000000000000000000000000000000"),
             exactHashKey("06000000000000000000000000000000"),
             exactHashKey("07000000000000000000000000000000")])

    def test05_Corner_03_1LevelClose_1(self):
        self.checkHkList(
            [exactHashKey("10000000000000000000000000000000"),
             exactHashKey("20000000000000000000000000000000")])

    def test05_Corner_03_1LevelClose_2(self):
        self.checkHkList(
            [exactHashKey("10000000000000000000000000000000"),
             exactHashKey("20000000000000000000000000000000"),
             exactHashKey("30000000000000000000000000000000")])

    def test05_Corner_03_1LevelClose_3(self):
        self.checkHkList(
            [exactHashKey("10000000000000000000000000000000"),
             exactHashKey("20000000000000000000000000000000"),
             exactHashKey("30000000000000000000000000000000"),
             exactHashKey("40000000000000000000000000000000"),
             exactHashKey("50000000000000000000000000000000"),
             exactHashKey("60000000000000000000000000000000"),
             exactHashKey("70000000000000000000000000000000")])


    def test10_LargeContainmentTest(self):

        n = 1000
        hashkeys = [makeHashKey(i) for i in range(n)]

        self.checkHkList(hashkeys)

    def test12_Regression_RegiveReferenceCounting(self):
        
        ht = newHT()
        hk = makeMarkedHashKey(0, 0, 5)
                
        for i in xrange(50):
            self.assert_(dgl.O_RefCount(hk) == 1)
            dgl.Ht_Give(ht, hk)
            
        dgl.O_DecRef(ht)

    def test13_Regression_HashSequence(self):
        
        ht = newHT()
        hkl = [makeHashKey("n7"),
               makeHashKey("n-3"),
               makeHashKey("n5"),
               makeHashKey("n9"),
               makeHashKey("n-1"),
               makeHashKey("n-5")]

        for hk in hkl:
            dgl.Ht_Give(ht, hk)

        self.assert_(dgl.Ht_Get(ht, makeHashKey("n-4")) == 0)

        dgl.O_DecRef(ht)

    def test14_BadClearIneffective(self):

        ht = newHT()

        dgl.Ht_Give(ht, makeHashKey(0))
        dgl.Ht_Give(ht, makeHashKey(1))
        dgl.Ht_Give(ht, makeHashKey(2))

        self.assert_(dgl.Ht_Size(ht) == 3)

        h = makeHashKey(2)
        self.assert_(dgl.Ht_Clear(ht,h) == 1)
        dgl.O_DecRef(h)

        self.assert_(dgl.Ht_Size(ht) == 2)

        h = makeHashKey(10)
        self.assert_(dgl.Ht_Clear(ht, h) == 0)
        dgl.O_DecRef(h)

        self.assert_(dgl.Ht_Size(ht) == 2)

        dgl.O_DecRef(ht)

    def test15_Corner_Regression_PopOnEmpty(self):

        ht = newHT()

        self.assert_(dgl.Ht_Size(ht) == 0)

        h = makeHashKey(0)
        self.assert_(dgl.Ht_Clear(ht, h) == 0)
        dgl.O_DecRef(h)

        self.assert_(dgl.Ht_Size(ht) == 0)

        dgl.O_DecRef(ht)

        
################################################################################
# Reference Counting / Locking stuff

    def testR01_Set(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_Set(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)
        
    def testR02_Clear(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.Ht_Set(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_Clear(ht, h)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(ht)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)

    def testR03_Give(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)
        dgl.O_IncRef(h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_Give(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)
        
    def testR04_Pop(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_Give(ht, h)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_Pop(ht, h)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)

    def testR05_SetDefault(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_SetDefault(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)

    def testR06_Replace(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        h2 = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h2) == 1, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 0, dgl.H_LockCount(h2))
        
        dgl.Ht_Set(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_Set(ht, h2)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        self.assert_(dgl.O_RefCount(h2) == 2, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 1, dgl.H_LockCount(h2))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        self.assert_(dgl.O_RefCount(h2) == 1, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 0, dgl.H_LockCount(h2))

        dgl.O_DecRef(h)
        dgl.O_DecRef(h2)

    def testR07_ReplaceDefault(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        h2 = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h2) == 1, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 0, dgl.H_LockCount(h2))
        
        dgl.Ht_Set(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_SetDefault(ht, h2)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        self.assert_(dgl.O_RefCount(h2) == 1, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 0, dgl.H_LockCount(h2))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        self.assert_(dgl.O_RefCount(h2) == 1, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 0, dgl.H_LockCount(h2))

        dgl.O_DecRef(h)
        dgl.O_DecRef(h2)

    def testR08_ReplaceWithSelf(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_Set(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_Set(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)

    def testR09_ReplaceWithSelfPassive(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_Set(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_SetDefault(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)

    def testR10_GiveReplaceDefault(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)
        dgl.O_IncRef(h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        h2 = makeHashKey(0)
        dgl.O_IncRef(h2)

        self.assert_(dgl.O_RefCount(h2) == 2, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 0, dgl.H_LockCount(h2))
        
        dgl.Ht_Give(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_Give(ht, h2)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        self.assert_(dgl.O_RefCount(h2) == 2, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 1, dgl.H_LockCount(h2))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        self.assert_(dgl.O_RefCount(h2) == 1, dgl.O_RefCount(h2))
        self.assert_(dgl.H_LockCount(h2) == 0, dgl.H_LockCount(h2))

        dgl.O_DecRef(h)
        dgl.O_DecRef(h2)

    def testR11_GiveReplaceWithSelf(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)
        dgl.O_IncRef(h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_Give(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_Give(ht, h)

        self.assert_(dgl.O_RefCount(h) == 2, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.O_DecRef(ht)
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)

    def testR04_PopAltQuery(self):
        ht = newHT()

        self.assert_(dgl.O_RefCount(ht) == 1, dgl.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))
        
        dgl.Ht_Give(ht, h)

        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 1, dgl.H_LockCount(h))

        dgl.Ht_Pop(ht, makeHashKey(0))
        
        self.assert_(dgl.O_RefCount(h) == 1, dgl.O_RefCount(h))
        self.assert_(dgl.H_LockCount(h) == 0, dgl.H_LockCount(h))

        dgl.O_DecRef(h)


################################################################################
# Tests about the marker stuff.
class TestHashTableMarkedKeys(unittest.TestCase):
    def testM_Basic_01_Existance(self):
        
        ht = newHT()
        hk = makeMarkedHashKey(0, 0, 5)
        
        dgl.Ht_Give(ht, hk)

        self.assert_(dgl.Ht_Contains(ht, hk))

        self.assert_(not dgl.Ht_ContainsAt(ht, hk, -1))
        self.assert_(dgl.Ht_ContainsAt(ht, hk, 0))
        self.assert_(not dgl.Ht_ContainsAt(ht, hk, 5))
        self.assert_(not dgl.Ht_ContainsAt(ht, hk, 6))

        dgl.O_DecRef(ht)

    def testM01_Hashes_Simple(self):
        ht = newHT()
        h = makeMarkedHashKey(0, 0, 5)
        dgl.Ht_Give(ht, h)

        self.assert_(hashesConsistent(ht, [-1, 6], [1, 4]))
        dgl.O_DecRef(ht)

    def testM01n_Hashes_Simple(self):
        ht = newHT()
        dgl.Ht_Give(ht, makeMarkedHashKey(0, -10, -5))
        self.assert_(hashesConsistent(ht, [-11, -4], [-9, -6]))
        dgl.O_DecRef(ht)

    def testM02_Hashes_AtNodes(self):
        ht = newHT()
        dgl.Ht_Give(ht, makeMarkedHashKey(0, 0, 5))
        self.assert_(hashesConsistent(ht, [-1, 5], [0, 4]))
        dgl.O_DecRef(ht)

    def testM02n_Hashes_AtNodes(self):
        ht = newHT()
        dgl.Ht_Give(ht, makeMarkedHashKey(0, -10, -5))
        self.assert_(hashesConsistent(ht, [-11, -5], [-10, -6]))
        dgl.O_DecRef(ht)

    def testM03_Hashes_SameNode(self):
        ht = newHT()
        dgl.Ht_Give(ht, makeMarkedHashKey(0, 1, 5))
        dgl.Ht_Give(ht, makeMarkedHashKey(1, 5, 9))
        self.assert_(hashesConsistent(ht, [-1, 0, 9, 10], [1,2,3,4],[5,8]))
        dgl.O_DecRef(ht)

    def testM03n_Hashes_SameNode(self):
        ht = newHT()
        dgl.Ht_Give(ht, makeMarkedHashKey(0, -9, -5))
        dgl.Ht_Give(ht, makeMarkedHashKey(1, -5, -1))
        self.assert_(hashesConsistent(ht, [-11, -10, -1, 0], [-9,-8,-7,-6],[-5,-2]))
        dgl.O_DecRef(ht)

    def testM04_Hashes_Intersection(self):
        
        ht = newHT()

        dgl.Ht_Give(ht, makeMarkedHashKey(0, 0, 6))
        dgl.Ht_Give(ht, makeMarkedHashKey(1, 1, 3 ))

        self.assert_(hashesConsistent(ht, 
                                      [-1, 6,7,8], 
                                      [0,3,4,5],
                                      [1,2]))

        dgl.O_DecRef(ht)

    def testM05n_Hashes_MultipleRanges(self):
        
        ht = newHT()

        dgl.Ht_Give(ht, makeMarkedHashKey(0, -10, 0))
        dgl.Ht_Give(ht, makeMarkedHashKey(1, -9, -7, -6, -5, -3, -2))

        self.assert_(hashesConsistent(ht, 
                                      [-11,0,1],
                                      [-10,-7,-5,-2,-1], 
                                      [-9,-8,-6,-3]))
        dgl.O_DecRef(ht)


    def testM06a_HashesInvariantOutsideRange_Single(self):
        ht = newHT()
        
        h1 = makeMarkedHashKey(0, 1, 5)
        h2 = makeMarkedHashKey(1, 3, 7)

        dgl.Ht_Give(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6], [1,2,3,4]))

        s_h1 = getHashAtMarkerLoc(ht, 1)

        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))

        dgl.Ht_Give(ht, h2)

        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))

        s_h2 = getHashAtMarkerLoc(ht, 3)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2], [3,4], [5,6]))
        
        dgl.O_DecRef(ht)



    def consistencyTest(self, N, marker_range, n_keys, n_key_marker_ranges):

        r = marker_range

        def newDict():
            ht = newHT()

            random.seed(0)

            for k in range(n_keys):
                
                key_range_args = []
                
                for n in range(n_key_marker_ranges):
                    a = random.randint(*r)
                    b = random.randint(*r)
                    
                    key_range_args.append(min(a,b))
                    key_range_args.append(max(a,b) + 1)

                dgl.Ht_Give(ht, makeMarkedHashKey(k, *key_range_args))

            d = dict( (m, getHashAtMarkerLoc(ht,m))
                      for m in range(r[0]-1, r[1]+2) )

            dgl.O_DecRef(ht)

            return d
        
        d1 = newDict()

        for i in range(N):
            self.assert_(d1 == newDict())

        
    def testM06_Hashes_LargeConsistencyTest_Compact_Small(self):
        self.consistencyTest(200, [-3,3], 2, 3)

    def testM07_Hashes_LargeConsistencyTest_Compact_Large(self):
        self.consistencyTest(200, [-3,3], 50, 2)

    def testM08_Hashes_LargeConsistencyTest_Spread_Small(self):
        self.consistencyTest(200, [-50,50], 10, 2)

    def testM09_Hashes_LargeConsistencyTest_Spread_Large(self):
        self.consistencyTest(100, [-50,50], 50, 5)

    def testM10_Hashes_Value_Simple(self):
        ht = newHT()
        dgl.Ht_Give(ht, makeMarkedHashKey(0, 0, 5))

        self.assert_(getHashAtMarkerLoc(ht, 1) 
                     == extractHash(makeHashKey(0)))

        dgl.O_DecRef(ht)


    def testM20_Hashes_Deletion_01_Simple(self):
        ht = newHT()

        h = makeMarkedHashKey(0, 1, 5)
        dgl.Ht_Give(ht, h)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6], [1,2,3,4]))

        dgl.Ht_Clear(ht, h)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6, 1,2,3,4]))

        dgl.O_DecRef(ht)

    def testM20_Hashes_Deletion_02_Double(self):
        ht = newHT()
        
        h1 = makeMarkedHashKey(0, 1, 5)
        h2 = makeMarkedHashKey(1, 3, 7)

        dgl.Ht_Give(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6], [1,2,3,4]))

        s_h1 = getHashAtMarkerLoc(ht, 1)
        dgl.Ht_Give(ht, h2)
        s_h2 = getHashAtMarkerLoc(ht, 3)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2], [3,4], [5,6]))

        dgl.Ht_Clear(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 3) == extractHash(h2))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 1, 2, 7, 8], [3,4, 5,6]))

        dgl.Ht_Clear(ht, h2)

        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6, 1,2,3,4]))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))

        dgl.O_DecRef(ht)

    def testM20_Hashes_Deletion_03_Inside(self):
        ht = newHT()

        h1 = makeMarkedHashKey(0, 1, 7)
        h2 = makeMarkedHashKey(1, 3, 5)

        dgl.Ht_Give(ht, h1)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2,3,4,5,6]))

        s_h1 = getHashAtMarkerLoc(ht, 1)

        dgl.Ht_Give(ht, h2)
 
        dgl._Ht_debug_HashTableConsistent(ht)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2, 5, 6], [3,4]))

        dgl.Ht_Clear(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 3) == extractHash(h2))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 1, 2, 7, 8, 5,6], [3,4]))

        dgl.Ht_Clear(ht, h2)

        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6, 1,2,3,4]))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))

        dgl.O_DecRef(ht)


    def testM21_Hashes_Replacement(self):

        ht = newHT()        

        h1 = makeMarkedHashKey(0, 1, 7)
        h2 = makeMarkedHashKey(0, 3, 5)

        dgl.Ht_Give(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)
        self.assert_(getHashAtMarkerLoc(ht, 1) == extractHash(h1))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2,3,4,5,6]))

        dgl.Ht_Give(ht, h2)

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)
        self.assert_(getHashAtMarkerLoc(ht, 3) == extractHash(h1))
        self.assert_(hashesConsistent(ht, range(-1,3) + range(5,10), [3,4]))

        dgl.O_DecRef(ht)

    def testM22_Corner_01_MinusInf_01(self):

        ht = newHT()        
        
        h1 = makeMarkedHashKey(0, mr_minus_infinity, 0)

        dgl.Ht_Give(ht, h1)
        
        self.assert_(getHashAtMarkerLoc(ht, mr_minus_infinity) == extractHash(h1))
        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash, getHashAtMarkerLoc(ht, 0))

        dgl.O_DecRef(ht)

    def testM22_Corner_01_MinusInf_02_Give(self):

        ht = newHT() 
        
        h1 = makeHashKey(0)
        
        dgl.Ht_Give(ht, makeMarkedHashKey(0, mr_minus_infinity, -5))
        dgl.Ht_Give(ht, makeMarkedHashKey(0, mr_minus_infinity, 0))
        dgl.Ht_Give(ht, makeMarkedHashKey(0, mr_minus_infinity, 5))
       
        self.assert_(getHashAtMarkerLoc(ht, mr_minus_infinity) == extractHash(h1))
        self.assert_(getHashAtMarkerLoc(ht, 5) == null_hash, getHashAtMarkerLoc(ht, 0))

        dgl.O_DecRef(ht)

    def testM22_Corner_02_PlusInf_01(self):
        ht = newHT()
        
        h1 = makeMarkedHashKey(0, mr_minus_infinity, mr_plus_infinity)

        dgl.Ht_Give(ht, h1)

        self.assert_(hashesConsistent(ht, [mr_minus_infinity, 0], [mr_plus_infinity]))
        self.assert_(getHashAtMarkerLoc(ht, mr_plus_infinity) == null_hash)

    def testM22_Corner_02_PlusInf_02(self):
        ht = newHT()
        
        h1 = makeHashKey(0)

        dgl.Ht_Give(ht, h1)

        self.assert_(hashesConsistent(ht, [mr_minus_infinity, 0], [mr_plus_infinity]))
        self.assert_(getHashAtMarkerLoc(ht, mr_plus_infinity) == null_hash)

class TestHashTableSummarize(unittest.TestCase):

    ################################################################################
    # Test the hash along marker values

    def checkHAMV(self, htl, zero_point, *cons_sets):
        
        hmv = 0

        for i, ht in enumerate(htl):
            hmv = dgl.Ht_Summarize_Add(hmv, ht)

        hmv = dgl.Ht_Summarize_Finish(hmv)

        self.assert_(hashesConsistent(hmv, *cons_sets))

        self.assert_(getHashAtMarkerLoc(hmv, zero_point) == null_hash)

        decRef(hmv)

    def testHAMV01_Single(self):
        ht = newHT()
        h1 = exactHashKey("01000000000000000000000000000000")

        dgl.Ht_Give(ht, addMarkerInfo(h1, 2,4))
        #dgl.Ht_debug_print(ht)
        #dgl.Ht_MSL_debug_Print(ht)
        self.checkHAMV([ht], 0, [0,1,4,5], [2,3])
        decRef(ht)

    def testHAMV02_Double(self):
        ht = newHT()
        dgl.Ht_Give(ht, addMarkerInfo(exactHashKey("01000000000000000000000000000000"), 2, 6))
        dgl.Ht_Give(ht, addMarkerInfo(exactHashKey("02000000000000000000000000000000"), 4, 8))
        self.checkHAMV([ht], 0, [0,1,8,9], [2,3], [4,5], [6,7])
        decRef(ht)

    def testHAMV03_Double_Sandwich(self):
        ht = newHT()
        
        h1 = exactHashKey("01000000000000000000000000000000")
        h2 = exactHashKey("02000000000000000000000000000000")

        dgl.Ht_Give(ht, addMarkerInfo(h1, 2, 8))
        dgl.Ht_Give(ht, addMarkerInfo(h2, 4, 6))
        self.checkHAMV([ht], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht)

    def testHAMV04_2Table_Double(self):
        h1 = exactHashKey("01000000000000000000000000000000")
        h2 = exactHashKey("02000000000000000000000000000000")

        ht1 = newHT()
        dgl.Ht_Give(ht1, addMarkerInfo(h1, 2, 6))

        ht2 = newHT()
        dgl.Ht_Give(ht2, addMarkerInfo(h2, 4, 8))

        self.checkHAMV([ht1, ht2], 0, [0,1,8,9], [2,3], [4,5], [6,7])
        decRef(ht1,ht2)

    def testHAMV05_2Table_Double_Sandwich(self):
        ht1 = newHT()
        dgl.Ht_Give(ht1, makeMarkedHashKey(0, 2, 8))

        ht2 = newHT()
        dgl.Ht_Give(ht2, makeMarkedHashKey(1, 4, 6))

        self.checkHAMV([ht1,ht2], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht1,ht2)

    def testHAMV06_2Table_Double_Duplicate(self):
        ht1 = newHT()
        dgl.Ht_Give(ht1, makeMarkedHashKey(0, 2, 6))

        ht2 = newHT()
        dgl.Ht_Give(ht2, makeMarkedHashKey(0, 4, 8))

        self.checkHAMV([ht1, ht2], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht1,ht2)

    def testHAMV07_2Table_Double_Sandwich_Duplicate(self):
        ht1 = newHT()
        dgl.Ht_Give(ht1, makeMarkedHashKey(0, 2, 8))

        ht2 = newHT()
        dgl.Ht_Give(ht2, makeMarkedHashKey(0, 4, 6))

        self.checkHAMV([ht1,ht2], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht1,ht2)

    def testHAMV08_50Table_Unique_Keys(self):
        
        distinct_values = []

        def new_Table(i):
            ht = newHT()
            dgl.Ht_Give(ht, makeMarkedHashKey(i, 2*i, 2*i+2))
            distinct_values.append([2*i, 2*i +1])

            return ht

        htl = [new_Table(i) for i in xrange(50)]

        self.checkHAMV(htl, -1, *distinct_values)
        decRef(*htl)

    def testHAMV09_50Table_Same_Keys(self):
        
        distinct_values = []

        def new_Table(i):
            ht = newHT()
            dgl.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*i+2))
            distinct_values.append(2*i)
            distinct_values.append(2*i+1)

            return ht

        htl = [new_Table(i) for i in xrange(50)]

        self.checkHAMV(htl, -1, [-1], distinct_values)
        decRef(*htl)

    def testHAMV10_3Table_Nested_Identical(self):
        
        N = 3

        distinct_values = []

        def new_Table(i):
            ht = newHT()
            dgl.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*(2*N - i)))
            distinct_values.append([2*i, 2*i+1, 2*(2*N - i) - 2, 2*(2*N - i) - 1])

            return ht

        htl = [new_Table(i) for i in xrange(N)]

        self.checkHAMV(htl, -1, [-1], *distinct_values)
        decRef(*htl)

    def testHAMV10_5Table_Nested_Identical(self):

        return # KNOWN FAIL

        # Reason is that the XOR operations cancel out, allowing
        # multiple repeat keys under this circumstance.  Solution is
        # to implement a proper merging operation based on the +
        # operator.
        
        N = 5  

        distinct_values = []

        def new_Table(i):
            ht = newHT()
            dgl.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*(2*N - i)))
            distinct_values.append([2*i, 2*i+1, 2*(2*N - i) - 2, 2*(2*N - i) - 1])

            return ht

        htl = [new_Table(i) for i in xrange(N)]

        self.checkHAMV(htl, -1, [-1], *distinct_values)
        decRef(*htl)


    def testHAMV11_100Table_Nested_Identical(self):
        
        return  # KNOWN FAIL

        N = 100

        distinct_values = []

        def new_Table(i):
            ht = newHT()
            dgl.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*(2*N - i)))
            distinct_values.append([2*i, 2*i+1, 2*(2*N - i) - 2, 2*(2*N - i) - 1])

            return ht

        htl = [new_Table(i) for i in xrange(N)]

        self.checkHAMV(htl, -1, [-1], *distinct_values)
        decRef(*htl)




if __name__ == '__main__':
    unittest.main()


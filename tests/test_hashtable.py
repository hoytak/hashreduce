#!/usr/bin/env python
import unittest, random
from common import *
from pprint import *
from itertools import product

ibd.Mr_Plus_Infinity.restype = c_long
ibd.Mr_Minus_Infinity.restype = c_long

mr_plus_infinity = ibd.Mr_Plus_Infinity()
mr_minus_infinity = ibd.Mr_Minus_Infinity()

ibd.Hti_New.restype = ctypes.c_void_p
ibd.Htib_New.restype = ctypes.c_void_p
ibd.Ht_Summarize_Update.restype = ctypes.c_void_p
ibd.Ht_Summarize_Finish.restype = ctypes.c_void_p
ibd.Ht_Get.restype = ctypes.c_void_p
ibd.Ht_View.restype = ctypes.c_void_p
ibd.Ht_HashAtMarkerPoint.restype = ctypes.c_void_p
ibd.Hti_Next.restype = ctypes.c_void_p

def isNull(t):
    return t is None or t == 0

def getHashTreeSet(ht):
    
    hti = ibd.Hti_New(ht)
    
    s = set()

    hk = c_void_p()

    while True:
        okay = ibd.Hti_Next(byref(hk), hti)
        
        if not okay:
            break
        else:
            h = extractHash(hk)
            assert h not in s
            s.add(h)

    
    htib = ibd.Htib_New(ht)
    
    sb = set()

    while True:
        okay = ibd.Htib_Next(byref(hk), htib)
        
        if not okay:
            break
        else:
            h = extractHash(hk)
            assert h not in sb 
            sb.add(h)

    assert s == sb 

    return s

newHT = ibd.NewHashTable
newHT.restype = ctypes.c_void_p

def getHashAtMarkerLoc(ht, m):
    hk = ibd.Ht_HashAtMarkerPoint(0, ht, c_long(m))
    assert ibd.O_RefCount(hk) == 1 
    
    s = extractHash(hk)
    assert ibd.O_RefCount(hk) == 1 

    ibd.O_DecRef(hk)
    
    return s

def getHashOfMarkerRange(ht, m1, m2):
    hk = ibd.Ht_HashOfMarkerRange(0, ht, c_long(m1), c_long(m2))
    assert ibd.O_RefCount(hk) == 1 
    
    s = extractHash(hk)
    assert ibd.O_RefCount(hk) == 1 

    ibd.O_DecRef(hk)
    
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
    
def getHTValiditySet(ht, rl):
    
    hti = ibd.Hti_New(ht)
    
    s = set()

    hk = c_void_p()

    while True:
        okay = ibd.Hti_Next(byref(hk), hti)
        
        if not okay:
            break
        else:
            h = extractHash(hk)
            assert h not in s

            for r in rl:
                if ibd.H_MarkerPointIsValid(hk, r):
                    s.add( (h, r) )
    
    return s                

class TestHashTableBasic(unittest.TestCase):
    """
    Just a few hash keys tests that are more sanity checks then full
    tests.  
    """

    def test01a_CreateDelete(self):
        ht = newHT()
        self.assert_(ibd.O_RefCount(ht) == 1, "refcount = %d" % ibd.O_RefCount(ht) )
        ibd.O_DecRef(ht)

    def test01b_SimpleRetrieval(self):
        ht = newHT()
        self.assert_(ibd.O_RefCount(ht) == 1)
        hk = makeHashKey(0)

        ibd.Ht_Give(ht, hk)
        
        self.assert_(ibd.Ht_View(ht, hk) == hk)

        ibd.O_DecRef(ht)

    def test02_Sizes(self):
        ht = newHT()
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)

        self.assert_(ibd.Ht_Size(ht) == 0)

        ibd.Ht_Give(ht, hk1)

        self.assert_(ibd.Ht_Size(ht) == 1)

        ibd.Ht_Give(ht, hk2)

        self.assert_(ibd.Ht_Size(ht) == 2)

        ibd.Ht_Pop(ht, hk1)

        self.assert_(ibd.Ht_Size(ht) == 1)
        
        ibd.Ht_Pop(ht, hk2)

        self.assert_(ibd.Ht_Size(ht) == 0)

        ibd.O_DecRef(ht)

    def test03_RetrieveOriginal(self):

        ht = newHT()
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(0)

        self.assert_(hk1 != hk2)

        ibd.Ht_Give(ht, hk1)

        self.assert_(ibd.Ht_View(ht, hk2) == hk1)

        ibd.O_DecRef(ht)

    def test04_SimpleIterator(self):
        ht = newHT()
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)

        ibd.Ht_Give(ht, hk1)
        ibd.Ht_Give(ht, hk2)

        strue = set([extractHash(hk1), extractHash(hk2)])

        s = getHashTreeSet(ht)

        self.assert_(s == strue, '%s != %s (true)' % (str(s), str(strue)))

        ibd.O_DecRef(ht)
        
    def checkHkList(self, hashkeys, null_hash_keys = []):

        ht = newHT()
        
        hashes = [extractHash(hk) for hk in hashkeys]

        count = 0
        
        for i, hk in enumerate(hashkeys):
            for hk2 in hashkeys[i:]:
                self.assert_(not ibd.Ht_Contains(ht, hk2))
                self.assert_(ibd.Ht_Clear(ht, hk2) == 0)

            self.assert_(ibd.Ht_Size(ht) == count, 
                         "%d (given) != %d (count)" % (ibd.Ht_Size(ht), count))

            ibd.Ht_Give(ht, hk)
            
            count += 1
            
            self.assert_(ibd.Ht_Size(ht) == count, 
                         "%d (given) != %d (count)" % (ibd.Ht_Size(ht), count))

        for hk in hashkeys:
            self.assert_(ibd.Ht_View(ht, hk) == hk)

            self.assert_(ibd.Ht_Size(ht) == count)

            for hk2 in null_hash_keys:
                self.assert_(ibd.Ht_Clear(ht, hk2) == 0)
                self.assert_(ibd.Ht_Size(ht) == count, 
                             "%d (given) != %d (count)" % (ibd.Ht_Size(ht), count))


        s = getHashTreeSet(ht)

        self.assert_(s == set(hashes))

        for hk in hashkeys:
            ibd.Ht_Clear(ht, hk)

            count -= 1
            
            self.assert_(ibd.Ht_Size(ht) == count)

            for hk2 in null_hash_keys:
                self.assert_(ibd.Ht_Clear(ht, hk2) == 0)
                self.assert_(ibd.Ht_Size(ht) == count, 
                             "%d (given) != %d (count)" % (ibd.Ht_Size(ht), count))


        ibd.O_DecRef(ht)

        
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

    def test05_Corner_01_Close_4(self):
        self.checkHkList(
            [exactHashKey("0000000000000000000000000000000%s" % c)
             for c in "0123456789abcdef"])

    def test05_Corner_01_Close_5(self):
        self.checkHkList(
            [exactHashKey("000000000000000000000000000000%s%s" % (c1, c2))
             for c1, c2 in product("0123456789abcdef", "0123456789abcdef")])

    def test05_Corner_02_2LevelClose_1(self):
        self.checkHkList(
            [exactHashKey("01000000000000000000000000000000"),
             exactHashKey("02000000000000000000000000000000")])

    def test05_Corner_02_2LevelClose_1_regression(self):
        self.checkHkList(
            [addMarkerInfo(exactHashKey("01000000000000000000000000000000"), 2,8),
             addMarkerInfo(exactHashKey("02000000000000000000000000000000"), 4,6)])

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

    def test05_Corner_03_1LevelClose_4(self):
        self.checkHkList(
            [exactHashKey("%s0000000000000000000000000000000" % c)
             for c in "0123456789abcdef"])

    def test05_Corner_03_1LevelClose_5(self):
        self.checkHkList(
            [exactHashKey("%s%s000000000000000000000000000000" % (c1, c2))
             for c1, c2 in product("0123456789abcdef", "0123456789abcdef")])

    def test10_LargeContainmentTest(self):

        n = 100
        hashkeys = [makeHashKey(i) for i in range(n)]

        self.checkHkList(hashkeys)

    def test13_Regression_HashSequence(self):
        
        ht = newHT()
        hkl = [makeHashKey("n7"),
               makeHashKey("n-3"),
               makeHashKey("n5"),
               makeHashKey("n9"),
               makeHashKey("n-1"),
               makeHashKey("n-5")]

        for hk in hkl:
            ibd.Ht_Give(ht, hk)

        self.assert_(isNull(ibd.Ht_View(ht, makeHashKey("n-4"))))

        ibd.O_DecRef(ht)

    def test14_BadClearIneffective(self):

        ht = newHT()

        ibd.Ht_Give(ht, makeHashKey(0))
        ibd.Ht_Give(ht, makeHashKey(1))
        ibd.Ht_Give(ht, makeHashKey(2))

        self.assert_(ibd.Ht_Size(ht) == 3)

        h = makeHashKey(2)
        self.assert_(ibd.Ht_Clear(ht,h))
        ibd.O_DecRef(h)

        self.assert_(ibd.Ht_Size(ht) == 2)

        h = makeHashKey(10)
        self.assert_(not ibd.Ht_Clear(ht, h))
        ibd.O_DecRef(h)

        self.assert_(ibd.Ht_Size(ht) == 2)

        ibd.O_DecRef(ht)

    def test15_Corner_Regression_PopOnEmpty(self):

        ht = newHT()

        self.assert_(ibd.Ht_Size(ht) == 0)

        h = makeHashKey(0)
        self.assert_(not ibd.Ht_Clear(ht, h))
        ibd.O_DecRef(h)

        self.assert_(ibd.Ht_Size(ht) == 0)

        ibd.O_DecRef(ht)

################################################################################
# Reference Counting / Locking stuff

    def testR01_Set(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        
        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)
        
    def testR02_Clear(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.Ht_Clear(ht, h)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(ht)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)

    def testR03_Give(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)
        ibd.O_IncRef(h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))
        
        ibd.Ht_Give(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)
        
    def testR04_Pop(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        
        ibd.Ht_Give(ht, h)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.Ht_Pop(ht, h)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)

    def testR05_SetDefault(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        
        ibd.Ht_SetDefault(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)

    def testR06_Replace(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        h2 = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h2) == 1, ibd.O_RefCount(h2))
        
        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.Ht_Set(ht, h2)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        self.assert_(ibd.O_RefCount(h2) == 2, ibd.O_RefCount(h2))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        self.assert_(ibd.O_RefCount(h2) == 1, ibd.O_RefCount(h2))

        ibd.O_DecRef(h)
        ibd.O_DecRef(h2)

    def testR07_ReplaceDefault(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        h2 = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h2) == 1, ibd.O_RefCount(h2))
        
        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.Ht_SetDefault(ht, h2)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        self.assert_(ibd.O_RefCount(h2) == 1, ibd.O_RefCount(h2))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        self.assert_(ibd.O_RefCount(h2) == 1, ibd.O_RefCount(h2))

        ibd.O_DecRef(h)
        ibd.O_DecRef(h2)

    def testR08_ReplaceWithSelf(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        
        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)

    def testR09_ReplaceWithSelfPassive(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        
        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.Ht_SetDefault(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)

    def testR10_GiveReplaceDefault(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)
        ibd.O_IncRef(h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        h2 = makeHashKey(0)
        ibd.O_IncRef(h2)

        self.assert_(ibd.O_RefCount(h2) == 2, ibd.O_RefCount(h2))
        ibd.Ht_Give(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.Ht_Give(ht, h2)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        self.assert_(ibd.O_RefCount(h2) == 2, ibd.O_RefCount(h2))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        self.assert_(ibd.O_RefCount(h2) == 1, ibd.O_RefCount(h2))

        ibd.O_DecRef(h)
        ibd.O_DecRef(h2)

    def testR11_GiveReplaceWithSelf(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)
        ibd.O_IncRef(h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))
        
        ibd.Ht_Give(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.Ht_Set(ht, h)

        self.assert_(ibd.O_RefCount(h) == 2, ibd.O_RefCount(h))

        ibd.O_DecRef(ht)
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)

    def testR12_PopAltQuery(self):
        ht = newHT()

        self.assert_(ibd.O_RefCount(ht) == 1, ibd.O_RefCount(ht))

        h = makeHashKey(0)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))
        
        ibd.Ht_Give(ht, h)

        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.Ht_Pop(ht, makeHashKey(0))
        
        self.assert_(ibd.O_RefCount(h) == 1, ibd.O_RefCount(h))

        ibd.O_DecRef(h)


    def testR13_OverwriteSameWithGive(self):
        ht = newHT()
        h1 = makeHashKey(1)
        h2 = makeHashKey(2)
        ibd.Ht_Set(ht, h1)
        ibd.Ht_Set(ht, h2)
        
        self.assert_(ibd.O_RefCount(h1) == 2)
        self.assert_(ibd.O_RefCount(h2) == 2)

        ibd.Ht_Give(ht, h1)

        self.assert_(ibd.O_RefCount(h1) == 1)
        self.assert_(ibd.O_RefCount(h2) == 2)

        ibd.Ht_Give(ht, h2)

        self.assert_(ibd.O_RefCount(h1) == 1)
        self.assert_(ibd.O_RefCount(h2) == 1)

        decRef(ht)

    def testR15_AddMarkerValidRange_01(self):

        ht = newHT()
        h = makeMarkedHashKey(1, 2, 4)
        
        ibd.Ht_Set(ht, h)

        self.assert_(getHashAtMarkerLoc(ht, 1) == null_hash)
        hashesConsistent(ht, [0,1,4,5,6,7,8,9,10], [2,3])

        ibd.Ht_InsertValidRange(ht, h, 6, 8)

        self.assert_(getHashAtMarkerLoc(ht, 1) == null_hash)
        hashesConsistent(ht, [0,1,4,5,8,9,10], [2,3,6,7])

        decRef(ht,h);

    def testR15_AddMarkerValidRange_02(self):

        ht = newHT()
        h = makeMarkedHashKey(1, 0, 0)
        
        ibd.Ht_InsertValidRange(ht, h, 2, 4)

        self.assert_(getHashAtMarkerLoc(ht, 1) == null_hash)
        hashesConsistent(ht, [0,1,4,5,6,7,8,9,10], [2,3])

        ibd.Ht_InsertValidRange(ht, h, 6, 8)

        self.assert_(getHashAtMarkerLoc(ht, 1) == null_hash)
        hashesConsistent(ht, [0,1,4,5,8,9,10], [2,3,6,7])

        decRef(ht,h);
        
    def testR15_AddMarkerValidRange_03(self):

        ht = newHT()
        h = makeMarkedHashKey(1, 0, 0)
        hk = makeHashKey(1)        
        
        ibd.Ht_InsertValidRange(ht, h, 2, 4)

        self.assert_(ibd.Ht_View(ht, hk) == h)

        self.assert_(getHashAtMarkerLoc(ht, 1) == null_hash)
        hashesConsistent(ht, [0,1,4,5,6,7,8,9,10], [2,3])

        ibd.Ht_InsertValidRange(ht, h, 6, 8)

        self.assert_(getHashAtMarkerLoc(ht, 1) == null_hash)
        hashesConsistent(ht, [0,1,4,5,8,9,10], [2,3,6,7])
        
        decRef(ht,h,hk);

    def testRR01_InsertAndClear(self):

        ht = newHT()
        s1 = getHashOfMarkerRange(ht, 0, 10)

        hk = makeMarkedHashKey(0, -5, 5)
        ibd.Ht_Give(ht, hk)
        
        self.assert_(s1 != getHashOfMarkerRange(ht, 0, 10))

        ibd.Ht_Clear(ht, hk)

        self.assert_(s1 == getHashOfMarkerRange(ht, 0, 10))

        decRef(ht);

    def testRR02_OutsideRangeIgnored_01(self):

        ht1 = newHT()
        
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 0, 5))
        s1 = getHashOfMarkerRange(ht1, 0, 10)

        ht2 = newHT()

        ibd.Ht_Give(ht2, makeMarkedHashKey(0, -5, 5))
        
        self.assert_(s1 == getHashOfMarkerRange(ht2, 0, 10))

        decRef(ht1,ht2);

    def testRR02_OutsideRangeIgnored_02(self):

        ht1 = newHT()
        
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 0, 10))
        s1 = getHashOfMarkerRange(ht1, 0, 10)

        ht2 = newHT()

        ibd.Ht_Give(ht2, makeMarkedHashKey(0, -5, 15))
        
        self.assert_(s1 == getHashOfMarkerRange(ht2, 0, 10))

        decRef(ht1,ht2);

    def testRR02_OutsideRangeIgnored_03(self):

        ht1 = newHT()
        
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 5, 10))
        s1 = getHashOfMarkerRange(ht1, 0, 10)

        ht2 = newHT()

        ibd.Ht_Give(ht2, makeMarkedHashKey(0, 5, 15))
        
        self.assert_(s1 == getHashOfMarkerRange(ht2, 0, 10))

        decRef(ht1,ht2);

    def testRR02_OutsideRangeIgnored_04(self):

        ht1 = newHT()
        
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 0, 5))
        s1 = getHashOfMarkerRange(ht1, 0, 10)

        ht2 = newHT()

        ibd.Ht_Give(ht2, makeMarkedHashKey(0, 0, 5))
        ibd.Ht_Give(ht2, makeMarkedHashKey(1, -5, 0))
        
        self.assert_(s1 == getHashOfMarkerRange(ht2, 0, 10))

        decRef(ht1,ht2);

    def testRR02_OutsideRangeIgnored_05(self):

        ht1 = newHT()
        
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 0, 5))
        s1 = getHashOfMarkerRange(ht1, 0, 10)

        ht2 = newHT()

        ibd.Ht_Give(ht2, makeMarkedHashKey(0, 0, 5))
        ibd.Ht_Give(ht2, makeMarkedHashKey(1, 10, 15))
        
        self.assert_(s1 == getHashOfMarkerRange(ht2, 0, 10))

        decRef(ht1,ht2);

################################################################################
# Tests about the marker stuff.
class TestHashTableMarkedKeys(unittest.TestCase):
    def testM_Basic_01_Existance(self):
        
        ht = newHT()
        hk = makeMarkedHashKey(0, 0, 5)
        
        ibd.Ht_Give(ht, hk)

        self.assert_(ibd.Ht_Contains(ht, hk))

        self.assert_(not ibd.Ht_ContainsAt(ht, hk, -1))
        self.assert_(ibd.Ht_ContainsAt(ht, hk, 0))
        self.assert_(not ibd.Ht_ContainsAt(ht, hk, 5))
        self.assert_(not ibd.Ht_ContainsAt(ht, hk, 6))

        ibd.O_DecRef(ht)

    def testM01_Hashes_Simple(self):
        ht = newHT()
        h = makeMarkedHashKey(0, 0, 5)
        ibd.Ht_Give(ht, h)

        self.assert_(hashesConsistent(ht, [-1, 6], [1, 4]))
        ibd.O_DecRef(ht)

    def testM01n_Hashes_Simple(self):
        ht = newHT()
        ibd.Ht_Give(ht, makeMarkedHashKey(0, -10, -5))
        self.assert_(hashesConsistent(ht, [-11, -4], [-9, -6]))
        ibd.O_DecRef(ht)

    def testM02_Hashes_AtNodes(self):
        ht = newHT()
        ibd.Ht_Give(ht, makeMarkedHashKey(0, 0, 5))
        self.assert_(hashesConsistent(ht, [-1, 5], [0, 4]))
        ibd.O_DecRef(ht)

    def testM02n_Hashes_AtNodes(self):
        ht = newHT()
        ibd.Ht_Give(ht, makeMarkedHashKey(0, -10, -5))
        self.assert_(hashesConsistent(ht, [-11, -5], [-10, -6]))
        ibd.O_DecRef(ht)

    def testM03_Hashes_SameNode(self):
        ht = newHT()
        ibd.Ht_Give(ht, makeMarkedHashKey(0, 1, 5))
        ibd.Ht_Give(ht, makeMarkedHashKey(1, 5, 9))
        self.assert_(hashesConsistent(ht, [-1, 0, 9, 10], [1,2,3,4],[5,8]))
        ibd.O_DecRef(ht)

    def testM03n_Hashes_SameNode(self):
        ht = newHT()
        ibd.Ht_Give(ht, makeMarkedHashKey(0, -9, -5))
        ibd.Ht_Give(ht, makeMarkedHashKey(1, -5, -1))
        self.assert_(hashesConsistent(ht, [-11, -10, -1, 0], [-9,-8,-7,-6],[-5,-2]))
        ibd.O_DecRef(ht)

    def testM04_Hashes_Intersection(self):
        
        ht = newHT()

        ibd.Ht_Give(ht, makeMarkedHashKey(0, 0, 6))
        ibd.Ht_Give(ht, makeMarkedHashKey(1, 1, 3 ))

        self.assert_(hashesConsistent(ht, 
                                      [-1, 6,7,8], 
                                      [0,3,4,5],
                                      [1,2]))

        ibd.O_DecRef(ht)

    def testM05n_Hashes_MultipleRanges(self):
        
        ht = newHT()

        ibd.Ht_Give(ht, makeMarkedHashKey(0, -10, 0))
        ibd.Ht_Give(ht, makeMarkedHashKey(1, -9, -7, -6, -5, -3, -2))

        self.assert_(hashesConsistent(ht, 
                                      [-11,0,1],
                                      [-10,-7,-5,-2,-1], 
                                      [-9,-8,-6,-3]))
        ibd.O_DecRef(ht)


    def testM06a_HashesInvariantOutsideRange_Single(self):
        ht = newHT()
        
        h1 = makeMarkedHashKey(0, 1, 5)
        h2 = makeMarkedHashKey(1, 3, 7)

        ibd.Ht_Give(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6], [1,2,3,4]))

        s_h1 = getHashAtMarkerLoc(ht, 1)

        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))

        ibd.Ht_Give(ht, h2)

        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))

        s_h2 = getHashAtMarkerLoc(ht, 3)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2], [3,4], [5,6]))
        
        ibd.O_DecRef(ht)


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

                ibd.Ht_Give(ht, makeMarkedHashKey(k, *key_range_args))

            d = dict( (m, getHashAtMarkerLoc(ht,m))
                      for m in range(r[0]-1, r[1]+2) )

            ibd.O_DecRef(ht)

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
        ibd.Ht_Give(ht, makeMarkedHashKey(0, 0, 5))

        self.assert_(getHashAtMarkerLoc(ht, 1) 
                     == extractHash(makeHashKey(0)))

        ibd.O_DecRef(ht)


    def testM20_Hashes_Deletion_01_Simple(self):
        ht = newHT()

        h = makeMarkedHashKey(0, 1, 5)
        ibd.Ht_Give(ht, h)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6], [1,2,3,4]))

        ibd.Ht_Clear(ht, h)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6, 1,2,3,4]))

        ibd.O_DecRef(ht)

    def testM20_Hashes_Deletion_02_Double(self):
        ht = newHT()
        
        h1 = makeMarkedHashKey(0, 1, 5)
        h2 = makeMarkedHashKey(1, 3, 7)

        ibd.Ht_Give(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)
        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6], [1,2,3,4]))

        s_h1 = getHashAtMarkerLoc(ht, 1)
        ibd.Ht_Give(ht, h2)
        s_h2 = getHashAtMarkerLoc(ht, 3)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2], [3,4], [5,6]))

        ibd.Ht_Clear(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 3) == extractHash(h2))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 1, 2, 7, 8], [3,4, 5,6]))

        ibd.Ht_Clear(ht, h2)

        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6, 1,2,3,4]))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))

        ibd.O_DecRef(ht)

    def testM20_Hashes_Deletion_03_Inside(self):
        ht = newHT()

        h1 = makeMarkedHashKey(0, 1, 7)
        h2 = makeMarkedHashKey(1, 3, 5)

        ibd.Ht_Give(ht, h1)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2,3,4,5,6]))

        s_h1 = getHashAtMarkerLoc(ht, 1)

        ibd.Ht_Give(ht, h2)
 
        ibd._Ht_debug_HashTableConsistent(ht)

        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))
        self.assert_(s_h1 == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2, 5, 6], [3,4]))

        ibd.Ht_Clear(ht, h1)

        self.assert_(getHashAtMarkerLoc(ht, 3) == extractHash(h2))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 1))
        self.assert_(hashesConsistent(ht, [-1, 0, 1, 2, 7, 8, 5,6], [3,4]))

        ibd.Ht_Clear(ht, h2)

        self.assert_(hashesConsistent(ht, [-1, 0, 5, 6, 1,2,3,4]))
        self.assert_(null_hash == getHashAtMarkerLoc(ht, 0))

        ibd.O_DecRef(ht)


    def testM21_Hashes_Replacement(self):

        ht = newHT()        
        assert ibd.O_RefCount(ht) == 1

        h1 = makeMarkedHashKey(0, 1, 7)
        assert ibd.O_RefCount(h1) == 1
        h2 = makeMarkedHashKey(0, 3, 5)
        assert ibd.O_RefCount(h2) == 1

        ibd.Ht_Give(ht, h1)

        assert ibd.O_RefCount(ht) == 1
        assert ibd.O_RefCount(h1) == 1
        assert ibd.O_RefCount(h2) == 1

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)

        assert ibd.O_RefCount(ht) == 1
        assert ibd.O_RefCount(h1) == 1
        assert ibd.O_RefCount(h2) == 1

        self.assert_(getHashAtMarkerLoc(ht, 1) == extractHash(h1))
#        print "HERE"

        assert ibd.O_RefCount(ht) == 1
        assert ibd.O_RefCount(h1) == 1
        assert ibd.O_RefCount(h2) == 1
        
        self.assert_(hashesConsistent(ht, [-1, 0, 7, 8], [1,2,3,4,5,6]))

        assert ibd.O_RefCount(ht) == 1
        assert ibd.O_RefCount(h1) == 1
        assert ibd.O_RefCount(h2) == 1

#        print "HERE 2"

        ibd.Ht_Give(ht, h2)

        assert ibd.O_RefCount(ht) == 1
        assert ibd.O_RefCount(h2) == 1

#        print "HERE 3"

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)

#        print "HERE 4"

        self.assert_(getHashAtMarkerLoc(ht, 3) == extractHash(h2))

#        print "HERE 5"

        self.assert_(hashesConsistent(ht, range(-1,3) + range(5,10), [3,4]))

#        print "HERE 6"

        ibd.O_DecRef(ht)

    def testM22_Corner_01_MinusInf_01(self):

        ht = newHT()        
        
        h1 = makeMarkedHashKey(0, mr_minus_infinity, 0)

        ibd.Ht_Give(ht, h1)
        
        self.assert_(getHashAtMarkerLoc(ht, mr_minus_infinity) == extractHash(h1))
        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash, getHashAtMarkerLoc(ht, 0))

        ibd.O_DecRef(ht)

    def testM22_Corner_01_MinusInf_02_Give(self):

        ht = newHT() 
        
        h1 = makeHashKey(0)
        
        ibd.Ht_Give(ht, makeMarkedHashKey(0, mr_minus_infinity, -5))
        ibd.Ht_Give(ht, makeMarkedHashKey(0, mr_minus_infinity, 0))
        ibd.Ht_Give(ht, makeMarkedHashKey(0, mr_minus_infinity, 5))
       
        self.assert_(getHashAtMarkerLoc(ht, mr_minus_infinity) == extractHash(h1))
        self.assert_(getHashAtMarkerLoc(ht, 5) == null_hash, getHashAtMarkerLoc(ht, 0))

        ibd.O_DecRef(ht)

    def testM22_Corner_02_PlusInf_01(self):
        ht = newHT()
        
        h1 = makeMarkedHashKey(0, mr_minus_infinity, mr_plus_infinity)

        ibd.Ht_Give(ht, h1)

        self.assert_(hashesConsistent(ht, [mr_minus_infinity, 0], [mr_plus_infinity]))
        self.assert_(getHashAtMarkerLoc(ht, mr_plus_infinity) == null_hash)

    def testM22_Corner_02_PlusInf_02(self):
        ht = newHT()
        
        h1 = makeHashKey(0)

        ibd.Ht_Give(ht, h1)

        self.assert_(hashesConsistent(ht, [mr_minus_infinity, 0], [mr_plus_infinity]))
        self.assert_(getHashAtMarkerLoc(ht, mr_plus_infinity) == null_hash)


class TestHashTableSummarize(unittest.TestCase):

    def checkReduce(self, ht, r):
        htr = ibd.Ht_ReduceTable(ht)

        for m in r:
            self.assert_(getHashAtMarkerLoc(htr, m) == getHashAtMarkerLoc(ht, m))
        

    def test01_Basic(self):

        ht = newHT()

        h = makeMarkedHashKey(0, 2,4)

        ibd.Ht_Give(ht, h)

        htr = ibd.Ht_ReduceTable(ht)

        self.assert_(getHashAtMarkerLoc(ht, 0) == null_hash)
        self.assert_(getHashAtMarkerLoc(ht, 2) == extractHash(h))
        self.assert_(getHashAtMarkerLoc(ht, 4) == null_hash)
        
        self.assert_(getHashAtMarkerLoc(htr, 0) == null_hash)
        self.assert_(getHashAtMarkerLoc(htr, 2) == extractHash(h))
        self.assert_(getHashAtMarkerLoc(htr, 4) == null_hash)


    ################################################################################
    # Test the hash along marker values

    def checkHAMV(self, htl, zero_point, *cons_sets):
        
        hmv = 0

        for i, ht in enumerate(htl):
            hmv = ibd.Ht_Summarize_Update(hmv, ht)

        hmv = ibd.Ht_Summarize_Finish(hmv)

        self.assert_(hashesConsistent(hmv, *cons_sets))

        self.assert_(getHashAtMarkerLoc(hmv, zero_point) == null_hash)
        
        decRef(hmv)

    def testHAMV01_Single(self):
        ht = newHT()
        h1 = exactHashKey("01000000000000000000000000000000")

        ibd.Ht_Give(ht, addMarkerInfo(h1, 2,4))
        #ibd.Ht_debug_print(ht)
        #ibd.Ht_MSL_debug_Print(ht)
        self.checkHAMV([ht], 0, [0,1,4,5], [2,3])
        decRef(ht)

    def testHAMV02_Double(self):
        ht = newHT()
        ibd.Ht_Give(ht, addMarkerInfo(exactHashKey("01000000000000000000000000000000"), 2, 6))
        ibd.Ht_Give(ht, addMarkerInfo(exactHashKey("02000000000000000000000000000000"), 4, 8))
        self.checkHAMV([ht], 0, [0,1,8,9], [2,3], [4,5], [6,7])
        decRef(ht)

    def testHAMV03_Double_Sandwich(self):
        ht = newHT()
        
        h1 = exactHashKey("01000000000000000000000000000000")
        h2 = exactHashKey("02000000000000000000000000000000")

        ibd.Ht_Give(ht, addMarkerInfo(h1, 2, 8))
        ibd.Ht_Give(ht, addMarkerInfo(h2, 4, 6))

        self.assert_(getHashAtMarkerLoc(ht, 2) == extractHash(h1))
        self.assert_(getHashAtMarkerLoc(ht, 7) == extractHash(h1))
        self.assert_(getHashAtMarkerLoc(ht, 4) == "03000000000000000000000000000000")
        
        self.checkHAMV([ht], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht)

    def testHAMV04_2Table_Double(self):
        h1 = exactHashKey("01000000000000000000000000000000")
        h2 = exactHashKey("02000000000000000000000000000000")

        ht1 = newHT()
        ibd.Ht_Give(ht1, addMarkerInfo(h1, 2, 6))

        ht2 = newHT()
        ibd.Ht_Give(ht2, addMarkerInfo(h2, 4, 8))

        self.checkHAMV([ht1, ht2], 0, [0,1,8,9], [2,3], [4,5], [6,7])
        decRef(ht1,ht2)

    def testHAMV05_2Table_Double_Sandwich(self):
        ht1 = newHT()
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 2, 8))

        ht2 = newHT()
        ibd.Ht_Give(ht2, makeMarkedHashKey(1, 4, 6))

        self.checkHAMV([ht1,ht2], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht1,ht2)

    def testHAMV06_2Table_Double_Duplicate(self):
        ht1 = newHT()
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 2, 6))

        ht2 = newHT()
        ibd.Ht_Give(ht2, makeMarkedHashKey(0, 4, 8))

        self.checkHAMV([ht1, ht2], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht1,ht2)

    def testHAMV07_2Table_Double_Sandwich_Duplicate(self):
        ht1 = newHT()
        ibd.Ht_Give(ht1, makeMarkedHashKey(0, 2, 8))

        ht2 = newHT()
        ibd.Ht_Give(ht2, makeMarkedHashKey(0, 4, 6))

        self.checkHAMV([ht1,ht2], 0, [0,1,8,9], [2,3,6,7], [4,5])
        decRef(ht1,ht2)

    def testHAMV08_50Table_Unique_Keys(self):
        
        distinct_values = []

        def new_Table(i):
            ht = newHT()
            ibd.Ht_Give(ht, makeMarkedHashKey(i, 2*i, 2*i+2))
            distinct_values.append([2*i, 2*i +1])

            return ht

        htl = [new_Table(i) for i in xrange(50)]

        self.checkHAMV(htl, -1, *distinct_values)
        decRef(*htl)

    def testHAMV08_50Table_Unique_Keys_Reversed(self):
        
        distinct_values = []

        def new_Table(i):
            ht = newHT()
            ibd.Ht_Give(ht, makeMarkedHashKey(i, 2*i, 2*i+2))
            distinct_values.append([2*i, 2*i +1])

            return ht

        htl = [new_Table(i) for i in xrange(50)]

        self.checkHAMV(reversed(htl), -1, *distinct_values)
        decRef(*htl)

    def testHAMV08_100Table_Unique_Keys_Random(self):
        
        distinct_values = []

        def new_Table(i):
            ht = newHT()
            ibd.Ht_Give(ht, makeMarkedHashKey(i, 2*i, 2*i+2))
            distinct_values.append([2*i, 2*i +1])

            return ht

        htl = [new_Table(i) for i in xrange(500)]

        self.checkHAMV(reversed(htl), -1, *distinct_values)
        decRef(*htl)

    def testHAMV09_50Table_Same_Keys(self):
        
        distinct_values = []

        def new_Table(i):
            ht = newHT()
            ibd.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*i+2))
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
            ibd.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*(2*N - i)))
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
            ibd.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*(2*N - i)))
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
            ibd.Ht_Give(ht, makeMarkedHashKey(0, 2*i, 2*(2*N - i)))
            distinct_values.append([2*i, 2*i+1, 2*(2*N - i) - 2, 2*(2*N - i) - 1])

            return ht

        htl = [new_Table(i) for i in xrange(N)]

        self.checkHAMV(htl, -1, [-1], *distinct_values)
        decRef(*htl)

    def testHAMV12_OverwriteWithSame(self):
        ht = newHT()
        h1 = addMarkerInfo(makeHashKey(1), 2, 6)
        h2 = addMarkerInfo(makeHashKey(2), 4, 8)
        ibd.Ht_Set(ht, h1)
        ibd.Ht_Set(ht, h2)
        
        hl1 = getHashMList(ht, range(10))
        self.checkHAMV([ht], 0, [0,1,8,9], [2,3], [4,5], [6,7])

        ibd.Ht_Set(ht, h1)

        self.assert_(hl1 == getHashMList(ht, range(10)))

        ibd.Ht_Set(ht, h2)

        self.assert_(hl1 == getHashMList(ht, range(10)))

        self.assert_(ibd.O_RefCount(h1) == 2)
        self.assert_(ibd.O_RefCount(h2) == 2)

        decRef(ht)
        decRef(h1)
        decRef(h2)

    def testHAMV13_RegiveWithSetDefault(self):
        ht = newHT()
        h1 = addMarkerInfo(makeHashKey(1), 2, 6)
        h2 = addMarkerInfo(makeHashKey(2), 4, 8)
        ibd.Ht_Set(ht, h1)
        ibd.Ht_Set(ht, h2)
        
        hl1 = getHashMList(ht, range(10))
        self.checkHAMV([ht], 0, [0,1,8,9], [2,3], [4,5], [6,7])

        ibd.Ht_SetDefault(ht, h1)

        self.assert_(hl1 == getHashMList(ht, range(10)))

        ibd.Ht_SetDefault(ht, h2)

        self.assert_(hl1 == getHashMList(ht, range(10)))

        self.assert_(ibd.O_RefCount(h1) == 2)
        self.assert_(ibd.O_RefCount(h2) == 2)

        decRef(ht)
        decRef(h1)
        decRef(h2)

    def testHAMV14_OverwriteSameWithGive(self):
        ht = newHT()
        h1 = addMarkerInfo(makeHashKey(1), 2, 6)
        h2 = addMarkerInfo(makeHashKey(2), 4, 8)
        ibd.Ht_Set(ht, h1)
        ibd.Ht_Set(ht, h2)
        
        hl1 = getHashMList(ht, range(10))
        self.checkHAMV([ht], 0, [0,1,8,9], [2,3], [4,5], [6,7])

        self.assert_(ibd.O_RefCount(h1) == 2)
        self.assert_(ibd.O_RefCount(h2) == 2)

        ibd.Ht_Give(ht, h1)

        self.assert_(ibd.O_RefCount(h1) == 1)
        self.assert_(hl1 == getHashMList(ht, range(10)))

        ibd.Ht_Give(ht, h2)

        self.assert_(ibd.O_RefCount(h2) == 1)
        self.assert_(hl1 == getHashMList(ht, range(10)))

        decRef(ht)

class TestHashTableSetOps(unittest.TestCase):

    def checkSetOp(self, op, hkl1, hkl2):
        ht1 = newHT()
        ht2 = newHT()

        def addIn(ht, hkl):

            for hk in hkl:
                if type(hk) is int:
                    ibd.Ht_Give(ht, makeHashKey(hk))
                elif type(hk) is tuple:
                    ibd.Ht_Give(ht, makeMarkedHashKey(*hk))
                else:
                    ibd.Ht_Give(ht, hk)

        addIn(ht1, hkl1)
        addIn(ht2, hkl2)

        r_set = range(10)

        s1 = getHTValiditySet(ht1, r_set)
        s2 = getHTValiditySet(ht2, r_set)

        if op == "union":
            ht3 = ibd.Ht_Union(ht1, ht2)
            s3_true = s1 | s2
        elif op == "intersection":
            ht3 = ibd.Ht_Intersection(ht1, ht2)
            s3_true = s1 & s2
        elif op == "difference":
            ht3 = ibd.Ht_Difference(ht1, ht2)
            s3_true = s1 - s2
        else:
            assert False

        s3 = getHTValiditySet(ht3, r_set)

        self.assert_(s3 == s3_true,
                     "\nin true, not test: %s;\nin test, not true: %s"
                     % ((",".join(sorted(t[0][:4] + "-%d" % t[1] for t in (s3_true - s3)))),
                        (",".join(sorted(t[0][:4] + "-%d" % t[1] for t in (s3 - s3_true))))))

        decRef(ht1, ht2, ht3)

    def setConsistencyTest(self, op, marker_range, n_keys, n_key_marker_ranges, n_unique):

        r = marker_range

        def newRandomHt(offset):
            ht = newHT()

            random.seed(0)

            for k in range(offset, n_keys + offset):
                
                key_range_args = []
                
                for n in range(n_key_marker_ranges):
                    a = random.randint(*r)
                    b = random.randint(*r)
                    
                    key_range_args.append(min(a,b))
                    key_range_args.append(max(a,b) + 1)

                ibd.Ht_Give(ht, makeMarkedHashKey(k, *key_range_args))

            return ht
        
        ht1 = newRandomHt(0)
        ht2 = newRandomHt(n_unique)

        s1 = getHTValiditySet(ht1, range(*r))
        s2 = getHTValiditySet(ht2, range(*r))

        if op == "union":
            ht3 = ibd.Ht_Union(ht1, ht2)
            s3_true = s1 | s2
        elif op == "intersection":
            ht3 = ibd.Ht_Intersection(ht1, ht2)
            s3_true = s1 & s2
        elif op == "difference":
            ht3 = ibd.Ht_Difference(ht1, ht2)
            s3_true = s1 - s2
        else:
            assert False

        s3 = getHTValiditySet(ht3, range(*r))

        self.assert_(s3 == s3_true,
                     "\nin true, not test: %s;\nin test, not true: %s"
                     % ((",".join(sorted(t[0][:4] + "-%d" % t[1] for t in (s3_true - s3)))),
                        (",".join(sorted(t[0][:4] + "-%d" % t[1] for t in (s3 - s3_true))))))

        decRef(ht1, ht2, ht3)

    def test01_Union_01(self):
        self.checkSetOp("union", [0], [1])
    def test01_Union_02_marked(self):
        self.checkSetOp("union", [(0, 2, 4)], [(1,3,5)])
    def test01_Union_03_marked_same_range(self):
        self.checkSetOp("union", [(0, 2, 4)], [(1,2,4)])
    def test01_Union_04_marked_same_key(self):
        self.checkSetOp("union", [(0, 2, 4)], [(0,2,4)])
    def test01_Union_05_marked_unmarked(self):
        self.checkSetOp("union", [(0, 2, 4)], [1])
    def test01_Union_06_marked_unmarked_same_key_01(self):
        self.checkSetOp("union", [(0, 2, 4)], [0])
    def test01_Union_07_marked_unmarked_same_key_02(self):
        self.checkSetOp("union", [0], [(0, 2, 4)])
    def test01_Union_08_many_ranges(self):
        from test_markers import miSet_06_large

        mi1, mi2 = miSet_06_large(2)
        h1 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h1, mi1)

        h2 = makeHashKey(1)
        ibd.H_GiveMarkerInfo(h2, mi2)

        self.checkSetOp("union", [h1], [h2])

    def test01_Union_09_many_ranges_same_key(self):
        from test_markers import miSet_06_large

        mi1, mi2 = miSet_06_large(2)
        h1 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h1, mi1)

        h2 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h2, mi2)

        self.checkSetOp("union", [h1], [h2])

    def test02_Union_Random_Consistency_01_small_same_keysets(self):
        self.setConsistencyTest("union", (-20,20), 5, 5, 0)

    def test02_Union_Random_Consistency_01_small_different_keysets(self):
        self.setConsistencyTest("union", (-20,20), 5, 5, 5)

    def test02_Union_Random_Consistency_02_medium_sparse_same_keysets(self):
        self.setConsistencyTest("union", (-20,20), 10, 1, 0)

    def test02_Union_Random_Consistency_02_medium_sparse_different_keysets(self):
        self.setConsistencyTest("union", (-20,20), 10, 1, 10)

    def test02_Union_Random_Consistency_02_medium_sparse_overlapping_keysets(self):
        self.setConsistencyTest("union", (-20,20), 10, 1, 5)

    def test02_Union_Random_Consistency_03_large_contiguous_same_keysets(self):
        self.setConsistencyTest("union", (-100,100), 100, 1, 0)

    def test02_Union_Random_Consistency_03_large_contiguous_different_keysets(self):
        self.setConsistencyTest("union", (-100,100), 100, 1, 100)

    def test02_Union_Random_Consistency_03_large_contiguous_overlappy_keysets(self):
        self.setConsistencyTest("union", (-100,100), 100, 1, 5)

    def test02_Union_Random_Consistency_03_large_same_keysets(self):
        self.setConsistencyTest("union", (-100,100), 100, 10, 0)

    def test02_Union_Random_Consistency_03_large_different_keysets(self):
        self.setConsistencyTest("union", (-100,100), 100, 10, 100)

    def test02_Union_Random_Consistency_03_large_overlapping_keysets(self):
        self.setConsistencyTest("union", (-100,100), 100, 10, 50)

    def test11_Intersection_01(self):
        self.checkSetOp("intersection", [0], [1])
    def test11_Intersection_02_marked(self):
        self.checkSetOp("intersection", [(0, 2, 4)], [(1,3,5)])
    def test11_Intersection_03_marked_same_range(self):
        self.checkSetOp("intersection", [(0, 2, 4)], [(1,2,4)])
    def test11_Intersection_04_marked_same_key(self):
        self.checkSetOp("intersection", [(0, 2, 4)], [(0,2,4)])
    def test11_Intersection_05_marked_unmarked(self):
        self.checkSetOp("intersection", [(0, 2, 4)], [1])
    def test11_Intersection_06_marked_unmarked_same_key_01(self):
        self.checkSetOp("intersection", [(0, 2, 4)], [0])
    def test11_Intersection_07_marked_unmarked_same_key_02(self):
        self.checkSetOp("intersection", [0], [(0, 2, 4)])
    def test11_Intersection_08_many_ranges(self):
        from test_markers import miSet_06_large

        mi1, mi2 = miSet_06_large(2)
        h1 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h1, mi1)

        h2 = makeHashKey(1)
        ibd.H_GiveMarkerInfo(h2, mi2)

        self.checkSetOp("intersection", [h1], [h2])

    def test11_Intersection_09_many_ranges_same_key(self):
        from test_markers import miSet_06_large

        mi1, mi2 = miSet_06_large(2)
        h1 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h1, mi1)

        h2 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h2, mi2)

        self.checkSetOp("intersection", [h1], [h2])

    def test12_Intersection_Random_Consistency_01_small_same_keysets(self):
        self.setConsistencyTest("intersection", (-20,20), 5, 5, 0)

    def test12_Intersection_Random_Consistency_01_small_different_keysets(self):
        self.setConsistencyTest("intersection", (-20,20), 5, 5, 5)

    def test12_Intersection_Random_Consistency_02_medium_sparse_same_keysets(self):
        self.setConsistencyTest("intersection", (-20,20), 10, 1, 0)

    def test12_Intersection_Random_Consistency_02_medium_sparse_different_keysets(self):
        self.setConsistencyTest("intersection", (-20,20), 10, 1, 10)

    def test12_Intersection_Random_Consistency_02_medium_sparse_overlapping_keysets(self):
        self.setConsistencyTest("intersection", (-20,20), 10, 1, 5)

    def test12_Intersection_Random_Consistency_03_large_contiguous_same_keysets(self):
        self.setConsistencyTest("intersection", (-100,100), 100, 1, 0)

    def test12_Intersection_Random_Consistency_03_large_contiguous_different_keysets(self):
        self.setConsistencyTest("intersection", (-100,100), 100, 1, 100)

    def test12_Intersection_Random_Consistency_03_large_contiguous_overlappy_keysets(self):
        self.setConsistencyTest("intersection", (-100,100), 100, 1, 5)

    def test12_Intersection_Random_Consistency_03_large_same_keysets(self):
        self.setConsistencyTest("intersection", (-100,100), 100, 10, 0)

    def test12_Intersection_Random_Consistency_03_large_different_keysets(self):
        self.setConsistencyTest("intersection", (-100,100), 100, 10, 100)

    def test12_Intersection_Random_Consistency_03_large_overlapping_keysets(self):
        self.setConsistencyTest("intersection", (-10,10), 100, 5, 50)

    def test21_Difference_01(self):
        self.checkSetOp("difference", [0], [1])
    def test21_Difference_02_marked(self):
        self.checkSetOp("difference", [(0, 2, 4)], [(1,3,5)])
    def test21_Difference_03_marked_same_range(self):
        self.checkSetOp("difference", [(0, 2, 4)], [(1,2,4)])
    def test21_Difference_04_marked_same_key(self):
        self.checkSetOp("difference", [(0, 2, 4)], [(0,2,4)])
    def test21_Difference_05_marked_unmarked(self):
        self.checkSetOp("difference", [(0, 2, 4)], [1])
    def test21_Difference_06_marked_unmarked_same_key_01(self):
        self.checkSetOp("difference", [(0, 2, 4)], [0])
    def test21_Difference_07_marked_unmarked_same_key_02(self):
        self.checkSetOp("difference", [0], [(0, 2, 4)])
    def test21_Difference_08_many_ranges(self):
        from test_markers import miSet_06_large

        mi1, mi2 = miSet_06_large(2)
        h1 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h1, mi1)

        h2 = makeHashKey(1)
        ibd.H_GiveMarkerInfo(h2, mi2)

        self.checkSetOp("difference", [h1], [h2])

    def test21_Difference_09_many_ranges_same_key(self):
        from test_markers import miSet_06_large

        mi1, mi2 = miSet_06_large(2)
        h1 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h1, mi1)

        h2 = makeHashKey(0)
        ibd.H_GiveMarkerInfo(h2, mi2)

        self.checkSetOp("difference", [h1], [h2])

    def test22_Difference_Random_Consistency_01_small_same_keysets(self):
        self.setConsistencyTest("difference", (-20,20), 5, 5, 0)

    def test22_Difference_Random_Consistency_01_small_different_keysets(self):
        self.setConsistencyTest("difference", (-20,20), 5, 5, 5)

    def test22_Difference_Random_Consistency_02_medium_sparse_same_keysets(self):
        self.setConsistencyTest("difference", (-20,20), 10, 1, 0)

    def test22_Difference_Random_Consistency_02_medium_sparse_different_keysets(self):
        self.setConsistencyTest("difference", (-20,20), 10, 1, 10)

    def test22_Difference_Random_Consistency_02_medium_sparse_overlapping_keysets(self):
        self.setConsistencyTest("difference", (-20,20), 10, 1, 5)

    def test22_Difference_Random_Consistency_03_large_contiguous_same_keysets(self):
        self.setConsistencyTest("difference", (-100,100), 100, 1, 0)

    def test22_Difference_Random_Consistency_03_large_contiguous_different_keysets(self):
        self.setConsistencyTest("difference", (-100,100), 100, 1, 100)

    def test22_Difference_Random_Consistency_03_large_contiguous_overlappy_keysets(self):
        self.setConsistencyTest("difference", (-100,100), 100, 1, 5)

    def test22_Difference_Random_Consistency_03_large_same_keysets(self):
        self.setConsistencyTest("difference", (-100,100), 100, 10, 0)

    def test22_Difference_Random_Consistency_03_large_different_keysets(self):
        self.setConsistencyTest("difference", (-100,100), 100, 10, 100)

    def test22_Difference_Random_Consistency_03_large_overlapping_keysets(self):
        self.setConsistencyTest("difference", (-100,100), 100, 10, 50)
        

if __name__ == '__main__':
    unittest.main()


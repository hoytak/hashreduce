#!/usr/bin/env python
import unittest, random

from common import *
from ctypes import *

def addMiRange(mi, r1, r2):
    ibd.Mi_AddValidRange(mi, c_long(r1), c_long(r2))

newMii = ibd.Mii_New
newMii.restype = ctypes.c_void_p
delMii = ibd.Mii_Delete

newMiri = ibd.Miri_New
newMiri.restype = ctypes.c_void_p
delMiri = ibd.Miri_Delete

isValid = ibd.Mi_IsValid

Mii_Next = ibd.Mii_Next

MiCopy = ibd.Mi_Copy
MiCopy.restype = ctypes.c_void_p

class MarkerRange(Structure):
    _fields_ = [("start", c_long),
                ("end", c_long)]

Miri_Next = ibd.Miri_Next

Mi_Complement = ibd.Mi_Complement
Mi_Complement.restype = ctypes.c_void_p

Mi_Union = ibd.Mi_Union
Mi_Union.restype = ctypes.c_void_p

Mi_Intersection = ibd.Mi_Intersection
Mi_Intersection.restype = ctypes.c_void_p

Mi_Difference = ibd.Mi_Difference
Mi_Difference.restype = ctypes.c_void_p

Mi_SymmetricDifference = ibd.Mi_SymmetricDifference
Mi_SymmetricDifference.restype = ctypes.c_void_p

ibd.Mr_Plus_Infinity.restype = ctypes.c_long
ibd.Mr_Minus_Infinity.restype = ctypes.c_long

ibd.Mr_Start.restype = ctypes.c_long
ibd.Mr_End.restype = ctypes.c_long

mr_plus_inf = ibd.Mr_Plus_Infinity()
mr_minus_inf = ibd.Mr_Minus_Infinity()

def delMi(*mi_list):
    for mi in mi_list:
        ibd.O_DecRef(mi)


def newMi(r1, r2, *args):
    ibd.Mi_New.restype = ctypes.c_void_p
    
    mi = ibd.Mi_New(c_long(r1), c_long(r2))
    
    for r1, r2 in zip(args[::2], args[1::2]):
        addMiRange(mi,r1,r2)

    mi_copy = MiCopy(mi)
    assert ibd.Mi_Equal(mi, mi_copy)
    ibd.O_DecRef(mi_copy)

    assert r1 >= r2 or ibd.Mi_ValidAnywhere(mi)

    return mi


def checkRanges(mi, valid_set, invalid_set):
    wrongly_invalid = [m for m in valid_set if not isValid(mi, m)]
    wrongly_valid   = [m for m in invalid_set if isValid(mi, m)]

    #print "valid_set = ", valid_set
    #print "wrongly_invalid = ", wrongly_invalid

    #print "invalid_set = ", invalid_set
    #print "wrongly_valid = ", wrongly_valid

    assert len(wrongly_valid) == 0 and len(wrongly_invalid) == 0,\
        "Wrongly Valid: %s; Wrongly Invalid: %s" \
        % (",".join(str(m) for m in sorted(wrongly_valid)), 
           ",".join(str(m) for m in sorted(wrongly_invalid)))


def checkSetOperation(operation, mi1, mi2, test_range = None):

    if test_range is None:
        test_range = [mr_minus_inf] + range(-50, 50) + [mr_plus_inf]

    def validSet(mi):
        return set( m for m in test_range if isValid(mi, c_long(m)) )

    s_1 = validSet(mi1)
    s_2 = validSet(mi2)

    # Check copying is okay
    mi1_copy = MiCopy(mi1)
    assert s_1 == validSet(mi1_copy)
    assert ibd.Mi_Equal(mi1, mi1_copy)

    ibd.O_DecRef(mi1_copy)

    mi2_copy = MiCopy(mi2)
    assert s_2 == validSet(mi2_copy)
    assert ibd.Mi_Equal(mi2, mi2_copy)
    ibd.O_DecRef(mi2_copy)
    
    if operation == "union":
        mi_f = Mi_Union
        s_f = set.union

    elif operation == "intersection":
        mi_f = Mi_Intersection
        s_f = set.intersection

    elif operation == "difference":
        mi_f = Mi_Difference
        s_f = set.difference

    elif operation == "symmetricDifference":
        mi_f = Mi_SymmetricDifference
        s_f = set.symmetric_difference

    mi_u = mi_f(mi1, mi2)
    test_set = validSet(mi_u)

    correct_set = s_f(s_1, s_2)

    if test_set != correct_set:
        print "FAIL Report (operation %s):" % operation
        print "mi1 = ",
        ibd.Mi_debug_printMi(mi1);
        print "\n s1 = ", sorted(s_1),
        print "\nmi2 = ",
        ibd.Mi_debug_printMi(mi2);
        print "\n s2 = ", sorted(s_2),
        print "\nmi_c = ",
        ibd.Mi_debug_printMi(mi_u);
        print "\n s_c = ", sorted(correct_set),
        print ""

        assert test_set == correct_set, \
            ("Op %s: In correct, not test: %s; in test, not correct: %s"
             % (operation,
                ",".join(str(m) for m in sorted(correct_set - test_set)),  
                ",".join(str(m) for m in sorted(test_set - correct_set))))


    # Assume sets are disposable
    delMi(mi1, mi2, mi_u)
    

# Some set operations

def miSet_01_overlap():
    return (newMi(0,5), newMi(3,8))

def miSet_02_nonoverlap():
    return (newMi(0,3), newMi(5,8))

def miSet_03_multiple():
    return (newMi(0,3, 5,12), newMi(2,7))

def miSet_04_multiple_disjoint():
    return (newMi(0,3, 8,12), newMi(5,7))

def miSet_05_multiple():
    return (newMi(0,3, 8,12), newMi(5,7))

def miSet_06_large(offset):
    mi1 = newMi(2,5)
    mi2 = newMi(2+offset,5+offset)

    for i in xrange(-10,10):
        addMiRange(mi1, i*10 + 2, i*10 + 5)
        addMiRange(mi2, i*10 + 2 + offset, i*10 + 5 + offset)

    return (mi1, mi2)

def miSet_07_large(r, n):

    random.seed(n + r[0] + r[1])

    def get():
        s = set()

        mi = newMi(0,0)

        for i in range(n):
            a = random.randint(*r)
            b = random.randint(*r)
            if a > b:
                a, b = b, a

            for x in range(a,b):
                s.add(x)

            addMiRange(mi, a, b)

        for i in range(*r):
            if i in s:
                if not ibd.Mi_IsValid(mi, c_long(i)):
                    print "marker range = "
                    ibd.Mi_debug_printMi(mi)
                    print ""
                    print ("After adding [%d, %d), %d is not valid"
                           % (a, b, i) )
                    raise AssertionError

        return mi

    return (get(), get())


class TestMarkers(unittest.TestCase):

    def checkRanges(self, mi, okay, bad):
        
        for m in okay:
            v = ibd.Mi_IsValid(mi, c_long(m))
            self.assert_(v != 0, "%d should be valid." % m)

        for m in bad:
            v = ibd.Mi_IsValid(mi, c_long(m))
            self.assert_(v == 0, "%d should not be valid." % m)


    def test01Simple(self):
        mi = newMi(5, 10)
        self.checkRanges(mi, okay=[5,6,7,8,9], bad=[3,4,10,11])
        delMi(mi)
        
    def test02_TwoDistinctRanges(self):
        mi = newMi(5, 7, 8, 10)
        self.checkRanges(mi, okay=[5,6,8,9], bad=[3,4,7,10,11])
        delMi(mi)
        
    def test03_ManyDistinctRanges(self):
        mi = newMi(0,2)

        for i in range(1, 10):
            addMiRange(mi, 10*i, 10*i+2)

        okay_base = [0,1]
        bad_base  = [2,3,4,5,6,7,8,9]

        for i in range(10):
            self.checkRanges(mi, 
                             okay=[10*i + a for a in okay_base],
                             bad = [10*i + a for a in bad_base])
        delMi(mi)
            
    def getNewMi(self, s, e, edge_range_category):
        
        mi = newMi(s,e)
        
        if 'b' in edge_range_category:
            addMiRange(mi, -10, -5)

        if 'a' in edge_range_category:
            addMiRange(mi, 10, 15)
    
        if 'B' in edge_range_category:
            for i in range(1, 10):
                addMiRange(mi, -10*i, -10*i+5)

        if 'A' in edge_range_category:
            for i in range(1, 10):
                addMiRange(mi, 10*i, 10*i+5)

        return mi


    def check_ExtendForward_01(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        self.checkRanges(mi, okay=[0,1], bad = [-1,2,3])
        addMiRange(mi, 1, 3)
        self.checkRanges(mi, okay=[0,1,2], bad = [-1,3])
        delMi(mi)

    def test04_ExtendForward_01  (self): self.check_ExtendForward_01('')
    def test04_ExtendForward_01a (self): self.check_ExtendForward_01('a')
    def test04_ExtendForward_01b (self): self.check_ExtendForward_01('b')
    def test04_ExtendForward_01ab(self): self.check_ExtendForward_01('ab')
    def test04_ExtendForward_01A (self): self.check_ExtendForward_01('A')
    def test04_ExtendForward_01Ab(self): self.check_ExtendForward_01('Ab')
    def test04_ExtendForward_01B (self): self.check_ExtendForward_01('B')
    def test04_ExtendForward_01aB(self): self.check_ExtendForward_01('aB')
    def test04_ExtendForward_01AB(self): self.check_ExtendForward_01('AB')

    def check_ExtendForward_02(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        self.checkRanges(mi, okay=[0,1], bad = [-1,2,3])
        addMiRange(mi, 2, 3)   # corner case w/ 2
        self.checkRanges(mi, okay=[0,1,2], bad = [-1,3])
        delMi(mi)

    def test04_ExtendForward_02  (self): self.check_ExtendForward_02('')
    def test04_ExtendForward_02a (self): self.check_ExtendForward_02('a')
    def test04_ExtendForward_02b (self): self.check_ExtendForward_02('b')
    def test04_ExtendForward_02ab(self): self.check_ExtendForward_02('ab')
    def test04_ExtendForward_02A (self): self.check_ExtendForward_02('A')
    def test04_ExtendForward_02Ab(self): self.check_ExtendForward_02('Ab')
    def test04_ExtendForward_02B (self): self.check_ExtendForward_02('B')
    def test04_ExtendForward_02aB(self): self.check_ExtendForward_02('aB')
    def test04_ExtendForward_02AB(self): self.check_ExtendForward_02('AB')

    def check_ExtendForward_03(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        self.checkRanges(mi, okay=[0,1], bad = [-1,2,3])
        addMiRange(mi, 0, 3)   # corner case w/ 0
        self.checkRanges(mi, okay=[0,1,2], bad = [-1,3])
        delMi(mi)

    def test04_ExtendForward_03  (self): self.check_ExtendForward_03('')
    def test04_ExtendForward_03a (self): self.check_ExtendForward_03('a')
    def test04_ExtendForward_03b (self): self.check_ExtendForward_03('b')
    def test04_ExtendForward_03ab(self): self.check_ExtendForward_03('ab')
    def test04_ExtendForward_03A (self): self.check_ExtendForward_03('A')
    def test04_ExtendForward_03Ab(self): self.check_ExtendForward_03('Ab')
    def test04_ExtendForward_03B (self): self.check_ExtendForward_03('B')
    def test04_ExtendForward_03aB(self): self.check_ExtendForward_03('aB')
    def test04_ExtendForward_03AB(self): self.check_ExtendForward_03('AB')


    def check_Combine_01(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        addMiRange(mi, 3, 5)
        self.checkRanges(mi, okay=[0,1,3,4], bad = [-1,2,6])
        addMiRange(mi, 1, 4)
        self.checkRanges(mi, okay=[0,1,2,3,4], bad = [-1,5])
        delMi(mi)

    def test05_Combine_01  (self): self.check_Combine_01('')
    def test05_Combine_01a (self): self.check_Combine_01('a')
    def test05_Combine_01b (self): self.check_Combine_01('b')
    def test05_Combine_01ab(self): self.check_Combine_01('ab')
    def test05_Combine_01A (self): self.check_Combine_01('A')
    def test05_Combine_01Ab(self): self.check_Combine_01('Ab')
    def test05_Combine_01B (self): self.check_Combine_01('B')
    def test05_Combine_01aB(self): self.check_Combine_01('aB')
    def test05_Combine_01AB(self): self.check_Combine_01('AB')


    def check_Combine_02(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        addMiRange(mi, 3, 5)
        self.checkRanges(mi, okay=[0,1,3,4], bad = [-1,2,6])
        addMiRange(mi, 2, 3)  # Corner case
        self.checkRanges(mi, okay=[0,1,2,3,4], bad = [-1,5])
        delMi(mi)

    def test05_Combine_02  (self): self.check_Combine_02('')
    def test05_Combine_02a (self): self.check_Combine_02('a')
    def test05_Combine_02b (self): self.check_Combine_02('b')
    def test05_Combine_02ab(self): self.check_Combine_02('ab')
    def test05_Combine_02A (self): self.check_Combine_02('A')
    def test05_Combine_02Ab(self): self.check_Combine_02('Ab')
    def test05_Combine_02B (self): self.check_Combine_02('B')
    def test05_Combine_02aB(self): self.check_Combine_02('aB')
    def test05_Combine_02AB(self): self.check_Combine_02('AB')


    def check_Combine_03(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        addMiRange(mi, 3, 5)
        self.checkRanges(mi, okay=[0,1,3,4], bad = [-1,2,6])
        addMiRange(mi, 0, 5)  # Corner case
        self.checkRanges(mi, okay=[0,1,2,3,4], bad = [-1,5])
        delMi(mi)

    def test05_Combine_03  (self): self.check_Combine_03('')
    def test05_Combine_03a (self): self.check_Combine_03('a')
    def test05_Combine_03b (self): self.check_Combine_03('b')
    def test05_Combine_03ab(self): self.check_Combine_03('ab')
    def test05_Combine_03A (self): self.check_Combine_03('A')
    def test05_Combine_03Ab(self): self.check_Combine_03('Ab')
    def test05_Combine_03B (self): self.check_Combine_03('B')
    def test05_Combine_03aB(self): self.check_Combine_03('aB')
    def test05_Combine_03AB(self): self.check_Combine_03('AB')


    def check_Combine_04(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        addMiRange(mi, 3, 5)
        self.checkRanges(mi, okay=[0,1,3,4], bad = [-1,2,6])
        addMiRange(mi, 0, 3)  # Corner case
        self.checkRanges(mi, okay=[0,1,2,3,4], bad = [-1,5])
        delMi(mi)

    def test05_Combine_04  (self): self.check_Combine_04('')
    def test05_Combine_04a (self): self.check_Combine_04('a')
    def test05_Combine_04b (self): self.check_Combine_04('b')
    def test05_Combine_04ab(self): self.check_Combine_04('ab')
    def test05_Combine_04A (self): self.check_Combine_04('A')
    def test05_Combine_04Ab(self): self.check_Combine_04('Ab')
    def test05_Combine_04B (self): self.check_Combine_04('B')
    def test05_Combine_04aB(self): self.check_Combine_04('aB')
    def test05_Combine_04AB(self): self.check_Combine_04('AB')

    def check_Combine_05(self, edge_range_category):
        mi = self.getNewMi(0,2,edge_range_category)
        addMiRange(mi, 3, 5)
        self.checkRanges(mi, okay=[0,1,3,4], bad = [-1,2,6])
        addMiRange(mi, 2, 5)  # Corner case
        self.checkRanges(mi, okay=[0,1,2,3,4], bad = [-1,5])
        delMi(mi)

    def test05_Combine_05  (self): self.check_Combine_05('')
    def test05_Combine_05a (self): self.check_Combine_05('a')
    def test05_Combine_05b (self): self.check_Combine_05('b')
    def test05_Combine_05ab(self): self.check_Combine_05('ab')
    def test05_Combine_05A (self): self.check_Combine_05('A')
    def test05_Combine_05Ab(self): self.check_Combine_05('Ab')
    def test05_Combine_05B (self): self.check_Combine_05('B')
    def test05_Combine_05aB(self): self.check_Combine_05('aB')
    def test05_Combine_05AB(self): self.check_Combine_05('AB')


    def check_ExtendBack_01(self, edge_range_category):
        mi = self.getNewMi(1,3,edge_range_category)
        self.checkRanges(mi, okay=[1,2], bad = [-1,0,3])
        addMiRange(mi, 0, 2)
        self.checkRanges(mi, okay=[0,1,2], bad = [-1,3])
        delMi(mi)

    def test06_ExtendBack_01  (self): self.check_ExtendBack_01('')
    def test06_ExtendBack_01a (self): self.check_ExtendBack_01('a')
    def test06_ExtendBack_01b (self): self.check_ExtendBack_01('b')
    def test06_ExtendBack_01ab(self): self.check_ExtendBack_01('ab')
    def test06_ExtendBack_01A (self): self.check_ExtendBack_01('A')
    def test06_ExtendBack_01Ab(self): self.check_ExtendBack_01('Ab')
    def test06_ExtendBack_01B (self): self.check_ExtendBack_01('B')
    def test06_ExtendBack_01aB(self): self.check_ExtendBack_01('aB')
    def test06_ExtendBack_01AB(self): self.check_ExtendBack_01('AB')

    def check_ExtendBack_02(self, edge_range_category):
        mi = self.getNewMi(1,3,edge_range_category)
        self.checkRanges(mi, okay=[1,2], bad = [-1,0,3])
        addMiRange(mi, 0, 1)   # corner case w/ 2
        self.checkRanges(mi, okay=[0,1,2], bad = [-1,3])
        delMi(mi)

    def test06_ExtendBack_02  (self): self.check_ExtendBack_02('')
    def test06_ExtendBack_02a (self): self.check_ExtendBack_02('a')
    def test06_ExtendBack_02b (self): self.check_ExtendBack_02('b')
    def test06_ExtendBack_02ab(self): self.check_ExtendBack_02('ab')
    def test06_ExtendBack_02A (self): self.check_ExtendBack_02('A')
    def test06_ExtendBack_02Ab(self): self.check_ExtendBack_02('Ab')
    def test06_ExtendBack_02B (self): self.check_ExtendBack_02('B')
    def test06_ExtendBack_02aB(self): self.check_ExtendBack_02('aB')
    def test06_ExtendBack_02AB(self): self.check_ExtendBack_02('AB')

    def check_ExtendBack_03(self, edge_range_category):
        mi = self.getNewMi(1,3,edge_range_category)
        self.checkRanges(mi, okay=[1,2], bad = [-1,0,3])
        addMiRange(mi, 0, 3)   # corner case w/ 0
        self.checkRanges(mi, okay=[0,1,2], bad = [-1,3])
        delMi(mi)

    def test06_ExtendBack_03  (self): self.check_ExtendBack_03('')
    def test06_ExtendBack_03a (self): self.check_ExtendBack_03('a')
    def test06_ExtendBack_03b (self): self.check_ExtendBack_03('b')
    def test06_ExtendBack_03ab(self): self.check_ExtendBack_03('ab')
    def test06_ExtendBack_03A (self): self.check_ExtendBack_03('A')
    def test06_ExtendBack_03Ab(self): self.check_ExtendBack_03('Ab')
    def test06_ExtendBack_03B (self): self.check_ExtendBack_03('B')
    def test06_ExtendBack_03aB(self): self.check_ExtendBack_03('aB')
    def test06_ExtendBack_03AB(self): self.check_ExtendBack_03('AB')


    def check_ExtendBoth_01(self, edge_range_category):
        mi = self.getNewMi(1,3,edge_range_category)
        self.checkRanges(mi, okay=[1,2], bad = [-1,0,3])
        addMiRange(mi, 0, 5)   # corner case w/ 0
        self.checkRanges(mi, okay=[0,1,2,3,4], bad = [-1,5])
        delMi(mi)

    def test07_ExtendBoth_01  (self): self.check_ExtendBoth_01('')
    def test07_ExtendBoth_01a (self): self.check_ExtendBoth_01('a')
    def test07_ExtendBoth_01b (self): self.check_ExtendBoth_01('b')
    def test07_ExtendBoth_01ab(self): self.check_ExtendBoth_01('ab')
    def test07_ExtendBoth_01A (self): self.check_ExtendBoth_01('A')
    def test07_ExtendBoth_01Ab(self): self.check_ExtendBoth_01('Ab')
    def test07_ExtendBoth_01B (self): self.check_ExtendBoth_01('B')
    def test07_ExtendBoth_01aB(self): self.check_ExtendBoth_01('aB')
    def test07_ExtendBoth_01AB(self): self.check_ExtendBoth_01('AB')


    def check_ChangeNothing_01(self, edge_range_category):
        mi = self.getNewMi(0,4,edge_range_category)
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        addMiRange(mi, 1, 2)   
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        delMi(mi)

    def test08_ChangeNothing_01  (self): self.check_ChangeNothing_01('')
    def test08_ChangeNothing_01a (self): self.check_ChangeNothing_01('a')
    def test08_ChangeNothing_01b (self): self.check_ChangeNothing_01('b')
    def test08_ChangeNothing_01ab(self): self.check_ChangeNothing_01('ab')
    def test08_ChangeNothing_01A (self): self.check_ChangeNothing_01('A')
    def test08_ChangeNothing_01Ab(self): self.check_ChangeNothing_01('Ab')
    def test08_ChangeNothing_01B (self): self.check_ChangeNothing_01('B')
    def test08_ChangeNothing_01aB(self): self.check_ChangeNothing_01('aB')
    def test08_ChangeNothing_01AB(self): self.check_ChangeNothing_01('AB')


    def check_ChangeNothing_02(self, edge_range_category):
        mi = self.getNewMi(0,4,edge_range_category)
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        addMiRange(mi, 0, 2)   # corner case w/ 0
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        delMi(mi)

    def test08_ChangeNothing_02  (self): self.check_ChangeNothing_02('')
    def test08_ChangeNothing_02a (self): self.check_ChangeNothing_02('a')
    def test08_ChangeNothing_02b (self): self.check_ChangeNothing_02('b')
    def test08_ChangeNothing_02ab(self): self.check_ChangeNothing_02('ab')
    def test08_ChangeNothing_02A (self): self.check_ChangeNothing_02('A')
    def test08_ChangeNothing_02Ab(self): self.check_ChangeNothing_02('Ab')
    def test08_ChangeNothing_02B (self): self.check_ChangeNothing_02('B')
    def test08_ChangeNothing_02aB(self): self.check_ChangeNothing_02('aB')
    def test08_ChangeNothing_02AB(self): self.check_ChangeNothing_02('AB')


    def check_ChangeNothing_03(self, edge_range_category):
        mi = self.getNewMi(0,4,edge_range_category)
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        addMiRange(mi, 0, 4)   # corner case w/ 0
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        delMi(mi)

    def test08_ChangeNothing_03  (self): self.check_ChangeNothing_03('')
    def test08_ChangeNothing_03a (self): self.check_ChangeNothing_03('a')
    def test08_ChangeNothing_03b (self): self.check_ChangeNothing_03('b')
    def test08_ChangeNothing_03ab(self): self.check_ChangeNothing_03('ab')
    def test08_ChangeNothing_03A (self): self.check_ChangeNothing_03('A')
    def test08_ChangeNothing_03Ab(self): self.check_ChangeNothing_03('Ab')
    def test08_ChangeNothing_03B (self): self.check_ChangeNothing_03('B')
    def test08_ChangeNothing_03aB(self): self.check_ChangeNothing_03('aB')
    def test08_ChangeNothing_03AB(self): self.check_ChangeNothing_03('AB')


    def check_ChangeNothing_04(self, edge_range_category):
        mi = self.getNewMi(0,4,edge_range_category)
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        addMiRange(mi, 1, 4)   # corner case w/ 0
        self.checkRanges(mi, okay=[0,1,2,3], bad = [-1,4,5])
        delMi(mi)

    def test08_ChangeNothing_04  (self): self.check_ChangeNothing_04('')
    def test08_ChangeNothing_04a (self): self.check_ChangeNothing_04('a')
    def test08_ChangeNothing_04b (self): self.check_ChangeNothing_04('b')
    def test08_ChangeNothing_04ab(self): self.check_ChangeNothing_04('ab')
    def test08_ChangeNothing_04A (self): self.check_ChangeNothing_04('A')
    def test08_ChangeNothing_04Ab(self): self.check_ChangeNothing_04('Ab')
    def test08_ChangeNothing_04B (self): self.check_ChangeNothing_04('B')
    def test08_ChangeNothing_04aB(self): self.check_ChangeNothing_04('aB')
    def test08_ChangeNothing_04AB(self): self.check_ChangeNothing_04('AB')

    def getMiiList(self, mi):
        mii = newMii(mi)

        ac = []

        while True:
            mr = MarkerRange()
            
            okay = Mii_Next(byref(mr), mii)
            
            if okay:
                ac.append( (ibd.Mr_Start(byref(mr)), ibd.Mr_End(byref(mr)) ) )
            else:
                delMii(mii)
                return ac

    def test10_MarkerRangeIterator_01_simple(self): 
        mi = newMi(5, 7)
        self.assert_(self.getMiiList(mi) == [ (5, 7) ] )


    def test10_MarkerRangeIterator_02_simple(self): 
        mi = newMi(5, 7)
        addMiRange(mi, 8, 10)
        l = self.getMiiList(mi)
        self.assert_(l == [ (5, 7), (8, 10) ] )

    def test10_MarkerRangeIterator_03_simple(self): 
        mi = newMi(5, 7)
        addMiRange(mi, 9, 11)
        addMiRange(mi, 12, 14)
        addMiRange(mi, 13, 15)
        l = self.getMiiList(mi)
        self.assert_(l == [ (5, 7), (9, 11), (12,15) ] )

    def test10_MarkerRangeIterator_04_corner(self): 
        mi = newMi(0, 0)
        l = self.getMiiList(mi)
        self.assert_(l == [], l)

    def test10_MarkerRangeIterator_05_corner(self): 
        mi = newMi(3, 3)
        l = self.getMiiList(mi)
        self.assert_(l == [], l)

    def test10_MarkerRangeIterator_06_corner(self): 
        l = self.getMiiList(0)
        self.assert_(l == [(mr_minus_inf, mr_plus_inf)])

    def getMiriList(self, mi):
        mii = newMiri(mi)

        ac = []

        while True:
            mr = MarkerRange()
            
            okay = Miri_Next(byref(mr), mii)
            
            if okay:
                ac.append( (ibd.Mr_Start(byref(mr)), ibd.Mr_End(byref(mr)) ) )
            else:
                delMiri(mii)
                return ac

    def test11_MarkerRangeRevIterator_01_simple(self): 
        mi = newMi(5, 7)
        self.assert_(self.getMiriList(mi) == [ (5, 7) ] )


    def test11_MarkerRangeRevIterator_02_simple(self): 
        mi = newMi(5, 7)
        addMiRange(mi, 8, 10)
        self.assert_(self.getMiriList(mi) == [ (8, 10), (5, 7) ] )

    def test11_MarkerRangeRevIterator_04_corner(self): 
        mi = newMi(0, 0)
        l = self.getMiriList(mi)
        self.assert_(l == [])

    def test11_MarkerRangeRevIterator_05_corner(self): 
        mi = newMi(3, 3)
        l = self.getMiriList(mi)
        self.assert_(l == [])

    def test11_MarkerRangeRevIterator_06_corner(self): 
        l = self.getMiriList(0)
        self.assert_(l == [(mr_minus_inf, mr_plus_inf)])

    def test20_SetComplement_01(self):
        mi = newMi(4,10)
        
        in_range  = range(4,10) 
        out_range = [mr_minus_inf] + range(-10, 4) + range(10,20) + [mr_plus_inf]

        checkRanges(mi, in_range, out_range)

        mi_c = Mi_Complement(mi)

        checkRanges(mi_c, out_range, in_range)

        delMi(mi, mi_c)

    def test20_SetComplement_02(self):
        mi = newMi(4,10, 12, 20)

        in_range  = range(4,10) + range(12,20)
        out_range = ([mr_minus_inf] 
                     + range(-10, 4) + range(10,12) + range(20,30) 
                     + [mr_plus_inf])

        checkRanges(mi, in_range, out_range)
        mi_c = Mi_Complement(mi)
        checkRanges(mi_c, out_range, in_range)
        
        delMi(mi, mi_c)

    def test20_SetComplement_03(self):

        in_range = []
        out_range = [mr_minus_inf] + [mr_plus_inf]

        mi = newMi(0,0)

        for i in xrange(50):
            addMiRange(mi, i*10 + 3, i*10 + 7)
            in_range  += range(i*10 + 3, i*10 + 7)
            out_range += range(i*10, i*10 + 3)
            out_range  += range(i*10 + 7, i*10 + 10)

        checkRanges(mi, in_range, out_range)
        mi_c = Mi_Complement(mi)
        checkRanges(mi_c, out_range, in_range)
        
        delMi(mi, mi_c)

    # Set union operations

    def test21_SetUnion_01_overlap(self):
        checkSetOperation("union", *miSet_01_overlap()) 

    def test21_SetUnion_01_overlap(self):
        checkSetOperation("union", *miSet_01_overlap()) 

    def test21_SetUnion_02_nonoverlap(self):
        checkSetOperation("union", *miSet_02_nonoverlap()) 

    def test21_SetUnion_03_multiple(self):
        checkSetOperation("union", *miSet_03_multiple())

    def test21_SetUnion_04_multiple_disjoint(self):
        checkSetOperation("union", *miSet_04_multiple_disjoint()) 

    def test21_SetUnion_05_multiple(self):
        checkSetOperation("union", *miSet_05_multiple()) 

    def test21_SetUnion_06_large_01(self):
        checkSetOperation("union", *miSet_06_large(0)) 

    def test21_SetUnion_06_large_02(self):
        checkSetOperation("union", *miSet_06_large(1)) 

    def test21_SetUnion_06_large_03(self):
        checkSetOperation("union", *miSet_06_large(3)) 

    def test21_SetUnion_06_large_04(self):
        checkSetOperation("union", *miSet_06_large(5)) 

    def test21_SetUnion_06_large_05(self):
        checkSetOperation("union", *miSet_06_large(10)) 

    def test21_SetUnion_07_small_01(self):
        checkSetOperation("union", *miSet_07_large( (-10, 10), 5)) 

    def test21_SetUnion_07_medium_01(self):
        for i in range(2,50):
            checkSetOperation("union", *miSet_07_large( (-100, 100), i)) 

    def test21_SetUnion_07_large_01(self):
        for i in range(2,500,25):
            checkSetOperation("union", *miSet_07_large( (-1000, 1000), i)) 

    # Set intersection operations

    def test22_SetIntersection_01_overlap(self):
        checkSetOperation("intersection", *miSet_01_overlap()) 

    def test22_SetIntersection_02_nonoverlap(self):
        checkSetOperation("intersection", *miSet_02_nonoverlap()) 

    def test22_SetIntersection_03_multiple(self):
        checkSetOperation("intersection", *miSet_03_multiple()) 

    def test22_SetIntersection_04_multiple_disjoint(self):
        checkSetOperation("intersection", *miSet_04_multiple_disjoint()) 

    def test22_SetIntersection_05_multiple(self):
        checkSetOperation("intersection", *miSet_05_multiple()) 

    def test22_SetIntersection_06_large_01(self):
        checkSetOperation("intersection", *miSet_06_large(0)) 

    def test22_SetIntersection_06_large_02(self):
        checkSetOperation("intersection", *miSet_06_large(1)) 

    def test22_SetIntersection_06_large_03(self):
        checkSetOperation("intersection", *miSet_06_large(3)) 

    def test22_SetIntersection_06_large_04(self):
        checkSetOperation("intersection", *miSet_06_large(5)) 

    def test22_SetIntersection_06_large_05(self):
        checkSetOperation("intersection", *miSet_06_large(10)) 

    def test22_SetIntersection_07_small_01(self):
        checkSetOperation("intersection", *miSet_07_large( (-10, 10), 5)) 

    def test22_SetIntersection_07_medium_01(self):
        for i in range(2,50):
            checkSetOperation("intersection", *miSet_07_large( (-100, 100), i)) 

    def test22_SetIntersection_07_large_01(self):
        for i in range(2,500,25):
            checkSetOperation("intersection", *miSet_07_large( (-1000, 1000), i)) 

    # Set difference operations

    def test23_SetDifference_01_overlap(self):
        checkSetOperation("difference", *miSet_01_overlap()) 

    def test23_SetDifference_01_overlap(self):
        checkSetOperation("difference", *miSet_01_overlap()) 

    def test23_SetDifference_02_nonoverlap(self):
        checkSetOperation("difference", *miSet_02_nonoverlap()) 

    def test23_SetDifference_03_multiple(self):
        checkSetOperation("difference", *miSet_03_multiple()) 

    def test23_SetDifference_04_multiple_disjoint(self):
        checkSetOperation("difference", *miSet_04_multiple_disjoint()) 

    def test23_SetDifference_05_multiple(self):
        checkSetOperation("difference", *miSet_05_multiple()) 

    def test23_SetDifference_06_large_01(self):
        checkSetOperation("difference", *miSet_06_large(0)) 

    def test23_SetDifference_06_large_02(self):
        checkSetOperation("difference", *miSet_06_large(1)) 

    def test23_SetDifference_06_large_03(self):
        checkSetOperation("difference", *miSet_06_large(3)) 

    def test23_SetDifference_06_large_04(self):
        checkSetOperation("difference", *miSet_06_large(5)) 

    def test23_SetDifference_06_large_05(self):
        checkSetOperation("difference", *miSet_06_large(10)) 

    def test23_SetDifference_07_small_01(self):
        checkSetOperation("difference", *miSet_07_large( (-10, 10), 5)) 

    def test23_SetDifference_07_medium_01(self):
        for i in range(2,50):
            checkSetOperation("difference", *miSet_07_large( (-100, 100), i)) 

    def test23_SetDifference_07_large_01(self):
        for i in range(2,500,25):
            checkSetOperation("difference", *miSet_07_large( (-1000, 1000), i)) 

    # Set symmetricDifference operations

    def test24_SetSymmetricDifference_01_overlap(self):
        checkSetOperation("symmetricDifference", *miSet_01_overlap()) 

    def test24_SetSymmetricDifference_01_overlap(self):
        checkSetOperation("symmetricDifference", *miSet_01_overlap()) 

    def test24_SetSymmetricDifference_02_nonoverlap(self):
        checkSetOperation("symmetricDifference", *miSet_02_nonoverlap()) 

    def test24_SetSymmetricDifference_03_multiple(self):
        checkSetOperation("symmetricDifference", *miSet_03_multiple()) 

    def test24_SetSymmetricDifference_04_multiple_disjoint(self):
        checkSetOperation("symmetricDifference", *miSet_04_multiple_disjoint()) 

    def test24_SetSymmetricDifference_05_multiple(self):
        checkSetOperation("symmetricDifference", *miSet_05_multiple()) 

    def test24_SetSymmetricDifference_06_large_01(self):
        checkSetOperation("symmetricDifference", *miSet_06_large(0)) 

    def test24_SetSymmetricDifference_06_large_02(self):
        checkSetOperation("symmetricDifference", *miSet_06_large(1)) 

    def test24_SetSymmetricDifference_06_large_03(self):
        checkSetOperation("symmetricDifference", *miSet_06_large(3)) 

    def test24_SetSymmetricDifference_06_large_04(self):
        checkSetOperation("symmetricDifference", *miSet_06_large(5)) 

    def test24_SetSymmetricDifference_06_large_05(self):
        checkSetOperation("symmetricDifference", *miSet_06_large(10)) 
        
    def test24_SetSymmetricDifference_07_small_01(self):
        checkSetOperation("symmetricDifference", *miSet_07_large( (-10, 10), 5)) 

    def test24_SetSymmetricDifference_07_medium_01(self):
        for i in range(2,50):
            checkSetOperation("symmetricDifference", *miSet_07_large( (-100, 100), i)) 

    def test24_SetSymmetricDifference_07_large_01(self):
        for i in range(2,500,25):
            checkSetOperation("symmetricDifference", *miSet_07_large( (-1000, 1000), i)) 
        


if __name__ == '__main__':
    unittest.main()


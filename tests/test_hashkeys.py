#!/usr/bin/env python
import unittest

from common import *
from random import shuffle
from ctypes import *

hk_prime_offset = dgl.Hk_GetPrimeOffset()
hk_prime = (2**128 - hk_prime_offset)

import random as rn


def errMsg(s, r1, r2):
    s = ("(result:%s) %d != %d (true)" % (s, r1, r2)
         + "\nNote: r = 2^128 - %d \n      t = 2^128 - %d" % (2**128 - r1, 2**128 - r2)
         + "\n      r = %x\n      t = %x" % (r1, r2))
    return s


class TestHashKeys(unittest.TestCase):
    """
    Just a few hash keys tests that are more sanity checks then full
    tests.  
    """

    def checkHashSetUnique(self, v_iter, func):

        values = [v for v in v_iter]
        
        count = 0
        idx = range(len(values))

        lookup = {}
        
        for c in xrange(20):
            
            shuffle(idx)

            for v in [values[i] for i in idx]:

                h = makeHash(v, func)

                #print 'value = %s, h = %s' %(str(v), h)

                if v in lookup:
                    self.assert_(lookup[v] == h)
                else:
                    lookup[v] = h

        self.assert_(len(set(lookup.values())) == len(set(lookup.keys())))


    def testExtractHashKey(self):
        self.assert_(len(makeHash(0, 'FromInt')) == 32)
    
    def testDifferentHashKeys_int(self):
        self.checkHashSetUnique(range(-500, 500), 'FromInt')

    def testDifferentHashKeys_string(self):
        self.checkHashSetUnique(("n%d" % i for i in range(500)), 'FromString')

    def testDifferentHashKeys_UnsignedInt(self):
        self.checkHashSetUnique(range(500), 'FromUnsignedInt')

    def testUnsignedvsSigned(self):
        self.assert_(makeHash(c_long(-100), 'FromInt') != makeHash(c_long(-100), 'FromUnsignedInt'))
        self.assert_(makeHash(c_long(-100), 'FromInt') != makeHash(c_long(100), 'FromUnsignedInt'))
        self.assert_(makeHash(100, 'FromInt') == makeHash(100, 'FromUnsignedInt'))

    def testRefCounts(self):
        hk = dgl.NewHashObject()
        
        self.assert_(dgl.O_RefCount(hk) == 1, 'refcount %d != 1' % dgl.O_RefCount(hk))

        dgl.O_IncRef(hk)

        self.assert_(dgl.O_RefCount(hk) == 2, 'refcount %d != 2' % dgl.O_RefCount(hk))

        dgl.O_DecRef(hk)

        self.assert_(dgl.O_RefCount(hk) == 1, 'refcount %d != 1' % dgl.O_RefCount(hk))

    ############################################################
    # Hashes as numbers

    def checkHashNumber(self, n):
        h = numberToHashKey(n)
        m = hashKeyToNumber(h)

        self.assert_(m == n, "(retrieved) %d != %d (given)" % (m, n))

    def testHashAsNumber_01(self):
        self.checkHashNumber(1)
        
    def testHashAsNumber_02(self):
        self.checkHashNumber(2**32 + 1)
        
    def testHashAsNumber_03(self):
        self.checkHashNumber(2**64 + 1)

    def testHashAsNumber_04(self):
        self.checkHashNumber(2**96 + 1)
        
    def testHashAsNumber_05(self):
        self.checkHashNumber(231237893458118445745882889251064478507)

    ############################################################
    # REduce

    def checkReducedValues(self, n1, n2):
        
        h1 = numberToHashKey(n1)
        h2 = numberToHashKey(n2)

        n_true = (n1 + n2) % hk_prime

        n_res_1 = hashKeyToNumber(dgl.H_Reduce(0, h1, h2))

        self.assert_(n_res_1 == n_true, errMsg("1", n_res_1, n_true))

        n_res_2 = hashKeyToNumber(dgl.H_Reduce(0, h2, h1))

        self.assert_(n_res_2 == n_true, errMsg("1", n_res_1, n_true))

        decRef(h1, h2)

    def testReduce_01(self):
        self.checkReducedValues(1,1)
        
    def testReduce_02(self):
        self.checkReducedValues(1,0)

    def testReduce_04(self):
        self.checkReducedValues(0,hk_prime - 1)

    def testReduce_05(self):
        self.checkReducedValues(1,hk_prime - 1)
    
    def testReduce_06(self):
        self.checkReducedValues(hk_prime - 1, hk_prime - 1)

    def checkListOfReducePairs(self, l):
        for n, m in l:
            self.checkReducedValues(n,m)

    def testReduce_zeros_01(self):
        l = [(n, hk_prime - n) for n in range(1, 100)]
        self.checkListOfReducePairs(l)

    def testReduce_zeros_02(self):
        l = [(n, hk_prime - n) for n in range(2**64 - 500, 2**64 + 500)]
        self.checkListOfReducePairs(l)

    def testReduce_07(self):
        l = [(n, hk_prime - 1) for n in range(2**64 - 500, 2**64 + 500)]
        self.checkListOfReducePairs(l)
        
    def testReduce_08(self):
        l = [(n, n) for n in range(2**64 - 500, 2**64 + 500)]
        self.checkListOfReducePairs(l)

    def testReduce_09(self):
        l = [(n, n-1) for n in range(2**64 - 500, 2**64 + 500)]
        self.checkListOfReducePairs(l)

    def testReduce_09b(self):
        l = [(n+1, n-1) for n in range(2**64 - 500, 2**64 + 500)]
        self.checkListOfReducePairs(l)

    def testReduce_10(self):
        l = [(n, n-1) for n in range(2**64 - 500, 2**64 + 500)]
        self.checkListOfReducePairs(l)
    
    def testReduce_11(self):
        l = [((2**128 - 1) - (2**64 - 1), n) for n in range(0, 500)]
        self.checkListOfReducePairs(l)

    def testReduce_12(self):
        l = [((2**128 - 1) - (2**64 - 1), hk_prime - 1 - n) for n in range(0, 500)]
        self.checkListOfReducePairs(l)

    def testReduce_13(self):
        l = [((2**128 - 1) - (2**64 - 1), 2**64 + n) for n in range(0, 500)]
        self.checkListOfReducePairs(l)

    def testReduce_14(self):
        l = [((2**128 - 1) - (2**64 - 1), 2**64 - n) for n in range(0, 500)]
        self.checkListOfReducePairs(l)

    def testReduce_20(self):
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)

        self.assert_(not dgl.H_Equal(hk1, hk2))

        hkn1 = dgl.H_Reduce(0, hk1, hk2)
        hkn2 = dgl.H_Reduce(0, hk2, hk1)
        
        self.assert_(dgl.H_Equal(hkn1, hkn2))

    def testReduce_21_Multiple(self):
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)

        self.assert_(not dgl.H_Equal(hk1, hk2))

        hkn1 = dgl.H_Reduce(0, hk1, hk2)
        hkn2 = dgl.H_Reduce(0, hk1, hkn1)
        
        self.assert_(not dgl.H_Equal(hkn1, hkn2))
        self.assert_(not dgl.H_Equal(hk1, hkn1))
        self.assert_(not dgl.H_Equal(hk1, hkn2))
        self.assert_(not dgl.H_Equal(hk2, hkn1))
        self.assert_(not dgl.H_Equal(hk2, hkn2))

    def checkNegative(self, n):
        
        h = numberToHashKey(n)
        nh = dgl.H_Negative(0, h)

        n_true = (hk_prime - n) % hk_prime

        n_res = hashKeyToNumber(nh)

        rh = hashKeyToNumber(dgl.H_Reduce(0, nh, h))

        self.assert_(n_res == n_true, errMsg("1", n_res, n_true))
        self.assert_(rh == 0, errMsg("reduced_negative", rh, 0))

        decRef(h)

    def testNegative_01(self):
        self.checkNegative(0)

    def testNegative_02(self):
        self.checkNegative(1)

    def testNegative_03(self):
        self.checkNegative(hk_prime - 1)

    def checkNegativeList(self, l):
        for n in l:
            self.checkNegative(n)

    def testNegative_04(self):
        self.checkNegativeList(range(500, 0, -1))

    def testNegative_05(self):
        self.checkNegativeList(range(hk_prime - 50, hk_prime-1))

    def testNegative_06(self):
        self.checkNegativeList(range(2**64 - 500, 2**64 + 500))

    def testNegative_07(self):
        self.checkNegativeList(range(hk_prime - 100, hk_prime-1))

    def testNegative_08(self):
        self.checkNegativeList((2**128 - 1) - (2**64 - 1) + n for n in range(0, 100))

    def testNegative_09(self):
        self.checkNegativeList((2**128 - 1) - (2**64 - 1) - n for n in range(0, 100))

    def testRehash_01(self):
        h = numberToHashKey(0)
        h2 = dgl.H_Rehash(0, h)
        self.assert_(hashKeyToNumber(h2) == 0, "(true) 0 != %d (returned)" % (hashKeyToNumber(h2)))

    def testRehash_02(self):
        h = numberToHashKey(1)
        h2 = dgl.H_Rehash(0,h)
        self.assert_(hashKeyToNumber(h) != hashKeyToNumber(h2))
        self.assert_(hashKeyToNumber(h) != 0)

    def checkReduceList(self, l):
        
        def reduceHashes(*l):

            # First get it through the reduce function
            rh = dgl.H_Reduce(0, l[0], l[1])
            
            for h in l[2:]:
                dgl.H_ReduceUpdate(rh, h)
                
            ret_n = hashKeyToNumber(rh)

            # Now get it through our own analysis

            true_n = sum(hashKeyToNumber(h) for h in l) % hk_prime

            self.assert_(ret_n == true_n, errMsg("Reduced key mismatch: ", ret_n, true_n))

            return ret_n

        hl  = [makeHashKey(e) for e in l]
        nhl = [dgl.H_Negative(0, h) for h in hl]

        for i in xrange(len(hl)):

            r_hashes = hl[:i] + hl[i+1:] + nhl[:i] + nhl[i+1:] + [hl[i]]

            for attempt in xrange(min(2*len(r_hashes), 5)):

                rn.shuffle(r_hashes)

                ret_n = reduceHashes(*r_hashes)
                
                self.assert_(ret_n == hashKeyToNumber(hl[i]), 
                             errMsg("Negatives not working: ", ret_n, hashKeyToNumber(hl[i]))) 


    def testNegativeInReduce_01(self):
        self.checkReduceList([0,1])

    def testNegativeInReduce_02(self):
        self.checkReduceList(range(10))

    def testNegativeInReduce_03(self):
        self.checkReduceList(range(100))

    ############################################################
    # Combine

    def testCombine(self):
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)

        self.assert_(not dgl.H_Equal(hk1, hk2))

        hkn1 = dgl.Hf_Combine(0, hk1, hk2)
        hkn2 = dgl.Hf_Combine(0, hk2, hk1)
        
        self.assert_(not dgl.H_Equal(hkn1, hkn2))
        self.assert_(not dgl.H_Equal(hk1, hkn1))
        self.assert_(not dgl.H_Equal(hk1, hkn2))
        self.assert_(not dgl.H_Equal(hk2, hkn1))
        self.assert_(not dgl.H_Equal(hk2, hkn2))
        
    def testCombineMultiple(self):
        hk1 = makeHashKey(0)
        hk2 = makeHashKey(1)
        hk3 = makeHashKey(2)

        self.assert_(not dgl.H_Equal(hk1, hk2))

        hkn1 = dgl.Hf_Combine(0, hk1, hk2)
        hkn2 = dgl.Hf_Combine(0, hk1, hk3)
        
        self.assert_(not dgl.H_Equal(hkn1, hkn2))
        self.assert_(not dgl.H_Equal(hk1, hkn1))
        self.assert_(not dgl.H_Equal(hk1, hkn2))
        self.assert_(not dgl.H_Equal(hk2, hkn1))
        self.assert_(not dgl.H_Equal(hk2, hkn2))

####### Marker stuff

    def testMarker_01_BasicOperations(self):
        hk1 = makeHashKey(0)

        # All should be valid 
        
        self.assert_(not dgl.H_IsMarked(hk1))

        self.assert_(dgl.H_MarkerPointIsValid(hk1, -1))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 0))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 5))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 10))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 15))

        dgl.H_AddMarkerValidRange(hk1, 0, 10)

        self.assert_(dgl.H_IsMarked(hk1))
        
        self.assert_(not dgl.H_MarkerPointIsValid(hk1, -1))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 0))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 5))
        self.assert_(not dgl.H_MarkerPointIsValid(hk1, 10))
        self.assert_(not dgl.H_MarkerPointIsValid(hk1, 15))
        
        dgl.H_ClearMarkerInfo(hk1)

        self.assert_(not dgl.H_IsMarked(hk1))

        self.assert_(dgl.H_MarkerPointIsValid(hk1, -1))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 0))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 5))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 10))
        self.assert_(dgl.H_MarkerPointIsValid(hk1, 15))
        
if __name__ == '__main__':
    unittest.main()


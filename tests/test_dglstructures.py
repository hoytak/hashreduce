#!/usr/bin/env python
"""
Tests the basics dgl routines.
"""

import unittest

from common import *
from dglcreation import *    
from parsers import *
from random import shuffle
from pprint import pprint

class TestDGLLabelReference(unittest.TestCase):

    def checkConsistent(self, g, f, d):
        l = [e for e in d]
        idx = range(len(l))
        
        h_d = {}

        for i in xrange(25):
            shuffle(idx)

            for i in idx:
                
                h = f(g, l[i])

                if l[i] in h_d:
                    self.assert_(h_d[l[i]] == h)
                else:
                    h_d[l[i]] = h

        #pprint(h_d)

        self.assert_(len(set(h_d.values())) == len(h_d))

    def test_01_Node_Number_Single(self):
        g = newDGLGraph()
        self.assert_(getNode(g, 0) == getNode(g, 0))

    def test_02_Node_Name_Single(self):
        g = newDGLGraph()
        self.assert_(getNode(g, "n") == getNode(g, "n"))

    def test_03_Edge_Number_Single(self):
        g = newDGLGraph()
        self.assert_(getEdge(g, 0) == getEdge(g, 0))

    def test_04_Edge_Name_Single(self):
        g = newDGLGraph()
        self.assert_(getEdge(g, "n") == getEdge(g, "n"))

    def test_05_Node_Number_Double(self):
        g = newDGLGraph()
        n0 = getNode(g, 0)
        n1 = getNode(g, 1)

        for i in xrange(5):
            self.assert_(getNode(g, 0) == getNode(g, 0) == n0)
            self.assert_(getNode(g, 1) == getNode(g, 1) == n1)

    def test_06_Node_Name_Double(self):
        g = newDGLGraph()
        n0 = getNode(g, "n0")
        n1 = getNode(g, "n1")

        for i in xrange(5):
            self.assert_(getNode(g, "n0") == getNode(g, "n0") == n0)
            self.assert_(getNode(g, "n1") == getNode(g, "n1") == n1)

    def test_07_Edge_Number_Double(self):
        g = newDGLGraph()
        n0 = getEdge(g, 0)
        n1 = getEdge(g, 1)

        for i in xrange(5):
            self.assert_(getEdge(g, 0) == getEdge(g, 0) == n0)
            self.assert_(getEdge(g, 1) == getEdge(g, 1) == n1)

    def test_08_Edge_Name_Double(self):
        g = newDGLGraph()
        n0 = getEdge(g, "e0")
        n1 = getEdge(g, "e1")

        for i in xrange(5):
            self.assert_(getEdge(g, "e0") == getEdge(g, "e0") == n0)
            self.assert_(getEdge(g, "e1") == getEdge(g, "e1") == n1)
        
    def test_11_Node_Number(self):
        g = newDGLGraph()
        self.checkConsistent(g, getNode, range(-10,10))
        delDGL(g)

    def test_12_Node_Name(self):
        g = newDGLGraph()
        self.checkConsistent(g, getNode, [("n%d" % n) for n in range(-10,10)])
        delDGL(g)

    def test_13_Node_Mix(self):
        g = newDGLGraph()
        self.checkConsistent(g, getNode, [("n%d" % n) for n in range(-10,10)] + range(-10,10))
        delDGL(g)

    def test_14_Edge_Number(self):
        g = newDGLGraph()
        self.checkConsistent(g, getEdge, range(-10,10))
        delDGL(g)

    def test_15_Edge_Name(self):
        g = newDGLGraph()
        self.checkConsistent(g, getEdge, [("n%d" % n) for n in range(-10,10)])
        delDGL(g)

    def test_16_Edge_Mix(self):
        g = newDGLGraph()
        self.checkConsistent(g, getEdge, [("n%d" % n) for n in range(-10,10)] + range(-10,10))
        delDGL(g)
        



class TestDGLComparisons(unittest.TestCase):
    
    def checkDGLComparison(self, d1, d2, equality_set, inequality_set):
        
        wrongly_inequal = [m for m in equality_set 
                           if not dglSameAtMarker(d1, d2, m)]
        wrongly_equal = [m for m in inequality_set 
                         if dglSameAtMarker(d1, d2, m)]

        assert len(wrongly_equal) == 0 and len(wrongly_inequal) == 0,\
            "Wrongly Equal: %s; Wrongly Inequal: %s" \
            % (",".join(str(m) for m in sorted(wrongly_equal)), 
               ",".join(str(m) for m in sorted(wrongly_inequal)))


    def test_01_2NodeSanity(self):
        
        d1 = createDGLGraph([("e", "n1", []),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [])])

        self.checkDGLComparison(d1, d2, range(10), [])

    def test_02_2NodeSanity_FullCompare(self):
        
        d1 = createDGLGraph([("e", "n1", []),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [])])

        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_03_3NodeSanity(self):
        
        d1 = createDGLGraph([("e", "n1", [("n3", 2)]),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n1", [("n3", 2)]),
                             ("e", "n2", [])])

        self.checkDGLComparison(d1, d2, range(10), [])

    def test_04_3NodeSanity_FullCompare(self):
        
        d1 = createDGLGraph([("e", "n1", [("n3", 2)]),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n1", [("n3", 2)]),
                             ("e", "n2", [])])

        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_05_3NodeSanity_rev(self):
        
        d1 = createDGLGraph([("e", "n1", [("n3", 2)]),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [("n3", 2)])])

        self.checkDGLComparison(d1, d2, range(10), [])

    def test_06_3NodeSanity_rev_FullCompare(self):
        
        d1 = createDGLGraph([("e", "n1", [("n3", 2)]),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [("n3", 2)])])

        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_07_3Node_Switch(self):
        
        d1 = createDGLGraph([("e", "n1", [("n3", 2)]), 
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n1", [("n3", 4)]),
                             ("e", "n2", [])])

        self.checkDGLComparison(d1, d2, [0,1,2,3,4,5,6,7,8,9,10], [])
        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_07_3Node_Switch_Simplest(self):
        
        d1 = createDGLGraph([("e", "n1", [("n3", 2)])])
        d2 = createDGLGraph([("e", "n1", [("n3", 4)])])

        self.checkDGLComparison(d1, d2, [0,1,2,3,4,5,6,7,8,9,10], [])
        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_07_3Node_Switch_rev(self):
        
        d1 = createDGLGraph([("e", "n1", [("n3", 2)]),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [("n3", 4)])])

        self.checkDGLComparison(d1, d2, [0,1,2,3,4,5,6,7,8,9,10], [])
        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_08_1Node_Continuity(self):
        d1 = createDGLGraph([("e", "n1", [("n1", 4)])])
        d2 = createDGLGraph([("e", "n2", [])])
        
        self.checkDGLComparison(d1, d2, range(-10,10), [])

    def test_09_1Node_Continuity_FullCompare(self):
        d1 = createDGLGraph([("e", "n1", [("n1", 4)])])
        d2 = createDGLGraph([("e", "n2", [])])
        
        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_10_2Node_2Edge(self):

        d1 = createDGLGraph([("e1", "n1", [])])

        d2 = createDGLGraph([("e2", "n1", [])])
        
        self.checkDGLComparison(d1, d2, [], [0,1,2,3,4,5,6,7,8,9,10])
        self.assert_(not dgl.DGLGraphEqual(d1, d2))

    def test_10_4Node_Equal(self):
        
        d1 = createDGLGraph([("e1", "n1", [("n2", 2), ("n4", 4)]),
                             ("e1", "n2", [("n3", 2), ("n3", 4)]),
                             ("e2", "n4", [("n1", 2), ("n2", 4)]),
                             ("e2", "n3", [("n4", 2), ("n1", 4)])])

        # Equal, just switched ordering
        d2 = createDGLGraph([("e2", "n3", [("n4", 2), ("n1", 4)]),
                             ("e1", "n1", [("n2", 2), ("n4", 4)]),
                             ("e2", "n4", [("n1", 2), ("n2", 4)]),
                             ("e1", "n2", [("n3", 2), ("n3", 4)])])
        
        self.checkDGLComparison(d1, d2, [0,1,2,3,4,5,6,7,8,9,10], [])
        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_10_4Node_Switch(self):
        
        d1 = createDGLGraph([("e1", "n1", []),
                             ("e1", "n2", [("n3", 2), ("n2", 4)]),
                             ("e2", "n3", [("n2", 2), ("n3", 4)]),
                             ("e2", "n4", [])])

        d2 = createDGLGraph([("e1", "n1", []),
                             ("e1", "n2", []),
                             ("e2", "n3", []),
                             ("e2", "n4", [])])

        self.checkDGLComparison(d1, d2, [0,1,2,3,4,5,6,7,8,9,10], [])
        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_20_Corner_01(self):
        
        d1 = createDGLGraph([("e", "n1", [("n1", 4)]),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [])])


        self.checkDGLComparison(d1, d2, range(10), [])
        self.assert_(dgl.DGLGraphEqual(d1, d2))

    def test_20_Corner_02_FullCompare(self):
        
        d1 = createDGLGraph([("e", "n1", [("n1", 4)]),
                             ("e", "n2", [])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [])])

        self.assert_(dgl.DGLGraphEqual(d1, d2))
        self.checkDGLComparison(d1, d2, range(10), [])

    def test_20_Corner_03_switch(self):
        
        d1 = createDGLGraph([("e", "n1", [("n2", 4)]),
                             ("e", "n2", [("n1", 4)])])

        d2 = createDGLGraph([("e", "n2", []),
                             ("e", "n1", [])])

        self.assert_(dgl.DGLGraphEqual(d1, d2))
        self.checkDGLComparison(d1, d2, range(10), [])

################################################################################
# More detailed examples, in which parsing is also tested.

    def testP01_Identical(self):

        s = """
            408 9 9 0
            408 10 10 0
            4080 11 11 0
            4080 12 12 0
            513 10 9 0
            513 12 12 0
            514 10 10 0
            514 12 12 1 2 11 
            515 9 9 0
            515 11 12 0
            516 9 10 0
            516 12 12 0

            408 9 9 0
            408 10 10 0
            4080 11 11 0
            4080 12 12 0
            513 10 10 0
            513 11 11 0
            514 9 9 0
            514 12 12 0
            515 9 10 0
            515 12 11 0
            516 9 9 0
            516 11 11 0

            408 9 9 0
            408 10 10 0
            4080 11 11 0
            4080 12 12 0
            513 10 9 0
            513 12 12 0
            514 10 10 0
            514 12 12 1 2 11 
            515 9 9 0
            515 11 12 0
            516 9 10 0
            516 12 12 0

            408 9 9 0
            408 10 10 0
            4080 11 11 0
            4080 12 12 0
            513 10 10 0
            513 11 11 0
            514 9 9 0
            514 12 12 0
            515 9 10 0
            515 12 11 0
            516 9 9 0
            516 11 11 0
            """

        dl = parse_F1_string(s)

        self.assert_(dgl.DGLGraphEqual(dl[0], dl[2]))
        self.assert_(dgl.DGLGraphEqual(dl[1], dl[3]))
        self.assert_(not dgl.DGLGraphEqual(dl[0], dl[1]))
 
        self.assert_(dgl.DGLGraphEqualAtMarker(dl[0], dl[2], 0))
        self.assert_(dgl.DGLGraphEqualAtMarker(dl[1], dl[3], 0))
        self.assert_(not dgl.DGLGraphEqualAtMarker(dl[0], dl[1], 0))

        self.assert_(dgl.DGLGraphEqualAtMarker(dl[0], dl[2], 1))
        self.assert_(dgl.DGLGraphEqualAtMarker(dl[1], dl[3], 1))
        self.assert_(not dgl.DGLGraphEqualAtMarker(dl[0], dl[1], 1))

       
    def testP02_Identical(self):

        s = """
            408 9 9 0
            408 10 10 0
            4080 11 11 0
            4080 12 12 0
            513 10 9 0
            513 12 12 0
            514 10 10 0
            514 12 12 1 2 11 
            515 9 9 0
            515 11 12 0
            516 9 10 0
            516 12 12 0

            408 9 9 0
            408 10 10 0
            4080 11 11 0
            4080 12 12 0
            513 10 10 0
            513 11 11 0
            514 9 9 0
            514 12 12 0
            515 9 10 0
            515 12 11 0
            516 9 9 0
            516 11 11 0

            408 9 9 0
            408 10 10 0
            4080 400 400 0
            4080 500 500 0
            513 10 9 0
            513 500 500 0
            514 10 10 0
            514 500 500 1 2 400 
            515 9 9 0
            515 400 500 0
            516 9 10 0
            516 500 500 0

            408 9 9 0
            408 10 10 0
            4080 400 400 0
            4080 500 500 0
            513 10 10 0
            513 400 400 0
            514 9 9 0
            514 500 500 0
            515 9 10 0
            515 500 400 0
            516 9 9 0
            516 400 400 0
            """

        dl = parse_F1_string(s)

        self.assert_(dgl.DGLGraphEqual(dl[0], dl[2]))
        self.assert_(dgl.DGLGraphEqual(dl[1], dl[3]))
        self.assert_(not dgl.DGLGraphEqual(dl[0], dl[1]))

        self.assert_(dgl.DGLGraphEqualAtMarker(dl[0], dl[2], 0))
        self.assert_(dgl.DGLGraphEqualAtMarker(dl[1], dl[3], 0))
        self.assert_(not dgl.DGLGraphEqualAtMarker(dl[0], dl[1], 0))

        self.assert_(dgl.DGLGraphEqualAtMarker(dl[0], dl[2], 1))
        self.assert_(dgl.DGLGraphEqualAtMarker(dl[1], dl[3], 1))
        self.assert_(not dgl.DGLGraphEqualAtMarker(dl[0], dl[1], 1))


    def testFileDupCount(self):
        pass
        #dl = parse_F1_file('tests/datafiles/test01.dglf1')
        #displayDuplicationCount(dl)

if __name__ == '__main__':
    unittest.main()


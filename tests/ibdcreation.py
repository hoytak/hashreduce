"""
Routines for creating the various files. 
"""

from common import *

newIBDGraph = ibd.NewIBDGraph
newIBDGraph.restype = ctypes.c_void_p

ibd.IBDGraphEdgeByName.restype = ctypes.c_void_p
ibd.IBDGraphEdgeByNumber.restype = ctypes.c_void_p
ibd.IBDGraphNodeByName.restype = ctypes.c_void_p
ibd.IBDGraphNodeByNumber.restype = ctypes.c_void_p
ibd.IBDGraphViewHash.restype = ctypes.c_void_p
ibd.IBDGraphGetHashAtMarker.restype = ctypes.c_void_p

mr_plus_infinity = ibd.Mr_Plus_Infinity()

mr_graph_max = mr_plus_infinity
mr_graph_min = 0

ibdSameAtMarker = ibd.IBDGraphEqualAtMarker

def delIBD(*args):
    for a in args:
        ibd.O_DecRef(a)

def getNode(g, name):
    if type(name) is str:
        return ibd.IBDGraphNodeByName(g, name)
    elif type(name) is int:
        return ibd.IBDGraphNodeByNumber(g, name)
    else:
        raise TypeError("Type of node '%s' must be either string or int" % str(name))

def getEdge(g, name):
    if type(name) is str:
        return ibd.IBDGraphEdgeByName(g, name)
    elif type(name) is int:
        return ibd.IBDGraphEdgeByNumber(g, name)
    else:
        raise TypeError("Type of edge '%s' must be either string or int" % str(name))
    

def addToGraph(g, edge_name, base_node, changes):
    # Adds an edge, with one set of connections, to the graph

    edge = getEdge(g,edge_name)
    cur_node = getNode(g,base_node)

    cur_range_min = mr_graph_min

    for n, m in changes:
        #print "@@@@@@@@@@@@ Adding node %s at range [%d, %d)" % (extractHash(cur_node), cur_range_min, m)
        ibd.IBDGraph_Connect(g, edge, cur_node, cur_range_min, m)

        cur_range_min = m
        cur_node = getNode(g,n)

    #print "@@@@@@@@@@@@ Adding node %s at range [%d, %d)" % (extractHash(cur_node), cur_range_min, mr_graph_max)
    ibd.IBDGraph_Connect(g, edge, cur_node, cur_range_min, mr_graph_max)

def createIBDGraph(connections):
    """
    Connections is a list of 3-tuples of (edge, base_node,
    change_list).
    """

    g = newIBDGraph()
    
    for e, bn, cl in connections:
        addToGraph(g, e, bn, cl)

    return g

def duplicateReportDict(ibd_list, marker_loc):

    count = {}
    
    for idx, g in enumerate(ibd_list):
        if marker_loc is None:
            h = extractHash(ibd.IBDGraphViewHash(g))
        else:
            h1 = ibd.IBDGraphGetHashAtMarker(g, marker_loc)
            h = extractHash(h1)
            ibd.O_DecRef(h1)

        if h in count:
            count[h][0] += 1
            count[h][1].append(idx+1)
        else:
            count[h] = [1, [idx+1]]

    return count

def displayDuplicationCount(ibd_list, marker_loc = None):
    count = duplicateReportDict(ibd_list, marker_loc)
    
    print "\nCount : Entries"

    for (c, v) in sorted(count.itervalues()):
        c_s = str(c)
        v_s = ", ".join(str(i) for i in v)

        print  c_s + " "*(6 - len(c_s)) + ": " + v_s




"""
Routines for creating the various files. 
"""

from common import *

newDGLGraph = dgl.NewDGLGraph
newDGLGraph.restype = ctypes.c_void_p

dgl.DGLGraphEdgeByName.restype = ctypes.c_void_p
dgl.DGLGraphEdgeByNumber.restype = ctypes.c_void_p
dgl.DGLGraphNodeByName.restype = ctypes.c_void_p
dgl.DGLGraphNodeByNumber.restype = ctypes.c_void_p
dgl.DGLGraphViewHash.restype = ctypes.c_void_p
dgl.DGLGraphGetHashAtMarker.restype = ctypes.c_void_p

mr_plus_infinity = dgl.Mr_Plus_Infinity()

mr_graph_max = mr_plus_infinity
mr_graph_min = 0

dglSameAtMarker = dgl.DGLGraphEqualAtMarker

def delDGL(*args):
    for a in args:
        dgl.O_DecRef(a)

def getNode(g, name):
    if type(name) is str:
        return dgl.DGLGraphNodeByName(g, name)
    elif type(name) is int:
        return dgl.DGLGraphNodeByNumber(g, name)
    else:
        raise TypeError("Type of node '%s' must be either string or int" % str(name))

def getEdge(g, name):
    if type(name) is str:
        return dgl.DGLGraphEdgeByName(g, name)
    elif type(name) is int:
        return dgl.DGLGraphEdgeByNumber(g, name)
    else:
        raise TypeError("Type of edge '%s' must be either string or int" % str(name))
    

def addToGraph(g, edge_name, base_node, changes):
    # Adds an edge, with one set of connections, to the graph

    edge = getEdge(g,edge_name)
    cur_node = getNode(g,base_node)

    cur_range_min = mr_graph_min

    for n, m in changes:
        #print "@@@@@@@@@@@@ Adding node %s at range [%d, %d)" % (extractHash(cur_node), cur_range_min, m)
        dgl.DGLGraph_Connect(g, edge, cur_node, cur_range_min, m)

        cur_range_min = m
        cur_node = getNode(g,n)

    #print "@@@@@@@@@@@@ Adding node %s at range [%d, %d)" % (extractHash(cur_node), cur_range_min, mr_graph_max)
    dgl.DGLGraph_Connect(g, edge, cur_node, cur_range_min, mr_graph_max)

def createDGLGraph(connections):
    """
    Connections is a list of 3-tuples of (edge, base_node,
    change_list).
    """

    g = newDGLGraph()
    
    for e, bn, cl in connections:
        addToGraph(g, e, bn, cl)

    return g

def duplicateReportDict(dgl_list, marker_loc):

    count = {}
    
    for idx, g in enumerate(dgl_list):
        if marker_loc is None:
            h = extractHash(dgl.DGLGraphViewHash(g))
        else:
            h1 = dgl.DGLGraphGetHashAtMarker(g, marker_loc)
            h = extractHash(h1)
            dgl.O_DecRef(h1)

        if h in count:
            count[h][0] += 1
            count[h][1].append(idx+1)
        else:
            count[h] = [1, [idx+1]]

    return count

def displayDuplicationCount(dgl_list, marker_loc = None):
    count = duplicateReportDict(dgl_list, marker_loc)
    
    print "\nCount : Entries"

    for (c, v) in sorted(count.itervalues()):
        c_s = str(c)
        v_s = ", ".join(str(i) for i in v)

        print  c_s + " "*(6 - len(c_s)) + ": " + v_s




#!/usr/bin/env python

# Change this line to include the proper path for the compiled .so
# file.
cdef extern from "ibd_fatpack.c":
    ctypedef void IBDGraph
    ctypedef void IBDGraphNode
    ctypedef void IBDGraphEdge
    ctypedef long markertype
    ctypedef void HashObject

    IBDGraph* NewIBDGraph(long id)
    IBDGraphNode* IBDGraphNodeByName(IBDGraph *g, char *name)
    IBDGraphNode* IBDGraphNodeByNumber(IBDGraph *g, long number)

    IBDGraphEdge* IBDGraphEdgeByName(IBDGraph *g, char *name)
    IBDGraphEdge* IBDGraphEdgeByNumber(IBDGraph *g, long number)

    void IBDGraph_Connect(IBDGraph *g, IBDGraphEdge *e, IBDGraphNode *n, 
                          markertype range_start, markertype range_end)

    bint IBDGraphEqualAtMarker(IBDGraph *g1, IBDGraph *g2, markertype m)
    bint IBDGraphEqual(IBDGraph *g1, IBDGraph *g2)

    HashObject* IBDGraphViewHash(IBDGraph *g)
    HashObject* IBDGraphGetHashAtMarker(IBDGraph *g, markertype m)
    char *H_HashAs8ByteString(void*)

    long _Mr_Plus_Infinity()

    void O_DECREF(void*)
    void O_INCREF(void*)

cdef extern from "Python.h":
    str PyString_FromStringAndSize(char *v, int len)

DEF mr_graph_min = 0

from optparse import *
import sys
from itertools import *
import re

cdef bint quiet_mode = False

cdef dict global_index_lookup = {}

cdef int getHashIndex(name):
    try:
        return global_index_lookup[name]
    except KeyError:
        pass

    cdef int index = len(global_index_lookup)
    global_index_lookup[name] = index
    return index

cdef class NodeChange:
    cdef int name
    cdef markertype m

cdef str extractHash(HashObject* h):
    return PyString_FromStringAndSize(H_HashAs8ByteString(h), 8)

cdef class EdgeDef:
    cdef int edge_name
    cdef int base_node
    cdef list changes

cdef class IBDGraphWrapper:
    cdef IBDGraph *g

    def __init__(self, long id):
        self.g = NewIBDGraph(id)

    cdef IBDGraphEdge* _getEdge(self, int name):
        return IBDGraphEdgeByNumber(self.g, name)

    cdef IBDGraphNode* _getNode(self, int name):
        return IBDGraphNodeByNumber(self.g, name)

    cdef add(self, EdgeDef e):

        cdef IBDGraphEdge *edge = NULL
        cdef IBDGraphNode *cur_node = NULL
        cdef markertype cur_range_min
        cdef NodeChange nc

        edge = self._getEdge(e.edge_name)
        cur_node = self._getNode(e.base_node)

        cur_range_min = mr_graph_min

        for nc in e.changes:
            IBDGraph_Connect(self.g, edge, cur_node, cur_range_min, nc.m)
            cur_range_min = nc.m
            cur_node = self._getNode(nc.name)

        IBDGraph_Connect(self.g, edge, cur_node, cur_range_min, _Mr_Plus_Infinity())

    cdef str getHash(self):
        cdef HashObject *h = IBDGraphViewHash(self.g)
        return extractHash(h)

    cdef str getHashAtMarker(self, markertype m):
        cdef HashObject *h = IBDGraphGetHashAtMarker(self.g, m)
        cdef str s = extractHash(h)
        O_DECREF(h)
        return s


cdef IBDGraphWrapper createIBDGraph(long id, list ed_l1, list ed_l2):
    """
    Connections is a list of 3-tuples of (edge, base_node,
    change_list).
    """

    cdef IBDGraphWrapper g = IBDGraphWrapper(id)

    cdef size_t i

    for ed in ed_l1:
        g.add(<EdgeDef?>ed)

    for ed in ed_l2:
        g.add(<EdgeDef?>ed)

    return g

################################################################################
# The file parsing
cdef object line_parse = re.compile(
    r"\s*(?P<edge>\w+)\s+(?P<ibd_0>\w+)\s+(?P<ibd_1>\w+)\s+(?P<change_count>\d+)")

cdef object change_parse = re.compile(
    r"\s*(?P<change_pos>\d+)\s+(?P<ibd>\w+)\s*")

cdef EdgeDef parse_F1_line(str l, size_t pos):

    cdef list changes
    cdef unsigned int change_count, i
    cdef unsigned int l_pos = 0

    cdef EdgeDef ed = EdgeDef()
    cdef NodeChange nc
    
    try:
        m = line_parse.match(l)

        if m is None:
            raise Exception("Parse Error; pattern not recognized (line = %s)" % l)

        mgroup = m.group

        change_count = int(mgroup('change_count'))
        ed.changes = [None]*(change_count + 1)

        # Set the base stuff
        ed.edge_name = getHashIndex(mgroup('edge'))
        ed.base_node = getHashIndex(mgroup('ibd_0'))

        nc = NodeChange()
        nc.name, nc.m = getHashIndex(mgroup('ibd_1') ), 1
        ed.changes[0] = nc
        
        l_pos += m.end()

        for i in range(change_count):
            
            m = change_parse.match(l[l_pos:])

            assert m is not None

            l_pos += m.end()
            
            mgroup = m.group

            nc = NodeChange()
            nc.name, nc.m = getHashIndex(mgroup('ibd')), int(mgroup('change_pos'))
            ed.changes[i+1] = nc
                           
        return ed

    except Exception, e:
        raise Exception("Parse error on line %d: %s" % (pos, str(e)))

cdef set _nextEdgeNameSet(list edge_list, size_t start_pos):
    cdef set seen_edges = set()

    for edge in edge_list[start_pos:]:
        if edge in seen_edges:
            break
        else:
            seen_edges.add(edge)

    return seen_edges
    

cdef list parse_F1_string(str string):

    cdef list lines = [ll.strip() for ll in string.split('\n')]
    cdef size_t pos

    cdef list parsed_lines = [
        parse_F1_line( (<str>l), pos+1) 
        for pos, l in enumerate(lines)
        if ( (<str>l) and not (<str>l).startswith("#"))]

    # Process these all to allow indexing by edge
    cdef size_t n_entries = len(parsed_lines) // 2
    
    cdef list edge_list = [None]*n_entries
    cdef list ed_l1 = [None]*n_entries
    cdef list ed_l2 = [None]*n_entries

    cdef EdgeDef ed1, ed2

    for pos in range(n_entries):
        
        ed1 = <EdgeDef>parsed_lines[2*pos]
        ed2 = <EdgeDef>parsed_lines[2*pos + 1]
        
        if ed1.edge_name != ed2.edge_name:
            raise Exception("Edges near lines %d don't match." % (2*pos))

        edge_list[pos] = ed1.edge_name
        ed_l1[pos] = ed1
        ed_l2[pos] = ed2

    # Now parse the file
    cdef set working_set = None
    cdef size_t cur_pos = 0
    cdef size_t step
    cdef list ibd_graphs = []
    cdef set next_set

    while True:
        next_set = _nextEdgeNameSet(edge_list, cur_pos)
        
        if len(next_set) == 0:
            break

        if working_set != next_set:
            working_set = next_set

        step = len(working_set)

        ibd_graphs.append(createIBDGraph(len(ibd_graphs),
                                         ed_l1[cur_pos:cur_pos+step],
                                         ed_l2[cur_pos:cur_pos+step]))

        cur_pos += step

    if not quiet_mode:
        print "Loaded %d graphs." % len(ibd_graphs)

    return ibd_graphs

cdef list parse_F1_file(filename):
    f = open(filename)
    ibd_list = parse_F1_string(f.read())
    f.close()
    
    return ibd_list

################################################################################
# Displaying the results

cdef class GraphDupInfo:
    cdef size_t count
    cdef list graph_indices

    def __richcmp__(GraphDupInfo gdi1, GraphDupInfo gdi2, int t):
        if t == 0:
            return gdi_lt(gdi1, gdi2)
        elif t == 1:
            return 
        elif t == 2:
            return gdi_eq(gdi1, gdi2)
        elif t == 3:
            return not gdi_eq(gdi1, gdi2)
        elif t == 4:
            return not gdi_lteq(gdi1, gdi2)
        elif t == 5:
            return not gdi_lt(gdi1, gdi2)
        else:
            assert False

cdef gdi_lt(GraphDupInfo gdi1, GraphDupInfo gdi2):

    if gdi1.count == gdi2.count:
        return gdi1.graph_indices[0] < gdi2.graph_indices[0]
    else:
        return gdi1.count < gdi2.count

cdef gdi_lteq(GraphDupInfo gdi1, GraphDupInfo gdi2):

    if gdi1.count == gdi2.count:
        return gdi1.graph_indices[0] <= gdi2.graph_indices[0]
    else:
        return gdi1.count <= gdi2.count

cdef gdi_eq(GraphDupInfo gdi1, GraphDupInfo gdi2):
    return gdi1.count == gdi2.count and gdi1.graph_indices[0] == graph_indices[1]

cdef dict duplicateReportDict(list ibd_list, marker_loc_request):

    cdef IBDGraphWrapper g
    cdef size_t idx

    cdef dict count = {}
    cdef markertype marker_loc
    cdef bint complete_graph

    if marker_loc_request is None:
        complete_graph = True
    else:
        complete_graph = False
        marker_loc = marker_loc_request

    cdef GraphDupInfo gdi
        
    for idx, g in enumerate(ibd_list):
        
        if complete_graph:
            h = g.getHash()
        else:
            h = g.getHashAtMarker(marker_loc)

        try:
            gdi = <GraphDupInfo>count[h]
            gdi.count += 1
            gdi.graph_indices.append(idx+1)
            
        except KeyError:
            gdi = GraphDupInfo()
            gdi.count = 1
            gdi.graph_indices = [idx+1]

            count[h] = gdi

    return count

cdef displayDuplicationCount(list ibd_list, marker_loc):
    
    cdef dict count = duplicateReportDict(ibd_list, marker_loc)

    if not quiet_mode:
        print "Count : Entries"

    cdef GraphDupInfo gdi
    cdef list report_values = count.values()

    report_values.sort()

    for gdi in report_values:
        c_s = str(gdi.count)
        v_s = ", ".join([str(i) for i in gdi.graph_indices])

        print  c_s + " "*(6 - len(c_s)) + ": " + v_s


if __name__ == '__main__':

    usage = '\n%s [options] <input file>' % sys.argv[0]

    description = "Bins IBD graphs." 

    parser = OptionParser(usage=usage, description=description)
    
    parser.add_option('-m', '--at-marker', dest='marker_loc', default=None,
                      help="Do the binning at a specified marker position.")

    parser.add_option('-q', '--quiet', dest='quiet_mode', default=False, action='store_true',
                      help="Quite mode; only print out the final equivalence classes.")

    options, args = parser.parse_args()

    # See what's going on...
    def print_use_help():
        print "Usage: %s\n" % usage
        print "\nUse --help to get a list of options."
        sys.exit(1)

    if len(args) != 1:
        print_use_help()

    quiet_mode = options.quiet_mode

    m = options.marker_loc

    if type(m) is str:
        m = int(m)
        
        if not quiet_mode:
            print "Finding equivalence classes at marker location %d." % m

    displayDuplicationCount(parse_F1_file(args[0]), m)

"""
Loads files of the various formats for testing purposes.
"""

from common import *
from ibdcreation import *
from itertools import *
import re

def parseFile(filename):
    """
    Parses a file and returns a list of ibd graph objects, each having
    only one reference count. 
    """
    
    if filename.endswith('.ibdf1'):
        return parse_F1_file(filename)
    else:
        raise ValueError("Format of file not recognized.")

def parse_F1_file(filename):
    f = open(filename)
    ibd_list = parse_F1_string(f.read())
    f.close()
    
    return ibd_list


def parse_F1_string(string):

    parsed_lines = [parse_F1_line(l, i+1) 
                    for i, l in enumerate(ll.strip() for ll in string.split('\n'))
                    if not (len(l) == 0 or l.startswith("#"))]

    # Process these all to allow indexing by edge
    edge_list = []

    for (pos, l1), l2 in zip(list(enumerate(parsed_lines))[::2], parsed_lines[1::2]):
        if l1[0] != l2[0]:
            raise Exception("Edges near lines %d don't match. ('%s' != '%s')" % (pos, l1[0], l2[0]))

        edge_list.append( (l1[0], [l1, l2]) )
    
    # now grab the first set of nodes
    def nextEdgeNameSet(start_pos):
        seen_edges = set()
        
        for edge, add_group in edge_list[start_pos:]:
            if edge in seen_edges:
                break
            else:
                seen_edges.add(edge)

        return seen_edges
        
    # Now parse the file
    working_set = None
    ibd_graphs = []
    cur_pos = 0
    n_groups = 0

    while True:
        next_set = nextEdgeNameSet(cur_pos)
        
        if len(next_set) == 0:
            break

        if working_set != next_set:
            n_groups += 1
            working_set = next_set
            
        connections = chain(*[
                line_group for edge, line_group in 
                (edge_list[cur_pos + i] for i in xrange(len(working_set)))
                ])
            
        cur_pos += len(working_set)
        
        ibd_graphs.append(createIBDGraph(connections))

    print "Loaded %d graphs." % len(ibd_graphs)

    return ibd_graphs

def parse_F1_line(l, pos):
    try:
        line_parse = r"\s*(?P<edge>\w+)\s+(?P<ibd_0>\w+)\s+(?P<ibd_1>\w+)\s+(?P<change_count>\d+)"
        changes_parse = r"\s*(?P<change_pos>\d+)\s+(?P<ibd>\w+)\s*"

        m = re.match(line_parse, l)

        if m is None:
            raise Exception("Parse Error; pattern not recognized (line = %s)" % l)

        edge = m.group('edge')
        ibd_0 = m.group('ibd_0')
        ibd_1 = m.group('ibd_1')
        changes = []

        change_count = int(m.group('change_count'))

        l = l[m.end():]

        for i in xrange(change_count):
            m = re.match(changes_parse, l)

            assert m is not None

            l = l[m.end():]

            changes.append( (m.group('ibd'), int(m.group('change_pos'))) )

        return [edge, ibd_0, [(ibd_1, 1)] + changes] 

    except Exception, e:
        raise Exception("Parse error on line %d: %s" % (pos, str(e)))

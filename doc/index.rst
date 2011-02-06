HashReduce
==========

HashReduce is a library for reducing dynamic structures into dynamic
hash keys for the purpose of fast comparison, indexing, and testing
operations.  In the targeted application (Identity-by-descent graphs
for genetic pedigrees), it speeds up several operations by many orders
of magnitude.  It is the code associated with my `tech report
<http://www.stat.washington.edu/research/reports/2010/tr567.pdf>`_ on
the subject, and is incorporated into the PANGAEA_ (Pedigree analysis
for genetics) framework for computations on genetic pedigree graphs;
if you intend to use this package for that purpose, download IBDgraph
from the PANGAEA_ website.

To standardize with that package, it is written in C; one thing on my
todo list is convert it to C++, which I think could make parts
significantly faster (as the current version reimplements some data
structures which are more optimized in C++).

The documentation is a bit limited for general use at this point; I
intend to change that as soon as I have time (sigh).  However, the
code is well organized and the header files are well commented, so
these should provide a decent reference.

.. toctree::
    :maxdepth: 2

    download
    license

.. _pangaea: http://www.stat.washington.edu/thompson/Genepi/pangaea.shtml

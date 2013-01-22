HashReduce
==========

HashReduce is a library for reducing dynamic structures into dynamic
hash keys for the purpose of fast comparison, indexing, and testing
operations.  In the targeted application (Identity-by-descent graphs
for genetic pedigrees), it speeds up several operations by many orders
of magnitude.  It implements the operations described in the following
two publications:

  - `Efficient Identification of Equivalences in Dynamic Graphs and Pedigree Structures. <http://www.stat.washington.edu/~hoytak/papers.html#efficient-identification-of-equivalences-in-dynamic-graphs-and-pedigree-structures>`_

  - `Efficient Testing Operations on Dynamic Graph Structures using Strong Hash Functions. <http://www.stat.washington.edu/research/reports/2010/tr567.pdf>`_

It is incorporated into the PANGAEA_ (Pedigree analysis for genetics)
framework for computations on genetic pedigree graphs. 

The documentation is a bit scattered for general use at this point; I
intend to change that as soon as I have time (sigh).  There are,
however, three places to start:

  - The appendix in the first of the papers above describes the API
    and functions implemented in the code.

  - The header files have extensive comments to detail how they are
    used. 

  - Two files are good places to look for example code.
    ibd_compare_c.c is the code for parsing and processing the IBD
    graph files using the library.  It calls functions in
    ibd_structures.{h,cpp}, which implement calls to the generic
    interface.   These two illustrate the proper use of the code. 

.. toctree::
    :maxdepth: 2

    download
    license

.. _pangaea: http://www.stat.washington.edu/thompson/Genepi/pangaea.shtml

#!/usr/bin/env python
import unittest, ctypes, sys, os
from ctypes import *
from ctypes.util import find_library

p = os.path.abspath("./")

if p not in sys.path:
    sys.path.append(p)


if os.getcwd().endswith('tests'):
    try:
        dgl = ctypes.cdll.LoadLibrary('../src/libhashreduce.so')
    except Exception:
        libfile = find_library('hashreduce')

        if libfile is None:
            raise ImportError("Cannot find library hashreduce.  Is it installed or compiled?")

        dgl = ctypes.cdll.LoadLibrary(libfile)

null_hash = '0'*32

dgl.H_Reduce.restype = ctypes.c_void_p
dgl.H_Rehash.restype = ctypes.c_void_p
dgl.H_Negative.restype = ctypes.c_void_p

def extractHash(hk):
    s = ctypes.create_string_buffer(33)
    dgl.H_ExtractHash(s, hk)
    return s.value[:32]

def exactHashKey(s):
    dgl.Hf_FillExact.restype = ctypes.c_void_p
    hk = dgl.Hf_FillExact(0, s)
    s_test = extractHash(hk)
    assert s_test == s, "(test) %s != %s (true)" % (s_test, s)
    return hk

def makeHashKey(v, f = None):
    hkf = dgl.NewHashObject

    hkf.restype = ctypes.c_void_p

    hk = hkf()

    
    if f is not None:
        if type(v) is not tuple:
            v = (v,)

        getattr(dgl, 'Hf_' + f)(hk, *v)

    elif type(v) is int:
        dgl.Hf_FromInt(hk, c_long(v))

    elif type(v) is str:
        dgl.Hf_FromString(hk, v)

    else:
        assert False

    return hk

def addMarkerInfo(hk, r_start, r_end, *arg_pairs):
    dgl.H_AddMarkerValidRange(hk, c_long(r_start), c_long(r_end))

    if(len(arg_pairs) != 0):
        assert(len(arg_pairs) % 2 == 0)

        for r_s, r_e in zip(arg_pairs[::2], arg_pairs[1::2]):
            dgl.H_AddMarkerValidRange(hk, c_long(r_s), c_long(r_e))
    
    return hk;


def makeMarkedHashKey(v, r_start, r_end, *arg_pairs):
    hk = makeHashKey(v)
    addMarkerInfo(hk, r_start, r_end, *arg_pairs)
    return hk


def hashKeyToNumber(hk):

    dgl.H_ExtractHashComponent.restype = ctypes.c_ulong
        
    a = dgl.H_ExtractHashComponent(hk, 0)
    b = dgl.H_ExtractHashComponent(hk, 1)
    c = dgl.H_ExtractHashComponent(hk, 2)
    d = dgl.H_ExtractHashComponent(hk, 3)

    return (2**96)*a + (2**64)*b + (2**32)*c + d

def numberToHashKey(n):
    a = (n // (2**96))
    b = (n % (2**96)) // (2**64)
    c = (n % (2**64)) // (2**32)
    d = n % (2**32)
    
    dgl.Hf_FillFromComponents.restype = ctypes.c_void_p
    return dgl.Hf_FillFromComponents(0, a, b, c, d)

def makeHash(v,f = None):
    hk = makeHashKey(v, f)
    h = extractHash(hk)
    dgl.O_DecRef(hk)
    return h
    
def dictOfHashes(n):

    d = {}
    
    for i in range(n):
        hk = makeHashKey(n)
        d[extractHash(hk)] = hk

    return d

def deleteDictOfHashes(d):
    for hk in d.itervalues():
        dgl.O_DecRef(hk)
    
def decRef(*args):
    for a in args:
        dgl.O_DecRef(a)



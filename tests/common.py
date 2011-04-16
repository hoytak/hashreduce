#!/usr/bin/env python
import unittest, ctypes, sys, os
from ctypes import *
from ctypes.util import find_library

p = os.path.abspath("./")

if p not in sys.path:
    sys.path.append(p)


if os.getcwd().endswith('tests'):
    try:
        ibd = ctypes.cdll.LoadLibrary('../src/libhashreduce.so')
    except Exception, e:
        libfile = find_library('hashreduce')

        if libfile is None:
            raise ImportError("Cannot find library hashreduce.  Is it installed or compiled?"
                              "(Original error: %s)" % str(e))

        ibd = ctypes.cdll.LoadLibrary(libfile)
else:
    try:
        ibd = ctypes.cdll.LoadLibrary('src/libhashreduce.so')
    except Exception, e:
        libfile = find_library('hashreduce')

        if libfile is None:
            raise ImportError("Cannot find library hashreduce.  Is it installed or compiled?"
                              "(Original error: %s)" % str(e))
        
        ibd = ctypes.cdll.LoadLibrary(libfile)
    
null_hash = '0'*32

ibd.H_Reduce.restype = ctypes.c_void_p
ibd.H_Rehash.restype = ctypes.c_void_p
ibd.H_Negative.restype = ctypes.c_void_p

def extractHash(hk):
    assert ibd.O_RefCount(hk) >= 1
    s = ctypes.create_string_buffer(33)
    ibd.H_ExtractHash(s, hk)
    return s.value[:32]

def exactHashKey(s):
    ibd.Hf_FillExact.restype = ctypes.c_void_p
    hk = ibd.Hf_FillExact(0, s)
    s_test = extractHash(hk)
    assert s_test == s, "(test) %s != %s (true)" % (s_test, s)
    assert ibd.O_RefCount(hk) == 1
    return hk

def makeHashKey(v, f = None):
    hkf = ibd.NewHashObject

    hkf.restype = ctypes.c_void_p

    hk = hkf()
    
    if f is not None:
        if type(v) is not tuple:
            v = (v,)

        getattr(ibd, 'Hf_' + f)(hk, *v)

    elif type(v) is int:
        ibd.Hf_FromInt(hk, c_long(v))
    elif type(v) is str:
        ibd.Hf_FromString(hk, v)

    else:
        assert False

    assert ibd.O_RefCount(hk) == 1

    return hk

def addMarkerInfo(hk, r_start, r_end, *arg_pairs):
    ibd.H_AddMarkerValidRange(hk, c_long(r_start), c_long(r_end))

    if(len(arg_pairs) != 0):
        assert(len(arg_pairs) % 2 == 0)

        for r_s, r_e in zip(arg_pairs[::2], arg_pairs[1::2]):
            ibd.H_AddMarkerValidRange(hk, c_long(r_s), c_long(r_e))

    return hk


def makeMarkedHashKey(v, r_start, r_end, *arg_pairs):
    hk = makeHashKey(v)
    addMarkerInfo(hk, r_start, r_end, *arg_pairs)
    assert ibd.O_RefCount(hk) == 1
    return hk


def hashKeyToNumber(hk):

    ibd.H_ExtractHashComponent.restype = ctypes.c_ulong
        
    a = ibd.H_ExtractHashComponent(hk, 0)
    b = ibd.H_ExtractHashComponent(hk, 1)
    c = ibd.H_ExtractHashComponent(hk, 2)
    d = ibd.H_ExtractHashComponent(hk, 3)

    return (2**96)*a + (2**64)*b + (2**32)*c + d

def numberToHashKey(n):
    a = (n // (2**96))
    b = (n % (2**96)) // (2**64)
    c = (n % (2**64)) // (2**32)
    d = n % (2**32)
    
    ibd.Hf_FillFromComponents.restype = ctypes.c_void_p

    hk = ibd.Hf_FillFromComponents(0, a, b, c, d)
    assert ibd.O_RefCount(hk) == 1

    return hk

def makeHash(v,f = None):
    hk = makeHashKey(v, f)
    h = extractHash(hk)
    ibd.O_DecRef(hk)
    return h
    
def dictOfHashes(n):

    d = {}
    
    for i in range(n):
        hk = makeHashKey(n)
        d[extractHash(hk)] = hk

    return d

def deleteDictOfHashes(d):
    for hk in d.itervalues():
        ibd.O_DecRef(hk)
    
def decRef(*args):
    for a in args:
        ibd.O_DecRef(a)



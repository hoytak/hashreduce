project(hashreduce)
cmake_minimum_required(VERSION 2.4)

add_library (hashreduce SHARED
  bitops.h                
  debugging.h      
  errorhandling.c
  hashkey_inline.h
  hashobject.h
  hashtable.h
  memorypool.c
  object.c
  brg_endian.h
  dgl.h
  errorhandling.h
  hashkeys.c
  hashobject_inline.h
  memorypool.h
  object.h
  dglstructures.c
  graphnode.h
  hashkeys.h
  hashobject.s
  markerinfo.c
  mtrandbasic.c
  optimizations.h
  dglstructures.h
  hashobject.c
  hashtable.c
  markerinfo.h
  mtrandbasic.h
  )

install(TARGETS hashreduce DESTINATION lib)




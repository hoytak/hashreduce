project(hashreduce)
cmake_minimum_required(VERSION 2.4)

add_library (hashreduce SHARED
  bitops.h                
  brg_endian.h
  debugging.h      
  ibd.h
  ibdstructures.c
  ibdstructures.h
  errorhandling.c
  errorhandling.h
  graphnode.h
  hashkey_inline.h
  hashkeys.c
  hashkeys.h
  hashobject.c
  hashobject.h
  hashobject_inline.h
  hashtable.c
  hashtable.h
  markerinfo.c
  markerinfo.h
  memorypool.c
  memorypool.h
  object.c
  object.h
  optimizations.h
  randfunctions.c
  randfunctions.h
  randfunctions_inline.h
  # hashreduce.c
  # hashreduce.h
  )

install(TARGETS hashreduce DESTINATION lib)




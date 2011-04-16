include(CheckTypeSize)

macro(CHECK_STRUCT_RESTRICT RESTRICT_NAME VARIABLE DEFINE_NAME)

  if(NOT STRUCT_RESTRICT_FOUND)
    message("Testing for struct restrict support with ${RESTRICT_NAME}.")
    check_type_size("${RESTRICT_NAME} " "struct_restrict_${DEFINE_NAME}")

    if("HAVE_struct_restrict_${DEFINE_NAME}")
      message("Found: Enabling restrict keyword support using ${RESTRICT_NAME}.")
      SET(STRUCT_RESTRICT_FOUND 1)
      SET(${VARIABLE} "${DEFINE_NAME}")
    else()
      set(STRUCT_RESTRICT_FOUND 0)
    endif()
  endif()
  
endmacro()

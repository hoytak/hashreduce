include(CheckTypeSize)

macro(CHECK_RESTRICT RESTRICT_NAME VARIABLE DEFINE_NAME)

  if(NOT RESTRICT_FOUND)
    message("Testing for restrict keyword support with ${RESTRICT_NAME}.")
    check_type_size("char * ${RESTRICT_NAME} " "restrict_kw_${DEFINE_NAME}")

    if("HAVE_restrict_kw_${DEFINE_NAME}")
      message("Found: Enabling restrict keyword support using ${RESTRICT_NAME}.")
      SET(RESTRICT_FOUND 1)
      SET(${VARIABLE} "${DEFINE_NAME}")
    else()
      set(RESTRICT_FOUND 0)
    endif()
  endif()
  
endmacro()

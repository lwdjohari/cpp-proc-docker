# Usage:
#   proc_generate(<OUTVAR> <PC_SRC> <PROJECT_INCLUDES>)
# Example:
#   proc_generate(GEN_emp "${CMAKE_SOURCE_DIR}/src/xora_proc_emp_fetch.pc"
#                 "${CMAKE_SOURCE_DIR}/inc;${CMAKE_SOURCE_DIR}/proc/include")
function(proc_generate OUTVAR PC_SRC PROJECT_INCLUDES)
  if (NOT PROC)
    message(FATAL_ERROR "Set -DPROC=/path/to/proc (Pro*C precompiler)")
  endif()
  if (NOT ORA_INC)
    message(FATAL_ERROR "Set -DORA_INC=/path/to/oci/include (e.g. /usr/include/oracle/23/client64)")
  endif()

  # Default sys_include; override with -DPROC_SYS_INCLUDE="a,b,c"
  if (NOT PROC_SYS_INCLUDE)
    set(PROC_SYS_INCLUDE
      "/usr/lib/gcc/x86_64-redhat-linux/11/include,\
/usr/lib/gcc/x86_64-redhat-linux/11/include-fixed,\
/usr/local/include,/usr/include")
  endif()

  get_filename_component(_abs_pc "${PC_SRC}" ABSOLUTE)
  get_filename_component(_name   "${PC_SRC}" NAME_WE)
  set(_gen_c "${CMAKE_CURRENT_BINARY_DIR}/${_name}_proc.c")

  if (NOT EXISTS "${_abs_pc}")
    message(FATAL_ERROR "proc_generate: source .pc not found: ${_abs_pc}")
  endif()

  # Build include CSV for Pro*C: start with ORA_INC, then caller-provided dirs
  set(_include_csv "${ORA_INC}")
  if (PROJECT_INCLUDES)
    # Warn about any missing include dirs
    foreach(_p ${PROJECT_INCLUDES})
      if (NOT EXISTS "${_p}")
        message(WARNING "proc_generate: include dir does not exist: ${_p}")
      endif()
    endforeach()
    string(REPLACE ";" "," _proj_inc_csv "${PROJECT_INCLUDES}")
    set(_include_csv "${_include_csv},${_proj_inc_csv}")
  endif()

  if (PCS_CFG AND (NOT EXISTS "${PCS_CFG}"))
    message(WARNING "proc_generate: PCS_CFG file not found: ${PCS_CFG}")
  endif()

  # Configure-time diagnostics
  message(STATUS "proc_generate: ${PC_SRC}")
  message(STATUS "  iname            = ${_abs_pc}")
  message(STATUS "  oname            = ${_gen_c}")
  message(STATUS "  include=(        = ${_include_csv} )")
  message(STATUS "  sys_include      = ${PROC_SYS_INCLUDE}")
  message(STATUS "  PROC             = ${PROC}")
  message(STATUS "  PCS_CFG          = ${PCS_CFG}")

  add_custom_command(
  OUTPUT "${_gen_c}"
  COMMAND ${CMAKE_COMMAND} -E echo "-- Pro*C precompile"
  COMMAND ${CMAKE_COMMAND} -E echo "--   iname = ${_abs_pc}"
  COMMAND ${CMAKE_COMMAND} -E echo "--   oname = ${_gen_c}"
  COMMAND ${CMAKE_COMMAND} -E echo "--   include = (${_include_csv})"
  COMMAND ${CMAKE_COMMAND} -E echo "--   sys_include = (${PROC_SYS_INCLUDE})"
  COMMAND ${CMAKE_COMMAND} -E echo "--   parse = none"              # <— add this echo
  COMMAND ${CMAKE_COMMAND} -E echo "--   config = ${PCS_CFG}"
  COMMAND "${PROC}"
          iname=${_abs_pc}
          oname=${_gen_c}
          code=ansi_c
          mode=ansi
          parse=none                                             # <— add this option
          "include=(${_include_csv})"
          "sys_include=(${PROC_SYS_INCLUDE})"
          define=XORA_PROC=1
          define=__x86_64__=1
          define=__GNUC__=11
          define=__GNUC_MINOR__=0
          config=${PCS_CFG}
  DEPENDS "${_abs_pc}"
  COMMENT "Pro*C: ${PC_SRC} → ${_gen_c}"
  VERBATIM
)


  set(${OUTVAR} "${_gen_c}" PARENT_SCOPE)
endfunction()

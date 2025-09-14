# ------------------------------------------------------------------------------
# procgen.cmake — Pro*C precompile helper (hardened path handling)
#
# Usage:
#   proc_generate(<OUTVAR> <PC_SRC> <PROJECT_INCLUDES>)
#
# Example:
#   proc_generate(GEN_emp
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/xora_proc_emp_fetch.pc"
#     "${CMAKE_CURRENT_SOURCE_DIR}/inc;${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb/inc")
#
# Required -D vars:
#   -DPROC=/opt/oracle/instantclient/bin/proc
#   -DORA_INC=/usr/include/oracle/23/client64
#
# Optional:
#   -DPROC_SYS_INCLUDE="a,b,c"
#   -DPCS_CFG=/opt/oracle/instantclient/lib/precomp/admin/pcscfg.cfg
#   -DPROC_DEFINES="NAME=VALUE;FOO=1"
# ------------------------------------------------------------------------------

function(proc_generate OUTVAR PC_SRC PROJECT_INCLUDES)
  # --- Preconditions ----------------------------------------------------------
  if (NOT PROC)
    message(FATAL_ERROR "proc_generate: Set -DPROC=/path/to/proc (Pro*C precompiler)")
  endif()
  if (NOT ORA_INC)
    message(FATAL_ERROR "proc_generate: Set -DORA_INC=/path/to/oci/include (e.g. /usr/include/oracle/23/client64)")
  endif()

  # Default sys_include CSV unless user overrides
  if (NOT PROC_SYS_INCLUDE)
    set(PROC_SYS_INCLUDE
      "/usr/lib/gcc/x86_64-redhat-linux/11/include,\
/usr/lib/gcc/x86_64-redhat-linux/11/include-fixed,\
/usr/local/include,/usr/include")
  endif()

  # --- Resolve/validate source & output --------------------------------------
  get_filename_component(_abs_pc "${PC_SRC}" ABSOLUTE)
  get_filename_component(_abs_pc "${_abs_pc}" REALPATH)
  if (NOT EXISTS "${_abs_pc}")
    message(FATAL_ERROR "proc_generate: source .pc not found: ${_abs_pc}")
  endif()

  get_filename_component(_name "${_abs_pc}" NAME_WE)
  set(_gen_c "${CMAKE_CURRENT_BINARY_DIR}/${_name}_proc.c")

  # --- Helper to trim + unquote a path ---------------------------------------
  function(_strip_and_unquote IN OUT)
    set(_p "${IN}")
    string(STRIP "${_p}" _p)                          # remove leading/trailing spaces
    if (_p MATCHES "^\"(.*)\"$")                      # strip surrounding quotes if present
      string(REGEX REPLACE "^\"(.*)\"$" "\\1" _p "${_p}")
    endif()
    set(${OUT} "${_p}" PARENT_SCOPE)
  endfunction()

  # --- Normalize ORA_INC ------------------------------------------------------
  _strip_and_unquote("${ORA_INC}" _ora_inc_trim)
  if (IS_ABSOLUTE "${_ora_inc_trim}")
    get_filename_component(_ora_inc_norm "${_ora_inc_trim}" REALPATH)
  else()
    get_filename_component(_ora_inc_norm "${_ora_inc_trim}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()
  if (NOT EXISTS "${_ora_inc_norm}")
    message(FATAL_ERROR "proc_generate: ORA_INC does not exist:\n  original='${ORA_INC}'\n  resolved='${_ora_inc_norm}'")
  endif()

  # --- Normalize caller-provided include dirs --------------------------------
  set(_resolved_inc_list "${_ora_inc_norm}")
  if (PROJECT_INCLUDES)
    foreach(_p ${PROJECT_INCLUDES})
      _strip_and_unquote("${_p}" _p_trim)
      if (_p_trim STREQUAL "")
        continue()
      endif()
      if (IS_ABSOLUTE "${_p_trim}")
        get_filename_component(_p_norm "${_p_trim}" REALPATH)
      else()
        get_filename_component(_p_norm "${_p_trim}" REALPATH BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
      endif()
      if (EXISTS "${_p_norm}")
        list(APPEND _resolved_inc_list "${_p_norm}")
      else()
        message(WARNING "proc_generate: include dir does not exist:\n  original='${_p}'\n  resolved='${_p_norm}'")
      endif()
    endforeach()
  endif()
  list(REMOVE_DUPLICATES _resolved_inc_list)

  # Build CSV for proc's include=(...)
  if (_resolved_inc_list)
    string(REPLACE ";" "," _include_csv "${_resolved_inc_list}")
  else()
    set(_include_csv "${_ora_inc_norm}")
  endif()

  # --- Optional config file ---------------------------------------------------
  if (PCS_CFG AND (NOT EXISTS "${PCS_CFG}"))
    message(WARNING "proc_generate: PCS_CFG file not found: ${PCS_CFG}")
  endif()

  # --- Optional extra defines for precompiler --------------------------------
  set(_define_args "")
  if (PROC_DEFINES)
    foreach(_def ${PROC_DEFINES})
      list(APPEND _define_args "define=${_def}")
    endforeach()
  endif()
  list(APPEND _define_args "define=XORA_PROC=1" "define=__x86_64__=1" "define=__GNUC__=11" "define=__GNUC_MINOR__=0")

  # --- Diagnostics ------------------------------------------------------------
  message(STATUS "proc_generate: ${PC_SRC}")
  message(STATUS "  iname              = ${_abs_pc}")
  message(STATUS "  oname              = ${_gen_c}")
  message(STATUS "  include (resolved) = ${_resolved_inc_list}")
  message(STATUS "  include CSV        = ${_include_csv}")
  message(STATUS "  sys_include        = ${PROC_SYS_INCLUDE}")
  message(STATUS "  PROC               = ${PROC}")
  message(STATUS "  PCS_CFG            = ${PCS_CFG}")
  message(STATUS "  EXTRA DEFINES      = ${PROC_DEFINES}")

  # --- Command ----------------------------------------------------------------
  add_custom_command(
    OUTPUT "${_gen_c}"
    COMMAND ${CMAKE_COMMAND} -E echo "-- Pro*C precompile"
    COMMAND ${CMAKE_COMMAND} -E echo "--   iname = ${_abs_pc}"
    COMMAND ${CMAKE_COMMAND} -E echo "--   oname = ${_gen_c}"
    COMMAND ${CMAKE_COMMAND} -E echo "--   include = (${_include_csv})"
    COMMAND ${CMAKE_COMMAND} -E echo "--   sys_include = (${PROC_SYS_INCLUDE})"
    COMMAND ${CMAKE_COMMAND} -E echo "--   parse = none"
    COMMAND ${CMAKE_COMMAND} -E echo "--   config = ${PCS_CFG}"
    COMMAND "${PROC}"
            iname=${_abs_pc}
            oname=${_gen_c}
            code=ansi_c
            mode=ansi
            parse=none
            "include=(${_include_csv})"
            "sys_include=(${PROC_SYS_INCLUDE})"
            ${_define_args}
            config=${PCS_CFG}
    DEPENDS "${_abs_pc}"
    COMMENT "Pro*C: ${PC_SRC} → ${_gen_c}"
    VERBATIM
  )

  set(${OUTVAR} "${_gen_c}" PARENT_SCOPE)
endfunction()

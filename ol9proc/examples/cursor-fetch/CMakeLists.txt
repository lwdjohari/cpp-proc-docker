cmake_minimum_required(VERSION 3.20)
project(xora_db C)

# ---- Configurable paths/flags (override with -D...) ----
set(PROC    "/opt/oracle/instantclient/bin/proc" CACHE FILEPATH "Pro*C precompiler")
set(PCS_CFG  "/opt/oracle/instantclient/lib/precomp/admin/pcscfg.cfg" CACHE FILEPATH "Pro*C config")
set(ORA_INC "/usr/include/oracle/23/client64"    CACHE PATH     "OCI include dir")
set(PROC_SYS_INCLUDE "/usr/lib/gcc/x86_64-redhat-linux/11/include,/usr/lib/gcc/x86_64-redhat-linux/11/include-fixed,/usr/local/include,/usr/include" CACHE STRING "Comma-separated system include paths for Pro*C")
set(XORA_OCI_LIBS "/opt/oracle/instantclient/lib" CACHE STRING "Extra link libs for OCI (e.g. clntsh)")

option(XORA_ENABLE_EXAMPLES "Build xora_demo example" ON)

# ---- Precompile helper ----
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(procgen)  

# ---- Include paths for compiled C (public headers) ----
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stb/inc
  ${ORA_INC}  # for <sqlca.h>
)

# ---- Explicit .pc sources (maintain this list) ----
set(XORA_PC_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/xora_contex.pc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/xora_proc_emp_fetch.pc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/xora_proc_emp_fvect.pc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/xora_proc_emp_crud.pc
)

# Guardrail: ensure each .pc includes the proc aggregator you use
# foreach(XORA_PC ${XORA_PC_SOURCES})
#   file(READ "${XORA_PC}" _pc_txt)
#   string(FIND "${_pc_txt}" "xora_proc.h" _has_agg)
#   if (_has_agg EQUAL -1)
#     message(FATAL_ERROR "Missing #include \"xora_proc.h\" in: ${XORA_PC}")
#   endif()
# endforeach()

# Project include dirs for Pro*C (semicolon-separated)
set(XORA_PC_INCLUDES
  "${CMAKE_CURRENT_SOURCE_DIR}/inc; ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stb/inc/")

# ---- Precompile .pc → .c ----
set(XORA_GENERATED_C_SOURCES "")
foreach(XORA_PC ${XORA_PC_SOURCES})
  get_filename_component(name "${XORA_PC}" NAME_WE)
  string(MAKE_C_IDENTIFIER "${name}" VARSAFE)
  set(OUTVAR "GEN_${VARSAFE}_C")
  proc_generate(${OUTVAR} "${XORA_PC}" "${XORA_PC_INCLUDES}")
  list(APPEND XORA_GENERATED_C_SOURCES "${${OUTVAR}}")
endforeach()

# ---- Library from generated C ----
add_library(xora_db STATIC ${XORA_GENERATED_C_SOURCES})
target_include_directories(xora_db 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stb/inc
        {ORA_INC}  # for <sqlca.h>
)
# if (XORA_OCI_LIBS)
  target_link_directories(xora_db PRIVATE "${XORA_OCI_LIBS}")
  target_link_libraries(xora_db PRIVATE clntsh)
  # RPATH so runtime finds libclntsh
set_target_properties(xora_db PROPERTIES
  BUILD_RPATH   "${XORA_OCI_LIBS}"
  INSTALL_RPATH "${XORA_OCI_LIBS}"
)

# endif()

# ---- Example binary ----
add_executable(xora src/main.c)
target_link_directories(xora PRIVATE "${XORA_OCI_LIBS}")
target_link_libraries(xora PRIVATE xora_db clntsh)


cmake_minimum_required(VERSION 3.15)
project(simple_fecth C)

# --- Paths from your working setup ---
set(PROC     "/opt/oracle/instantclient/bin/proc"            CACHE FILEPATH "Path to proc")
set(PCS_CFG  "/opt/oracle/instantclient/lib/precomp/admin/pcscfg.cfg" CACHE FILEPATH "Pro*C config")
set(ORA_INC  "/usr/include/oracle/23/client64"               CACHE PATH    "Oracle SDK include (sqlca.h)")
set(ORA_LIB  "/opt/oracle/instantclient/lib"                 CACHE PATH    "Oracle client lib dir")
set(PC_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/simplefetch.pc"                  CACHE FILEPATH "Pro*C source")

# --- Sanity checks ---
if(NOT EXISTS "${PROC}")
  message(FATAL_ERROR "proc not found at ${PROC}")
endif()
if(NOT EXISTS "${PCS_CFG}")
  message(FATAL_ERROR "pcscfg.cfg not found at ${PCS_CFG}")
endif()
if(NOT EXISTS "${ORA_INC}/sqlca.h")
  message(FATAL_ERROR "sqlca.h not found in ${ORA_INC}")
endif()
if(NOT EXISTS "${PC_SRC}")
  message(FATAL_ERROR "PC source not found: ${PC_SRC}")
endif()

set(GENERATED_C "${CMAKE_CURRENT_BINARY_DIR}/simplefetch.c")

# --- EXACT proc command you confirmed working ---
add_custom_command(
  OUTPUT  "${GENERATED_C}"
  COMMAND "${PROC}"
          iname=${PC_SRC}
          oname=${GENERATED_C}
          code=ansi_c
          "include=(${ORA_INC})"
          "sys_include=(/usr/lib/gcc/x86_64-redhat-linux/11/include,/usr/lib/gcc/x86_64-redhat-linux/11/include-fixed,/usr/local/include,/usr/include)"
          define=__x86_64__=1
          define=__GNUC__=11
          define=__GNUC_MINOR__=0
          config=${PCS_CFG}
  DEPENDS "${PC_SRC}"
  COMMENT "Pro*C: precompiling ${PC_SRC} -> ${GENERATED_C}"
  VERBATIM
)

add_executable(simplefetch "${GENERATED_C}")
target_include_directories(simplefetch PRIVATE "${ORA_INC}")

# Link Oracle client
target_link_directories(simplefetch PRIVATE "${ORA_LIB}")
target_link_libraries(simplefetch PRIVATE clntsh)

# RPATH so runtime finds libclntsh
set_target_properties(simplefetch PROPERTIES
  BUILD_RPATH   "${ORA_LIB}"
  INSTALL_RPATH "${ORA_LIB}"
)


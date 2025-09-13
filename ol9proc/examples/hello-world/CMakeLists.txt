cmake_minimum_required(VERSION 3.15)
project(hello-world C)

# --- Paths from your working setup ---
set(PROC     "/opt/oracle/instantclient/bin/proc"            CACHE FILEPATH "Path to proc")
set(PCS_CFG  "/opt/oracle/instantclient/lib/precomp/admin/pcscfg.cfg" CACHE FILEPATH "Pro*C config")
set(ORA_INC  "/usr/include/oracle/23/client64"               CACHE PATH    "Oracle SDK include (sqlca.h)")
set(ORA_LIB  "/opt/oracle/instantclient/lib"                 CACHE PATH    "Oracle client lib dir")
set(HW_PC_SRC   "${CMAKE_CURRENT_SOURCE_DIR}/hello.pc"          ) # Add prefix avoid race in build when using ninja


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
if(NOT EXISTS "${HW_PC_SRC}")
  message(FATAL_ERROR "PC source not found: ${HW_PC_SRC}")
endif()

set(HW_GENERATED_C "${CMAKE_CURRENT_BINARY_DIR}/hello_proc.c") # Add prefix avoid race in build when using ninja


# --- EXACT proc command  ---
add_custom_command(
  OUTPUT  "${HW_GENERATED_C}"
  COMMAND "${PROC}"
          iname=${HW_PC_SRC}
          oname=${HW_GENERATED_C}
          code=ansi_c
          "include=(${ORA_INC})"
          "sys_include=(/usr/lib/gcc/x86_64-redhat-linux/11/include,/usr/lib/gcc/x86_64-redhat-linux/11/include-fixed,/usr/local/include,/usr/include)"
          define=__x86_64__=1
          define=__GNUC__=11
          define=__GNUC_MINOR__=0
          config=${PCS_CFG}
  DEPENDS "${HW_PC_SRC}"
  COMMENT "Pro*C: precompiling ${HW_PC_SRC} -> ${HW_GENERATED_C}"
  VERBATIM
)

add_executable(hello_c "${HW_GENERATED_C}")
target_include_directories(hello_c PRIVATE "${ORA_INC}")

# Link Oracle client
target_link_directories(hello_c PRIVATE "${ORA_LIB}")
target_link_libraries(hello_c PRIVATE clntsh)

# RPATH so runtime finds libclntsh
set_target_properties(hello_c PROPERTIES
  BUILD_RPATH   "${ORA_LIB}"
  INSTALL_RPATH "${ORA_LIB}"
)


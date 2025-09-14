cmake_minimum_required(VERSION 3.10)

# Project metadata
project(printf101_demos
  VERSION 1.0
  LANGUAGES C)

# Require C99 (portable ISO C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # no GNU extensions by default

# Source
add_executable(printf101
  main.c)

# Warnings & sanitizers (configurable)
if(MSVC)
  # /W4: high warnings, /permissive-: stricter conformance
  target_compile_options(printf101 PRIVATE /W4 /permissive-)
else()
  # -Wall -Wextra: good warnings; -Werror optional
  target_compile_options(printf101 PRIVATE -Wall -Wextra -Wpedantic)
  # Enable this locally if you want: -Werror treats warnings as errors
  # target_compile_options(printf101 PRIVATE -Werror)

  # Address sanitizer toggle (optional)
  option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
  if(ENABLE_ASAN)
    target_compile_options(printf101 PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(printf101 PRIVATE -fsanitize=address)
  endif()
endif()

# On some platforms you may need to link m for math (not used here)
# target_link_libraries(printf101 PRIVATE m)

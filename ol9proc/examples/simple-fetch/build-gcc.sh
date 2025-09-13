#!/usr/bin/env bash
set -euo pipefail

# ---- Paths (match your system) ----
PROC="/opt/oracle/instantclient/bin/proc"
PCS_CFG="/opt/oracle/instantclient/lib/precomp/admin/pcscfg.cfg"
ORA_INC="/usr/include/oracle/23/client64"
ORA_LIB="/opt/oracle/instantclient/lib"

SRC="${1:-hello.pc}"
MODE="${2:-}"   # optional: --exact

BASE="$(basename "$SRC" .pc)"
GEN="${BASE}_proc.c"
OUT="hello_c"
[[ -r "$SRC" ]] || { echo "ERR: $SRC not found"; exit 1; }

if [[ "$MODE" == "--exact" ]]; then
  # === EXACT: your original commands, byte-for-byte behavior ===
  "$PROC" \
    iname="$SRC" oname="$GEN" \
    code=ansi_c \
    "include=(${ORA_INC})" \
    "sys_include=(/usr/lib/gcc/x86_64-redhat-linux/11/include,/usr/lib/gcc/x86_64-redhat-linux/11/include-fixed,/usr/local/include,/usr/include)" \
    define=__x86_64__=1 define=__GNUC__=11 define=__GNUC_MINOR__=0 \
    "config=${PCS_CFG}"
else
  # === QUIET: no glibc-parse noise; same output ===
  "$PROC" \
    iname="$SRC" oname="$GEN" \
    code=ansi_c \
    parse=none sqlcheck=syntax \
    "include=(${ORA_INC})" \
    "config=${PCS_CFG}"
fi

gcc -I"$ORA_INC" "$GEN" \
    -L"$ORA_LIB" -Wl,-rpath,"$ORA_LIB" \
    -lclntsh -o "$OUT"

./"$OUT"


#!/usr/bin/env bash
set -eo pipefail

# load system profile for non-login shells too
# Source system profile safely (nounset off while sourcing)
if [ -f /etc/profile ]; then
  set +u
  . /etc/profile
  set -u 2>/dev/null || true
fi


log() { echo "[entrypoint] $*"; }
bool() { case "${1,,}" in true|1|yes|on) return 0 ;; *) return 1 ;; esac; }

# ---------------- Root SSH controls ----------------
SSHD_ENABLED="${SSHD_ENABLED:-false}"
ROOT_PASSWORD="${ROOT_PASSWORD:-}"

# ---------------- Extra user controls ----------------
CREATE_USER="${CREATE_USER:-false}"     # true|false
USER_NAME="${USER_NAME:-dev}"
USER_UID="${USER_UID:-1000}"
USER_GID="${USER_GID:-1000}"
USER_PASSWORD="${USER_PASSWORD:-}"
USER_SUDO="${USER_SUDO:-true}"          # true -> NOPASSWD sudo
USER_SHELL="${USER_SHELL:-/bin/bash}"
TAKE_WORKSPACE="${TAKE_WORKSPACE:-true}"

log "SSHD_ENABLED=${SSHD_ENABLED}"
log "CREATE_USER=${CREATE_USER} USER_NAME=${USER_NAME} USER_UID=${USER_UID} USER_GID=${USER_GID} USER_SUDO=${USER_SUDO} TAKE_WORKSPACE=${TAKE_WORKSPACE}"

# Always ensure /workspace exists (bind mount may be empty)
install -d -m 0755 -o root -g root /workspace || true

# ---------------- Configure root SSH (password login) ----------------
if bool "${SSHD_ENABLED}"; then
  if [[ -z "${ROOT_PASSWORD}" ]]; then
    ROOT_PASSWORD=$(tr -dc 'A-Za-z0-9!@#%^_+=' </dev/urandom | head -c 18)
    log "Generated ROOT_PASSWORD (dev only): ${ROOT_PASSWORD}"
  else
    log "Using provided ROOT_PASSWORD (masked)"
  fi
  echo "root:${ROOT_PASSWORD}" | chpasswd

  sed -ri 's/^#?PasswordAuthentication .*/PasswordAuthentication yes/' /etc/ssh/sshd_config
  sed -ri 's/^#?PermitRootLogin .*/PermitRootLogin yes/' /etc/ssh/sshd_config
  echo 'ClientAliveInterval 60' >>/etc/ssh/sshd_config
  echo 'ClientAliveCountMax  3'  >>/etc/ssh/sshd_config
else
  log "SSHD DISABLED (set SSHD_ENABLED=true to enable)"
fi

# ---------------- Create extra user (optional) ----------------
if bool "${CREATE_USER}"; then
  if [[ -z "${USER_PASSWORD}" ]]; then
    log "ERROR: CREATE_USER=true but USER_PASSWORD is empty. Refusing to start."
    exit 64
  fi

  # 1) Resolve/create group by GID
  EXISTING_GROUP_BY_GID="$(getent group "${USER_GID}" | cut -d: -f1 || true)"
  if [[ -n "${EXISTING_GROUP_BY_GID}" ]]; then
    GROUP_NAME="${EXISTING_GROUP_BY_GID}"
    log "Reusing existing group gid=${USER_GID} name='${GROUP_NAME}'"
  else
    GROUP_NAME="${USER_NAME}"
    if ! getent group "${GROUP_NAME}" >/dev/null 2>&1; then
      groupadd -g "${USER_GID}" "${GROUP_NAME}"
      log "Created group name='${GROUP_NAME}' gid=${USER_GID}"
    else
      # group name exists but with different gid — fallback: use that name's gid
      EXISTING_GID="$(getent group "${GROUP_NAME}" | cut -d: -f3)"
      log "WARNING: group '${GROUP_NAME}' already exists with gid=${EXISTING_GID}; overriding desired gid=${USER_GID}"
      USER_GID="${EXISTING_GID}"
    fi
  fi

  # 2) Create / ensure user with desired UID/GID
  if ! id -u "${USER_NAME}" >/dev/null 2>&1; then
    useradd -m -u "${USER_UID}" -g "${USER_GID}" -s "${USER_SHELL}" "${USER_NAME}"
    log "Created user: ${USER_NAME} (uid=${USER_UID}, gid=${USER_GID}, group='${GROUP_NAME}', shell=${USER_SHELL})"
  else
    ACT_UID="$(id -u "${USER_NAME}")"
    ACT_GID="$(id -g "${USER_NAME}")"
    log "User ${USER_NAME} already exists (uid=${ACT_UID}, gid=${ACT_GID})"
    # Align UID/GID if mismatched (best-effort)
    if [[ "${ACT_UID}" != "${USER_UID}" ]]; then
      usermod -u "${USER_UID}" "${USER_NAME}"
      log "Updated UID for ${USER_NAME} -> ${USER_UID}"
    fi
    if [[ "${ACT_GID}" != "${USER_GID}" ]]; then
      usermod -g "${USER_GID}" "${USER_NAME}"
      log "Updated GID for ${USER_NAME} -> ${USER_GID}"
    fi
  fi

  echo "${USER_NAME}:${USER_PASSWORD}" | chpasswd
  log "Set password for ${USER_NAME} (masked)"

  # 3) Sudo privileges
  if bool "${USER_SUDO}"; then
    echo "${USER_NAME} ALL=(ALL) NOPASSWD:ALL" >/etc/sudoers.d/90-${USER_NAME}
    chmod 0440 /etc/sudoers.d/90-${USER_NAME}
    log "Granted sudo (NOPASSWD) to ${USER_NAME}"
  fi

  # 4) Ensure home exists & is owned (works even if /home or /home/$USER is a bind mount)
  USER_HOME="$(getent passwd "${USER_NAME}" | cut -d: -f6)"
  install -d -m 0755 -o "${USER_UID}" -g "${USER_GID}" "${USER_HOME}"
  install -d -m 0700 -o "${USER_UID}" -g "${USER_GID}" "${USER_HOME}/.ssh"
  touch "${USER_HOME}/.ssh/authorized_keys"
  chown "${USER_UID}:${USER_GID}" "${USER_HOME}/.ssh/authorized_keys"
  chmod 0600 "${USER_HOME}/.ssh/authorized_keys"
  log "Prepared home=${USER_HOME} (uid=${USER_UID}, gid=${USER_GID}) and SSH dir for ${USER_NAME}"

  # 5) Hand over /workspace if requested
  if bool "${TAKE_WORKSPACE}"; then
    if [[ -d /workspace ]]; then
      chown -R "${USER_UID}:${USER_GID}" /workspace || true
      log "Ownership of /workspace -> ${USER_UID}:${USER_GID}"
    else
      log "WARNING: /workspace not found to chown"
    fi
  fi
fi

# ---------------- Start SSHD (if enabled) ----------------
if bool "${SSHD_ENABLED}"; then
  ssh-keygen -A >/dev/null 2>&1 || true
  /usr/sbin/sshd -E /var/log/sshd.log
  log "SSHD started on port 22"
  log "Login:"
  log " - root: ssh -p <port> root@<host>"
  if bool "${CREATE_USER}"; then
    log " - ${USER_NAME}: ssh -p <port> ${USER_NAME}@<host>"
  fi
fi

exec "$@"


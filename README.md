# C, C++, and Pro*C/C++ Docker Development Container

This container provides a **ready-to-use development environment** for:  
- Standard **C and C++ projects**  
- Oracle **Pro*C/C++ applications** (via Oracle Instant Client + precompiler)
- CMAKE & GDB Ready 

It’s built for developers who want a consistent workspace with modern compilers, CMake/Ninja builds, debugging tools, and Oracle database connectivity without tedious setup.  

You can develop as **root** or create a dedicated **developer user** (with its own home, sudo, and persistent workspace).

## Table of Contents
- [C, C++, and Pro\*C/C++ Docker Development Container](#c-c-and-procc-docker-development-container)
  - [Why this exists](#why-this-exists)
  - [Highlights of ol9proc Dev Image](#highlights)
  - [Oracle Instant Client requirement](#oracle-instant-client-requirement)
  - [Building the ol9proc Image (Dev Image)](#building-the-image)
  - [Running with Docker Compose for ol9proc-dev Container](#running-with-docker-compose)
  - [Connecting to ol9proc-dev Container via SSH](#connecting-via-ssh)
  - [VS Code Remote SSH setup](#vs-code-remote-ssh-setup)
  - [Mini Hello World Pro\*C Project](#mini-hello-world-proc-project)
    - [Build and run inside container](#build-and-run-inside-container)
  - [Development workflow](#development-workflow)
  - [Oracle23ai DB Docker Container Setup](#oracle23ai-docker-container-setup)

## Why this exists  

Getting **Pro C/C++, C, or C++** running cleanly on a modern system can be a slog:  
- Oracle Instant Client RPMs in the right place  
- Header paths, linker flags, and library quirks  
- Wiring **CMake** toolchains and **Ninja** builds without breaking something  

All of that just to see “Hello, world” compile.  

This container cuts straight through the noise:  
- **GCC, Clang, CMake, and Ninja** are preinstalled and ready  
- Oracle client and `proc` are already wired into the build flow  
- Example CMake config shows the pattern, you can expand it as you like  

It’s a **Development Container** (not a Production Container). Perfect if you:  
- Already know these languages and want it to *just work out of the box*  
- Or you’re learning Pro C/C++, C, or C++ but don’t have time to build the environment yourself  

Mount your code, run CMake, and get back to coding.  


## Highlights

- **Toolchain**: GCC, G++, Clang/LLVM, CMake, Ninja, pkg-config  
- **Pro*C support**: Oracle Instant Client with Pro*C precompiler (`proc`)  
- **Debugging**: GDB and gdbserver, container debugging enabled (`SYS_PTRACE`, `seccomp=unconfined`)  
- **Persistent dev**: `/workspace` for source code and `/home/<user>` for your user environment, mapped to host  
- **Custom dev user**: created from environment variables, UID/GID matching host, with sudo access  
- **Remote-friendly**: SSHD enabled with password login, works with VS Code Remote SSH  



## Oracle Instant Client requirement

Before building the image, download these three RPMs from Oracle Technology Network:  
https://www.oracle.com/database/technologies/instant-client/downloads.html  

Place them **next to the Dockerfile**. Example file names (version may vary, x86_64 shown):  

- **Basic**  
  `oracle-instantclient<DBVERSION>-basic-<VERSION>.x86_64.rpm`  
  Runtime libraries needed to run apps.  

- **SDK (devel)**  
  `oracle-instantclient<DBVERSION>-devel-<VERSION>.x86_64.rpm`  
  Header files and makefiles needed to build apps.  

- **Precomp (Pro*C/C++)**  
  `oracle-instantclient<DBVERSION>-precomp-<VERSION>.x86_64.rpm`  
  The Pro*C/C++ precompiler (`proc`) used to convert `.pc` → `.c/.cpp`.
  
- **sqlplus (optional)**  
  `oracle-instantclient-sqlplus-<VERSION>.x86_64.rpm`  
  If you also want sqlplus to be available, include the SQL*Plus RPM at build time (optional).  

The dockerfile is written to be ready to support any version of Oracle instant client supplied.
```dockerfile
# --- Instant Client RPMs (Basic, SDK, Precomp) ---
ADD oracle-instantclient-*.rpm /tmp/
RUN rpm -ivh /tmp/oracle-instantclient-*.rpm && rm -f /tmp/*.rpm
```



## Building the Image

```bash
# Clone repo
git clone https://github.com/lwdjohari/cpp-proc-docker.git
cd cpp-proc-docker/ol9proc

# Place the three Oracle Instant Client RPMs here

# Build
docker build -t ol9proc .

# Or if you want fresh build each time with --no-cache
docker build --no-cache -t ol9proc .
```



## Running with Docker Compose

Example `docker-compose.yml`:

```yaml
services:
  dev:
    image: ol9proc:latest
    container_name: ol9proc-dev
    environment:
      SSHD_ENABLED: "true"
      ROOT_PASSWORD: "<ROOT_PASSWD>"

      CREATE_USER: "true"  # disable if not needed
      USER_NAME: "<YOUR_USER>"
      USER_UID: "1001"
      USER_GID: "1001"
      USER_PASSWORD: "<USER_PASSWD>"
      USER_SUDO: "true"
      TAKE_WORKSPACE: "true"

    ports:
      - "2222:22"
      - "9001-9010:9001-9010"
    extra_hosts:
      - "host.docker.internal:host-gateway" # reach host DB
    volumes:
      - <CONTAINER_WORKSPACE_HOST_DIRPATH>:/workspace
      - <CONTAINER_HOME_HOST_DIRPATH>:/home:rw   # persist all user homes
      - <CONTAINER_SSH_HOST_DIRPATH>:/root/.ssh

    cap_add: [ "SYS_PTRACE" ]
    security_opt: [ "seccomp=unconfined" ]
    restart: unless-stopped
```

Bring it up:

```bash
docker compose up -d
```

Check the logs on successful container run  

```bash
docker compose logs -f ol9proc-dev
```

Output  

```
[entrypoint] SSHD_ENABLED=true
[entrypoint] CREATE_USER=true USER_NAME=<YOUR_USER> USER_UID=<USER_UID> USER_GID=<USER_GID> USER_SUDO=true TAKE_WORKSPACE=true
[entrypoint] Using provided ROOT_PASSWORD (masked)
[entrypoint] Created group name='<YOUR_USER>' gid=<USER_GID>
[entrypoint] Created user: <YOUR_USER> (uid=<USER_UID>, gid=<USER_GID>, group='<YOUR_USER>', shell=/bin/bash)
[entrypoint] Set password for <YOUR_USER> (masked)
[entrypoint] Granted sudo (NOPASSWD) to <YOUR_USER>
[entrypoint] Prepared home=/home/<YOUR_USER> (uid=<USER_UID>, gid=<USER_GID>) and SSH dir for <YOUR_USER>
[entrypoint] Ownership of /workspace -> <USER_UID>:<USER_GID>
[entrypoint] SSHD started on port 22
[entrypoint] Login:
[entrypoint]  - root: ssh -p <port> root@<host>
[entrypoint]  - <YOUR_USER>: ssh -p <port> <YOUR_USER>@<host>
```


## Connecting via SSH

```bash
# root login
ssh -p 2222 root@localhost

# dev user login
ssh -p 2222 <YOUR_USER>@localhost
```

Passwords come from `ROOT_PASSWORD` and `USER_PASSWORD`.  
If `ROOT_PASSWORD` is omitted, a random one is generated and shown in logs:  
```bash
docker logs ol9proc-dev | grep ROOT_PASSWORD
```



## VS Code Remote SSH setup

To use VS Code Remote SSH, add this to your `~/.ssh/config` on the host:

```
Host ol9proc-dev
  HostName localhost
  Port 2222
  User <YOUR_USER>
  ForwardAgent yes
  StrictHostKeyChecking no
  UserKnownHostsFile=/dev/null
```

Then in VS Code, select **Remote-SSH: Connect to Host… → ol9proc-dev**.  
You’ll get a full-featured development environment inside the container with your workspace at `/workspace`.



## Mini Hello World Pro*C Project

Here’s a minimal test project you can drop into `/workspace` to confirm everything works.

**CMakeLists.txt**
```cmake
cmake_minimum_required(VERSION 3.15)
project(proc_hello C)

# --- Paths from your working setup ---
set(PROC     "/opt/oracle/instantclient/bin/proc"            CACHE FILEPATH "Path to proc")
set(PCS_CFG  "/opt/oracle/instantclient/lib/precomp/admin/pcscfg.cfg" CACHE FILEPATH "Pro*C config")
set(ORA_INC  "/usr/include/oracle/23/client64"               CACHE PATH    "Oracle SDK include (sqlca.h)")
set(ORA_LIB  "/opt/oracle/instantclient/lib"                 CACHE PATH    "Oracle client lib dir")
set(PC_SRC   "${CMAKE_SOURCE_DIR}/hello.pc"                  CACHE FILEPATH "Pro*C source")

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

set(GENERATED_C "${CMAKE_CURRENT_BINARY_DIR}/hello_proc.c")

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

add_executable(hello_c "${GENERATED_C}")
target_include_directories(hello_c PRIVATE "${ORA_INC}")

# Link Oracle client
target_link_directories(hello_c PRIVATE "${ORA_LIB}")
target_link_libraries(hello_c PRIVATE clntsh)

# RPATH so runtime finds libclntsh
set_target_properties(hello_c PROPERTIES
  BUILD_RPATH   "${ORA_LIB}"
  INSTALL_RPATH "${ORA_LIB}"
)

```

**hello.pc**
```c
#include <stdio.h>

EXEC SQL INCLUDE sqlca;

int main() {
    printf("Hello from Pro*C!\n");
    return 0;
}
```

**hello_proc.c** is generated automatically by `proc` at build time.



### Build and run inside container

```bash
cd /cpp-proc-docker/ol9proc/examples

# use build-cmake.sh script
./build-cmake.sh

# or using cmake commands
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# run the executable
./build/hello_c
```

Expected output:
```
Hello from Pro*C!
```


## Development workflow

- Place code in `/workspace` (bind-mounted from host).  
- For normal C/C++: use CMake + Ninja with GCC or Clang.  
- For Pro*C: run `proc` via CMake custom commands.  
- Debug with GDB or VS Code Remote SSH.

## Oracle23ai Docker Container Setup
You will need working Oracle 23ai db to work on Pro*C/C++.    
I have prepare docker compose file to prepare working container and   
persistent volume docker container for Oracle 23ai.  
  
- Goto folder `oracle23aif`
- Modify docker-compose.yaml to your docker configurations
- Run with command
   ```bash
  docker compose up -d
  ```
  
- Open container log using
  ```bash
  docker logs -f oracle23aif
  ```
   
   
  If you found error `DB NULL PASSWORD` change the password using `docker exec oracle23aif ./setPassword.sh <PASSWORD>` command.  
  Note: change the password as per your docker-compose.yaml.
     
    
   If you found error creating DB inside container log
  ```
  Prepare for db operation
  Cannot create directory "/opt/oracle/oradata/FREE".
  ```
  Execute this command, and restart the container
  ```bash
  
  # Make it owned by the oracle user used in the image
  sudo chown -R 54321:54321 <HOST_MAP_VOLUME_PATH>/oracle23aif
  sudo chmod -R 775 <HOST_MAP_VOLUME_PATH>/oracle23aif
  docker container stop oracle23aif
  docker container start oracle23aif
  ```
  Check the log container, make sure the `oradata` can be written
  ```
  Prepare for db operation
  7% complete
  Copying database files
  29% complete
  Creating and starting Oracle instance
  43% complete
  Completing Database Creation
  50% complete
  Creating Pluggable Databases
  71% complete
  Executing Post Configuration Actions
  93% complete
  Running Custom Scripts
  100% complete
  Database creation complete. For details check the logfiles at:
   /opt/oracle/cfgtoollogs/dbca/FREE.
  ```

    
- If you are encountered error `DB NULL PASSWORD` or want to change the DB Password after container is run, execute
  ```bash
  docker exec oracle23aif ./setPassword.sh <YOUR_PASSWD_IN_DOCKER_COMPOSE>
  ```
  Verified the container log if the password has been changed successfully.
- Connect sqlplus or any db admin gui to the Oracle 23ai DB Container
    
  From another Dev Container (eg: ol9proc-dev)
  ```
  sqlplus user/pass@//host.docker.internal:1521/FREEPDB1
  ```
    
  You must enable option in your docker-compose.yaml to be able for dev container connect to your Oracle DB Container.
     
  ```yaml
  extra_hosts:
  - "host.docker.internal:host-gateway"
  ```
    
  If you are connecting from inside Oracle DB Container, use this command  
  ```
  sqlplus user/pass@//localhost:1521/FREEPDB1
  ```


  

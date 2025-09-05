# C, C++, and Pro*C/C++ Docker Development Container

This container provides a **ready-to-use development environment** for:  
- Standard **C and C++ projects**  
- Oracle **Pro*C/C++ applications** (via Oracle Instant Client + precompiler)
- CMAKE & GDB Ready 

It’s built for developers who want a consistent workspace with modern compilers, CMake/Ninja builds, debugging tools, and Oracle database connectivity without tedious setup.  

You can develop as **root** or create a dedicated **developer user** (with its own home, sudo, and persistent workspace).



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
      - <WCONTAINER_WORKSPACE_HOST_DIRPATH>:/workspace
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

# Run proc to convert .pc → .c
add_custom_command(
  OUTPUT hello_proc.c
  COMMAND proc iname=${CMAKE_SOURCE_DIR}/hello.pc oname=hello_proc.c
  DEPENDS hello.pc
  COMMENT "Running Pro*C precompiler"
)

add_executable(proc_hello hello_proc.c)
target_include_directories(proc_hello PRIVATE $ENV{ORACLE_HOME}/precomp/public)
target_link_libraries(proc_hello PRIVATE clntsh)
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
cd /workspace
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/proc_hello
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

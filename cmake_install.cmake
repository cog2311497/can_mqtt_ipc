# cmake_install.cmake
# Equivalent of the install.sh bash script.
# Usage:
#   cmake -P cmake_install.cmake
#   cmake -DDEST=/custom/path -P cmake_install.cmake
#
# Requires: CMake 3.16+ (for file(GET_RUNTIME_DEPENDENCIES) and GLOB globbing).
# The script must be run from the project root (same CWD as the original bash script).

cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Configurable paths (override via -D on the command line)
# ---------------------------------------------------------------------------
if(NOT DEFINED DEST)
  set(DEST "/opt/can_mqtt_ipc")
endif()

if(NOT DEFINED SYS_DEST)
  set(SYS_DEST "/etc/systemd/system")
endif()

message(STATUS "Installing to ${DEST}...")

# ---------------------------------------------------------------------------
# Helper: run a command with sudo, abort on failure
# ---------------------------------------------------------------------------
macro(sudo_exec)
  execute_process(
    COMMAND sudo ${ARGN}
    RESULT_VARIABLE _rc
  )
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "Command failed (exit ${_rc}): sudo ${ARGN}")
  endif()
endmacro()

# ---------------------------------------------------------------------------
# 1. Clean and recreate destination tree
# ---------------------------------------------------------------------------
sudo_exec(mkdir -p "${DEST}")
sudo_exec(find "${DEST}" -mindepth 1 -delete)   # rm -rf $DEST/*
sudo_exec(mkdir -p
  "${DEST}/presenter"
  "${DEST}/bridge"
  "${DEST}/producer"
)

# ---------------------------------------------------------------------------
# 2. Helper macros: copy_file / copy_exec / copy_dir
# ---------------------------------------------------------------------------
macro(copy_file SRC DST)
  sudo_exec(install -m 644 "${SRC}" "${DST}")
  message(STATUS "Copied: ${SRC} -> ${DST}")
endmacro()

macro(copy_exec SRC DST)
  sudo_exec(install -m 755 "${SRC}" "${DST}")
  message(STATUS "Installed executable: ${SRC} -> ${DST}")
endmacro()

macro(copy_dir SRC DST)
  # Replicates: sudo cp -r $SRC/* $DST/
  sudo_exec(cp -r "${SRC}/." "${DST}/")
  message(STATUS "Copied directory: ${SRC} -> ${DST}")
endmacro()

# ---------------------------------------------------------------------------
# 3. Runtime files
# ---------------------------------------------------------------------------
copy_file("config.json"                    "${DEST}/")
copy_exec("build/bridge/bridge"            "${DEST}/bridge/")
copy_exec("build/producer/producer"        "${DEST}/producer/")
copy_file("presenter/presenter/main.py"    "${DEST}/presenter/")

# ---------------------------------------------------------------------------
# 4. Virtual environment
# ---------------------------------------------------------------------------
sudo_exec(mkdir -p "${DEST}/presenter/venv")

# Locate the venv directory: build/presenter/presenter-*-py*
file(GLOB _VENV_CANDIDATES LIST_DIRECTORIES true
  "${CMAKE_CURRENT_LIST_DIR}/build/presenter/presenter-*-py*"
)

# Filter to directories only
set(_VENV_DIR "")
foreach(_candidate IN LISTS _VENV_CANDIDATES)
  if(IS_DIRECTORY "${_candidate}")
    set(_VENV_DIR "${_candidate}")
    break()
  endif()
endforeach()

if(_VENV_DIR STREQUAL "")
  message(FATAL_ERROR
    "Error: No presenter virtual environment found in build/presenter/. Aborting.")
endif()

copy_dir("${_VENV_DIR}" "${DEST}/presenter/venv")

# ---------------------------------------------------------------------------
# 5. Systemd service files
# ---------------------------------------------------------------------------
copy_file("services/vcan.service"          "${SYS_DEST}/vcan.service")
copy_file("services/vcan-iface.target"     "${SYS_DEST}/vcan-iface.target")
copy_file("services/vcan-iface@.service"   "${SYS_DEST}/vcan-iface@.service")
copy_file("services/producer.service"      "${SYS_DEST}/producer.service")
copy_file("services/bridge.service"        "${SYS_DEST}/bridge.service")
copy_file("services/presenter.service"     "${SYS_DEST}/presenter.service")

# ---------------------------------------------------------------------------
# 6. Reload systemd
# ---------------------------------------------------------------------------
message(STATUS "Reloading systemd...")
sudo_exec(systemctl daemon-reload)

# ---------------------------------------------------------------------------
# 7. Read CAN interfaces from config.json and enable services
# ---------------------------------------------------------------------------
# Use cmake -E env to call jq; fall back gracefully if jq is absent.
execute_process(
  COMMAND jq -r ".can_interfaces[]" "${CMAKE_CURRENT_LIST_DIR}/config.json"
  OUTPUT_VARIABLE _INTERFACES_RAW
  RESULT_VARIABLE _JQ_RC
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

message(STATUS "Enabling and starting services...")
sudo_exec(systemctl enable --now vcan.service)

if(_JQ_RC EQUAL 0 AND NOT _INTERFACES_RAW STREQUAL "")
  # Split newline-separated output into a CMake list
  string(REPLACE "\n" ";" _INTERFACES "${_INTERFACES_RAW}")
  foreach(_iface IN LISTS _INTERFACES)
    if(NOT _iface STREQUAL "")
      sudo_exec(systemctl enable --now "vcan-iface@${_iface}.service")
    endif()
  endforeach()
else()
  message(WARNING "Could not read can_interfaces from config.json — skipping per-interface service activation.")
endif()

sudo_exec(systemctl enable --now producer.service)
sudo_exec(systemctl enable --now bridge.service)
sudo_exec(systemctl enable --now presenter.service)

message(STATUS "Installation complete.")

# cmake_uninstall.cmake
# Equivalent of the uninstall.sh bash script.
# Usage:
#   cmake -P cmake_uninstall.cmake
#   cmake -DDEST=/custom/path -P cmake_uninstall.cmake
#
# Requires: CMake 3.16+
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

# ---------------------------------------------------------------------------
# Helper: run a command with sudo, ignore failures (mirrors || true)
# ---------------------------------------------------------------------------
macro(sudo_exec_optional)
  execute_process(
    COMMAND sudo ${ARGN}
    RESULT_VARIABLE _rc
  )
  # Intentionally not checking _rc — matches bash "|| true" behaviour
endmacro()

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
# 1. Read CAN interfaces from config.json (optional)
# ---------------------------------------------------------------------------
set(_INTERFACES "")
set(_CONFIG "${CMAKE_CURRENT_LIST_DIR}/config.json")

if(EXISTS "${_CONFIG}")
  execute_process(
    COMMAND jq -r ".can_interfaces[]" "${_CONFIG}"
    OUTPUT_VARIABLE _INTERFACES_RAW
    RESULT_VARIABLE _JQ_RC
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(_JQ_RC EQUAL 0 AND NOT _INTERFACES_RAW STREQUAL "")
    string(REPLACE "\n" ";" _INTERFACES "${_INTERFACES_RAW}")
  endif()
endif()

# ---------------------------------------------------------------------------
# 2. Stop services
# ---------------------------------------------------------------------------
message(STATUS "Stopping and disabling services...")

sudo_exec_optional(systemctl stop presenter.service)
sudo_exec_optional(systemctl stop bridge.service)
sudo_exec_optional(systemctl stop producer.service)

foreach(_iface IN LISTS _INTERFACES)
  if(NOT _iface STREQUAL "")
    sudo_exec_optional(systemctl stop "vcan-iface@${_iface}.service")
  endif()
endforeach()

sudo_exec_optional(systemctl stop vcan.service)

# ---------------------------------------------------------------------------
# 3. Disable services
# ---------------------------------------------------------------------------
sudo_exec_optional(systemctl disable presenter.service)
sudo_exec_optional(systemctl disable bridge.service)
sudo_exec_optional(systemctl disable producer.service)

foreach(_iface IN LISTS _INTERFACES)
  if(NOT _iface STREQUAL "")
    sudo_exec_optional(systemctl disable "vcan-iface@${_iface}.service")
  endif()
endforeach()

sudo_exec_optional(systemctl disable vcan.service)

# ---------------------------------------------------------------------------
# 4. Remove systemd unit files
# ---------------------------------------------------------------------------
message(STATUS "Removing systemd unit files...")

foreach(_unit IN ITEMS
  "${SYS_DEST}/vcan.service"
  "${SYS_DEST}/vcan-iface.target"
  "${SYS_DEST}/vcan-iface@.service"
  "${SYS_DEST}/producer.service"
  "${SYS_DEST}/bridge.service"
  "${SYS_DEST}/presenter.service"
)
  sudo_exec_optional(rm -f "${_unit}")
endforeach()

# ---------------------------------------------------------------------------
# 5. Reload systemd
# ---------------------------------------------------------------------------
message(STATUS "Reloading systemd...")
sudo_exec(systemctl daemon-reload)
sudo_exec_optional(systemctl reset-failed)

# ---------------------------------------------------------------------------
# 6. Remove application directory
# ---------------------------------------------------------------------------
message(STATUS "Removing application directory ${DEST}")
sudo_exec(rm -rf "${DEST}")

message(STATUS "Uninstall complete.")
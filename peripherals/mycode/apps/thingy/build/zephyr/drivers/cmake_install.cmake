# Install script for directory: /home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/zephyr/drivers

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/mnguyen4000/zephyr_install/zephyr-sdk-0.16.5/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/disk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/interrupt_controller/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/misc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/pcie/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/usb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/usb_c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/clock_control/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/console/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/entropy/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/gpio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/i2c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/pinctrl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/regulator/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/sensor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/serial/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/mnguyen4000/CSSE4011-Vulcan-Yellow/peripherals/mycode/apps/thingy/build/zephyr/drivers/timer/cmake_install.cmake")
endif()


# Install script for directory: D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/LibMySQL")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE OPTIONAL FILES
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/COPYING"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/LICENSE.mysql"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Readme" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/README")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Documentation" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/docs" TYPE FILE FILES
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/Docs/INFO_SRC"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/Docs/INFO_BIN"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/zlib/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/extra/yassl/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/extra/yassl/taocrypt/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/include/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/dbug/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/strings/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/vio/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/regex/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/mysys/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/mysys_ssl/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/libmysql/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/extra/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/scripts/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/testclients/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/support-files/cmake_install.cmake")
  include("D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/packaging/WiX/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

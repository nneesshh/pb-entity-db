# Install script for directory: D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include

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

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/mysql.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/mysql_com.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_command.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/mysql_time.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_list.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_alloc.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/typelib.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/binary_log_types.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_dbug.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/m_string.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_sys.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_xml.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/mysql_embed.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_thread.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_thread_local.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/decimal.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/errmsg.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_global.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_getopt.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/sslopt-longopts.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_dir.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/sslopt-vars.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/sslopt-case.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/sql_common.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/keycache.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/m_ctype.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_compiler.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/mysql_com_server.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/my_byteorder.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/byte_order_generic.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/byte_order_generic_x86.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/little_endian.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/big_endian.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/thr_cond.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/thr_mutex.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/thr_rwlock.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/include/mysql_version.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/include/my_config.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/include/mysqld_ername.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/include/mysqld_error.h"
    "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/build/include/sql_state.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Development" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mysql" TYPE DIRECTORY FILES "D:/dd/sdk32/support/mysql/mysql-connector-c-6.1.10-src/include/mysql/" REGEX "/[^/]*\\.h$" REGEX "/psi\\_abi[^/]*$" EXCLUDE)
endif()


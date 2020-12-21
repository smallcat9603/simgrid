# Install script for directory: /github/simgrid/simgrid-3.26

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/opt/simgrid3.26")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpimain" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpimain")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpimain"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/simgrid" TYPE EXECUTABLE FILES "/github/simgrid/simgrid-3.26/build/lib/simgrid/smpimain")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpimain" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpimain")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpimain"
         OLD_RPATH "/github/simgrid/simgrid-3.26/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpimain")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpireplaymain" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpireplaymain")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpireplaymain"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/simgrid" TYPE EXECUTABLE FILES "/github/simgrid/simgrid-3.26/build/lib/simgrid/smpireplaymain")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpireplaymain" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpireplaymain")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpireplaymain"
         OLD_RPATH "/github/simgrid/simgrid-3.26/build/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/simgrid/smpireplaymain")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/simgrid/html/" TYPE DIRECTORY OPTIONAL FILES "/github/simgrid/simgrid-3.26/build/doc/html/")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES
    "/github/simgrid/simgrid-3.26/build/bin/smpicc"
    "/github/simgrid/simgrid-3.26/build/bin/smpicxx"
    "/github/simgrid/simgrid-3.26/build/bin/smpirun"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES
    "/github/simgrid/simgrid-3.26/build/bin/smpif90"
    "/github/simgrid/simgrid-3.26/build/bin/smpiff"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE PROGRAM FILES "/github/simgrid/simgrid-3.26/build/include/smpi/mpi.mod")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM FILES "/github/simgrid/simgrid-3.26/build/bin/tesh")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "simgrid-colorizer" FILES "/github/simgrid/simgrid-3.26/tools/MSG_visualization/colorize.pl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "simgrid_update_xml" FILES "/github/simgrid/simgrid-3.26/tools/simgrid_update_xml.pl")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE PROGRAM RENAME "simgrid_convert_TI_traces" FILES "/github/simgrid/simgrid-3.26/tools/simgrid_convert_TI_traces.py")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so.3.26" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so.3.26")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so.3.26"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/github/simgrid/simgrid-3.26/build/lib/libsimgrid.so.3.26")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so.3.26" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so.3.26")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so.3.26")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/github/simgrid/simgrid-3.26/build/lib/libsimgrid.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libsimgrid.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/github/simgrid/simgrid-3.26/build/simgrid.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/actor.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/barrier.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/comm.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/engine.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/exec.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/Exception.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/chrono.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/plugins" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/plugins/dvfs.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/plugins" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/plugins/energy.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/plugins" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/plugins/file_system.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/plugins" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/plugins/live_migration.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/plugins" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/plugins/load.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/smpi/replay.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/instr.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/mailbox.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/simdag.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/modelchecker.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/forward.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/simix.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/simix.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/simix" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/simix/blocking_simcall.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/future.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/disk.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/host.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/link.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/cond.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/mutex.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/semaphore.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/storage.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/vm.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/zone.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Activity.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Actor.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Barrier.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Comm.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/ConditionVariable.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Disk.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Engine.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Exec.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Host.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Io.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Link.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Mailbox.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Mutex.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/NetZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Semaphore.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/Storage.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/s4u" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u/VirtualMachine.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/s4u.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/resource" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/resource/Action.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/resource" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/resource/Model.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/resource" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/resource/Resource.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/ClusterZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/DijkstraZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/DragonflyZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/EmptyZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/FatTreeZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/FloydZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/FullZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/NetPoint.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/NetZoneImpl.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/RoutedZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/TorusZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/VivaldiZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid/kernel/routing" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/simgrid/kernel/routing/WifiZone.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/mpi.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/sampi.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/smpi.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/smpi_main.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/smpi_helpers.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/smpi_helpers_internal.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/smpi_extended_traces.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/smpi_extended_traces_fortran.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/smpi/forward.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/asserts.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/automaton.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/automaton.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/backtrace.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/base.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/config.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/config.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/dict.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/dynar.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/ex.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/Extendable.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/file.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/functional.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/function_types.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/future.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/graph.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/log.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/log.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/mallocator.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/misc.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/module.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/PropertyHolder.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/parmap.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/parse_units.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/range.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/random.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/replay.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/signal.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/str.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/string.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/synchro.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/sysdep.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/system_error.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/utility.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/virtu.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/xbt" TYPE FILE FILES "/github/simgrid/simgrid-3.26/include/xbt/xbt_os_time.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/smpi" TYPE FILE FILES "/github/simgrid/simgrid-3.26/build/include/smpi/mpif.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/build/include/simgrid/config.h")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/simgrid" TYPE FILE FILES "/github/simgrid/simgrid-3.26/build/include/simgrid/version.h")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/github/simgrid/simgrid-3.26/build/examples/c/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/s4u/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/smpi/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/smpi/NAS/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/smpi/smpi_s4u_masterworker/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/smpi/replay_multiple/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/smpi/replay_multiple_manual_deploy/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/python/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/deprecated/java/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/deprecated/msg/mc/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/examples/deprecated/simdag/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/java/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/kernel/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/lua/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/mc/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/msg/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/python/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/s4u/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/simdag/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/simix/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/surf/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/xbt/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/isp/umpire/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/attr/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/coll/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/comm/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/datatype/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/errhan/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/attr/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/coll/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/info/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/comm/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/datatype/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/ext/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/init/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/pt2pt/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/util/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/topo/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f77/rma/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f90/coll/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f90/datatype/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f90/info/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f90/init/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f90/pt2pt/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f90/util/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/f90/rma/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/group/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/info/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/io/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/init/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/pt2pt/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/topo/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/rma/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/teshsuite/smpi/mpich3-test/perf/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/tools/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/tools/graphicator/cmake_install.cmake")
  include("/github/simgrid/simgrid-3.26/build/tools/tesh/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/github/simgrid/simgrid-3.26/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")

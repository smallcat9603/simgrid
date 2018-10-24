#########################################
### Fill in the "make install" target ###
#########################################

# doc
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/doc/html/)
install(DIRECTORY "${CMAKE_BINARY_DIR}/doc/html/"
  DESTINATION doc/simgrid/html/)

# binaries
if(enable_smpi)
  install(PROGRAMS
    ${CMAKE_BINARY_DIR}/bin/smpicc
    ${CMAKE_BINARY_DIR}/bin/smpicxx
    ${CMAKE_BINARY_DIR}/bin/smpirun
    DESTINATION bin/)
  if(SMPI_FORTRAN)
    install(PROGRAMS
      ${CMAKE_BINARY_DIR}/bin/smpif90
      ${CMAKE_BINARY_DIR}/bin/smpiff
      DESTINATION bin/)
    install(PROGRAMS
      ${CMAKE_BINARY_DIR}/include/smpi/mpi.mod
      DESTINATION include/smpi/)
  endif()
endif()

install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/tesh  DESTINATION bin/)

install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/graphicator  DESTINATION bin/)

install(PROGRAMS ${CMAKE_HOME_DIRECTORY}/tools/MSG_visualization/colorize.pl
  DESTINATION bin/
  RENAME simgrid-colorizer)

add_custom_target(simgrid-colorizer ALL
  COMMENT "Install ${CMAKE_BINARY_DIR}/bin/colorize"
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/tools/MSG_visualization/colorize.pl ${CMAKE_BINARY_DIR}/bin/colorize)

install(PROGRAMS ${CMAKE_HOME_DIRECTORY}/tools/simgrid_update_xml.pl
  DESTINATION bin/
  RENAME simgrid_update_xml)

add_custom_target(simgrid_update_xml ALL
  COMMENT "Install ${CMAKE_BINARY_DIR}/bin/simgrid_update_xml"
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/tools/simgrid_update_xml.pl ${CMAKE_BINARY_DIR}/bin/simgrid_update_xml)

install(PROGRAMS ${CMAKE_HOME_DIRECTORY}/tools/simgrid_convert_TI_traces.py
  DESTINATION bin/
  RENAME simgrid_convert_TI_traces)

add_custom_target(simgrid_convert_TI_traces ALL
    COMMENT "Install ${CMAKE_BINARY_DIR}/bin/simgrid_convert_TI_traces"
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/tools/simgrid_convert_TI_traces.py ${CMAKE_BINARY_DIR}/bin/simgrid_convert_TI_traces)

# libraries
install(TARGETS simgrid DESTINATION lib/)

if(enable_java)
  set(SIMGRID_JAR_TO_INSTALL "${SIMGRID_JAR}")
  install(TARGETS simgrid-java   DESTINATION lib/)
  install(FILES ${SIMGRID_JAR_TO_INSTALL}
      DESTINATION java/
      RENAME simgrid.jar)
endif()

# include files
foreach(file ${headers_to_install}  ${generated_headers_to_install})
  get_filename_component(location ${file} PATH)
  string(REPLACE "${CMAKE_CURRENT_BINARY_DIR}/" "" location "${location}")
  install(FILES ${file} DESTINATION ${location})
endforeach()

# example files
foreach(file ${examples_to_install})
  string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/examples/" "" file ${file})
  get_filename_component(location ${file} PATH)
  install(FILES "examples/${file}"
    DESTINATION doc/simgrid/examples/${location})
endforeach(file ${examples_to_install})

###########################################
### Fill in the "make uninstall" target ###
###########################################

add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/doc/simgrid
  COMMAND ${CMAKE_COMMAND} -E	echo "uninstall doc ok"
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/lib/libsimgrid*
  COMMAND ${CMAKE_COMMAND} -E   remove -f ${CMAKE_INSTALL_PREFIX}/lib/lua/5.1/simgrid*
  COMMAND ${CMAKE_COMMAND} -E	echo "uninstall lib ok"
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/smpicc
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/smpicxx
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/smpiff
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/smpif90
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/smpirun
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/tesh
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/simgrid-colorizer
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/simgrid_update_xml
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/simgrid_convert_TI_traces
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/bin/graphicator
  COMMAND ${CMAKE_COMMAND} -E	echo "uninstall bin ok"
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/instr
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/msg
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/simdag
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/smpi
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/simix
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/surf
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/xbt
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/mc
  COMMAND ${CMAKE_COMMAND} -E	remove_directory ${CMAKE_INSTALL_PREFIX}/include/simgrid
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/include/simgrid.h
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/include/simgrid/config.h
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/include/xbt.h
  COMMAND ${CMAKE_COMMAND} -E	echo "uninstall include ok"
  COMMAND ${CMAKE_COMMAND} -E	remove -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/simgrid_update_xml.1
  COMMAND ${CMAKE_COMMAND} -E   remove -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/tesh.1
  COMMAND ${CMAKE_COMMAND} -E   remove -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/smpicc.1
  COMMAND ${CMAKE_COMMAND} -E   remove -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/smpicxx.1
  COMMAND ${CMAKE_COMMAND} -E   remove -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/smpirun.1
  COMMAND ${CMAKE_COMMAND} -E   remove -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/smpiff.1
  COMMAND ${CMAKE_COMMAND} -E   remove -f ${CMAKE_INSTALL_PREFIX}/share/man/man1/smpif90.1


  COMMAND ${CMAKE_COMMAND} -E	echo "uninstall man ok"
  WORKING_DIRECTORY "${CMAKE_INSTALL_PREFIX}")

if(SIMGRID_HAVE_LUA)
  add_custom_command(TARGET uninstall
    COMMAND ${CMAKE_COMMAND} -E echo "uninstall binding lua ok"
    COMMAND ${CMAKE_COMMAND} -E remove -f ${CMAKE_INSTALL_PREFIX}/lib/lua/5.1/simgrid.${LIB_EXE}
    WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}/")
endif()

################################################################
## Build a sain "make dist" target to build a source package ###
##   containing only the files that I explicitely state      ###
##   (instead of any cruft laying on my disk as CPack does)  ###
################################################################

# This is the complete list of what will be added to the source archive
set(source_to_pack
  ${headers_to_install}
  ${source_of_generated_headers}
  ${BINDINGS_SRC}
  ${JEDULE_SRC}
  ${JMSG_C_SRC}
  ${JMSG_JAVA_SRC}
  ${LUA_SRC}
  ${MC_SRC_BASE}
  ${MC_SRC}
  ${MC_SIMGRID_MC_SRC}
  ${MSG_SRC}
  ${S4U_SRC}
  ${NS3_SRC}
  ${PLUGINS_SRC}
  ${RNGSTREAM_SRC}
  ${SIMDAG_SRC}
  ${SIMGRID_SRC}
  ${SIMIX_SRC}
  ${SMPI_SRC}
  ${SURF_SRC}
  ${TRACING_SRC}
  ${XBT_RL_SRC}
  ${XBT_SRC}
  ${EXTRA_DIST}
  ${CMAKE_SOURCE_FILES}
  ${CMAKEFILES_TXT}
  ${DOC_FIGS}
  ${DOC_IMG}
  ${DOC_SOURCES}
  ${DOC_TOOLS}
  ${PLATFORMS_EXAMPLES}
  ${README_files}
  ${bin_files}
  ${examples_src}
  ${tesh_files}
  ${teshsuite_src}
  ${testsuite_src}
  ${tools_src}
  ${txt_files}
  ${xml_files}
  )

##########################################
### Fill in the "make dist-dir" target ###
##########################################

add_custom_target(dist-dir
  COMMENT "Generating the distribution directory"
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_NAME}-${release_version}/
  COMMAND ${CMAKE_COMMAND} -E remove ${PROJECT_NAME}-${release_version}.tar.gz
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_NAME}-${release_version}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_NAME}-${release_version}/doc/html/
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/doc/html/ ${PROJECT_NAME}-${release_version}/doc/html/
  COMMAND rm -f `grep -rl " Reference" ${PROJECT_NAME}-${release_version}/doc/html/` # Doxygen, go away
  COMMAND rm -f `grep -rl "Member List" ${PROJECT_NAME}-${release_version}/doc/html/` # Doxygen, you're getting annoying
  )
add_dependencies(dist-dir maintainer_files)

set(dirs_in_tarball "")
foreach(file ${source_to_pack})
  #message(${file})
  # This damn prefix is still set somewhere (seems to be in subdirs)
  string(REPLACE "${CMAKE_HOME_DIRECTORY}/" "" file "${file}")

  # Create the directory on need
  get_filename_component(file_location ${file} PATH)
  string(REGEX MATCH ";${file_location};" OPERATION "${dirs_in_tarball}")
  if(NOT OPERATION)
    set(dirs_in_tarball "${dirs_in_tarball};${file_location};")
    add_custom_command(
      TARGET dist-dir
      COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_NAME}-${release_version}/${file_location}/)
  endif()

  # Actually copy the file
  add_custom_command(
    TARGET dist-dir
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/${file} ${PROJECT_NAME}-${release_version}/${file_location})
endforeach(file ${source_to_pack})

add_custom_command(
  TARGET dist-dir
  COMMAND ${CMAKE_COMMAND} -E echo "${GIT_VERSION}" > ${PROJECT_NAME}-${release_version}/.gitversion)

##########################################################
### Link all sources to the bindir if srcdir != bindir ###
##########################################################
add_custom_target(hardlinks
   COMMENT "Making the source files available from the bindir"
)
if (NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  foreach(file ${source_to_pack})
    #message(${file})
    # This damn prefix is still set somewhere (seems to be in subdirs)
    string(REPLACE "${CMAKE_HOME_DIRECTORY}/" "" file "${file}")

    # Create the directory on need
    get_filename_component(file_location ${file} PATH)
    string(REGEX MATCH ";${file_location};" OPERATION "${dirs_in_bindir}")
    if(NOT OPERATION)
      set(dirs_in_tarball "${dirs_in_bindir};${file_location};")
      add_custom_command(
        TARGET hardlinks
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${file_location}/)
    endif()

    # Actually copy the file
    add_custom_command(
      TARGET hardlinks
      COMMAND if test -f ${CMAKE_HOME_DIRECTORY}/${file} \; then rm -f ${CMAKE_BINARY_DIR}/${file}\; ln ${CMAKE_HOME_DIRECTORY}/${file} ${CMAKE_BINARY_DIR}/${file_location}\; fi
    )
  endforeach(file ${source_to_pack})
endif()


######################################
### Fill in the "make dist" target ###
######################################

add_custom_target(dist
  COMMENT "Removing the distribution directory"
  DEPENDS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${release_version}.tar.gz
  COMMAND ${CMAKE_COMMAND} -E echo ${PROJECT_NAME}-${release_version} > ${CMAKE_BINARY_DIR}/VERSION
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_NAME}-${release_version}/)

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-${release_version}.tar.gz
  COMMENT "Compressing the archive from the distribution directory"
  COMMAND ${CMAKE_COMMAND} -E tar cf ${PROJECT_NAME}-${release_version}.tar ${PROJECT_NAME}-${release_version}/
  COMMAND gzip -9v ${PROJECT_NAME}-${release_version}.tar
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_NAME}-${release_version}/)
add_dependencies(dist dist-dir)

if(NOT enable_maintainer_mode)
  add_custom_target(echo-dist
    COMMAND ${CMAKE_COMMAND} -E echo "WARNING: ----------------------------------------------------"
    COMMAND ${CMAKE_COMMAND} -E echo "WARNING: Distrib is generated without option maintainer mode "
    COMMAND ${CMAKE_COMMAND} -E echo "WARNING: ----------------------------------------------------")
  add_dependencies(dist echo-dist)
endif()

###########################################
### Fill in the "make distcheck" target ###
###########################################

set(CMAKE_BINARY_TEST_DIR ${CMAKE_BINARY_DIR})

# Allow to test the "make dist"
add_custom_target(distcheck
  COMMAND ${CMAKE_COMMAND} -E echo "XXX compare archive with git repository"
  COMMAND ${CMAKE_HOME_DIRECTORY}/tools/internal/check_dist_archive -batch ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}.tar.gz

  COMMAND ${CMAKE_COMMAND} -E echo "XXX remove old copy"
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}

  COMMAND ${CMAKE_COMMAND} -E echo "XXX Untar distrib"
  COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}.tar.gz ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}

  COMMAND ${CMAKE_COMMAND} -E echo "XXX create build and install subtrees"
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_build
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_inst

  COMMAND ${CMAKE_COMMAND} -E echo "XXX Configure"
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_build
          ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_inst -Denable_lto=OFF ..
	
  COMMAND ${CMAKE_COMMAND} -E echo "XXX Build"
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_build ${CMAKE_MAKE_PROGRAM} -j 4

  COMMAND ${CMAKE_COMMAND} -E echo "XXX Test"
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_build ctest --output-on-failure -j 4

  COMMAND ${CMAKE_COMMAND} -E echo "XXX Install"
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_build ${CMAKE_MAKE_PROGRAM} install
  COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_inst/lib/libsimgrid.so
  ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_inst/lib/libsimgridtest.so

  COMMAND ${CMAKE_COMMAND} -E echo "XXX Install with documentation"
  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}/_build ${CMAKE_MAKE_PROGRAM} install

  COMMAND ${CMAKE_COMMAND} -E echo "XXX Remove temp directories"
  COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_TEST_DIR}/${PROJECT_NAME}-${release_version}
  )
add_dependencies(distcheck dist)

#######################################
### Fill in the "make check" target ###
#######################################

if(enable_memcheck)
  add_custom_target(check COMMAND ctest -D ExperimentalMemCheck)
else()
  add_custom_target(check COMMAND make test)
endif()

#######################################
### Fill in the "make xxx-clean" target ###
#######################################

add_custom_target(maintainer-clean
  COMMAND ${CMAKE_COMMAND} -E remove -f src/config_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/cunit_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/dict_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/dynar_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/ex_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/set_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/simgrid_units_main.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/xbt_str_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/xbt_synchro_unit.c
  COMMAND ${CMAKE_COMMAND} -E remove -f src/mpif.f90
  WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}")

include(CPack)

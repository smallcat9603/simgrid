!include "MUI2.nsh"

;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "/home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\doc\webcruft\simgrid_logo_win_2011.bmp" ; optional
  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "/home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\LICENSE-LGPL-2.1"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages
  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
;--------------------------------

Name "Simgrid"
outFile "SimGrid-3.12.0_SGjava_"
Icon "/home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\doc\webcruft\SGicon.ico"

RequestExecutionLevel admin

# set the default installation directory
InstallDir c:\SimGrid3.12.0

Section "Libraries and Headers" LibSection

	setOutPath $INSTDIR
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\AUTHORS
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\Changelog
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\COPYING
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\LICENSE-LGPL-2.1
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\NEWS

	# install lib
	CreateDirectory $INSTDIR\lib
	setOutPath $INSTDIR\lib
	file lib\libsimgrid.dll
	file lib\libsimgrid.def
	
	#install headers
	CreateDirectory  $INSTDIR\include
	setOutPath $INSTDIR\include
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt.h
	file include\simgrid_config.h
	
	CreateDirectory  $INSTDIR\include\xbt
	setOutPath $INSTDIR\include\xbt
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\misc.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\sysdep.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\virtu.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\str.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\strbuff.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\hash.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\function_types.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\asserts.h 
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\ex.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\log.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\module.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\mallocator.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\dynar.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\dict.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\set.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\heap.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\graph.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\fifo.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\swag.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\lib.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\matrix.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\peer.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\config.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\cunit.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\graphxml_parse.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\graphxml.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\synchro_core.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\queue.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\setset.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\mmalloc.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\parmap.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\automaton.h
    file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\xbt_os_thread.h
    file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\xbt\RngStream.h

	CreateDirectory  $INSTDIR\include\simgrid
	setOutPath $INSTDIR\include\simgrid
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\simgrid\platf.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\simgrid\modelchecker.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\simgrid\simix.h

	CreateDirectory  $INSTDIR\include\msg
	setOutPath $INSTDIR\include\msg
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\msg\msg.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\msg\datatypes.h
	
	CreateDirectory  $INSTDIR\include\simdag
	setOutPath $INSTDIR\include\simdag
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\simdag\simdag.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\simdag\datatypes.h

	
	CreateDirectory  $INSTDIR\include\surf
	setOutPath $INSTDIR\include\surf
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\surf\surfxml_parse.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\surf\simgrid_dtd.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\surf\surf_routing.h
	
	CreateDirectory  $INSTDIR\include\instr
	setOutPath $INSTDIR\include\instr
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\instr\instr.h
	
SectionEnd
Section "Binaries" BinSection	
	
	# insatll bin
	CreateDirectory $INSTDIR\bin
	setOutPath $INSTDIR\bin
	file bin\colorize
	file bin\graphicator
	file bin\simgrid_update_xml

	
SectionEnd
Section "Documentation" DocSection	
	
	# install doc
	CreateDirectory $INSTDIR\doc
	setOutPath $INSTDIR\doc
	file /nonfatal /r /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\doc\html
	# create a shortcut in the start menu programs directory
	CreateDirectory "$SMPROGRAMS\SimGrid 3.12.0"
	createShortCut  "$SMPROGRAMS\SimGrid 3.12.0\Documentation.lnk"	"$INSTDIR\doc\html\index.html"
	createShortCut  "$SMPROGRAMS\SimGrid 3.12.0\Website.lnk"	"http://simgrid.gforge.inria.fr/"
	
SectionEnd
Section "Examples" ExamplesSection	

	CreateDirectory $INSTDIR\examples
	setOutPath $INSTDIR\examples
	
	#install examples for platforms
	file /r /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\platforms
	
	# install example HelloWorld
	file /r /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\doc\HelloWorld
		
	# install example MasterSlave
	CreateDirectory $INSTDIR\examples\MasterSlave
	setOutPath $INSTDIR\examples\MasterSlave
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\msg\masterslave\masterslave_forwarder.c
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\msg\masterslave\deployment_masterslave_forwarder.xml
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\platforms\platform.xml
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\doc\HelloWorld\CMakeLists.txt

	# create shortcuts in the start menu programs directory
	CreateDirectory "$SMPROGRAMS\SimGrid 3.12.0\Examples\"
	createShortCut  "$SMPROGRAMS\SimGrid 3.12.0\Examples\HelloWorld project.lnk"	"$INSTDIR\examples\HelloWorld"
	createShortCut  "$SMPROGRAMS\SimGrid 3.12.0\Examples\MasterSlave project.lnk"	"$INSTDIR\examples\MasterSlave"
	
SectionEnd

Section "Java Bindings" JavaSection	
	# install java library and examples
	CreateDirectory $INSTDIR\examples\simgrid-java
	setOutPath $INSTDIR\lib
	file /nonfatal "lib\simgrid-java.dll"
	file /nonfatal "lib\simgrid-java.def"
	file /nonfatal "simgrid.jar"
	setOutPath $INSTDIR\examples\simgrid-java
	file /nonfatal /r ".\examples\"
	# create shortcuts in the start menu programs directory
	createShortCut  "$SMPROGRAMS\SimGrid 3.12.0\Examples\Java project.lnk"	"$INSTDIR\examples\simgrid-java"
	# create a popup box, with an OK button"
	messageBox MB_OK "WARNING! Please add to your environment variable CLASSPATH value '$INSTDIR\lib\simgrid.jar;.' before executing Simgrid classes."
SectionEnd

Section "SMPI Library" SMPISection
	setOutPath $INSTDIR\bin
	file bin\smpicc
	file bin\smpiff
	file bin\smpirun
	file bin\smpif90
	
    setOutPath $INSTDIR\lib
	
	CreateDirectory  $INSTDIR\include\smpi
	setOutPath $INSTDIR\include\smpi
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\smpi\smpi.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\smpi\mpi.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\smpi\smpi_cocci.h
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\include\smpi\smpi_main.h
	file include\smpi\mpif.h
	
	CreateDirectory $INSTDIR\examples\smpi
	file /r /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\smpi\tracing
	file /r /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\smpi\replay
	setOutPath $INSTDIR\examples\smpi
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\smpi\bcbench.c
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\smpi\mvmul.c
	file /r /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\smpi\mc
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\smpi\hostfile
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\platforms\small_platform_with_routers.xml
	file /home/koibuchi/simgrid/copyX20141203/MpiEnv/simgrid/Simgrid-git\examples\smpi\CMakeLists.txt
SectionEnd

# default section start
section	
	# define uninstaller name
	writeUninstaller $INSTDIR\uninstaller

	# create a shortcut in the start menu programs directory
	CreateDirectory "$SMPROGRAMS\SimGrid 3.12.0"
	createShortCut  "$SMPROGRAMS\SimGrid 3.12.0\Uninstall simgrid.lnk" "$INSTDIR\uninstaller"

	# Include for some of the windows message defines
	!include "winmessages.nsh"
	# HKLM (all users) vs HKCU (current user) defines
	!define env_hklm 'HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"'
	!define env_hkcu 'HKCU "Environment"'
	# Set Variables
	WriteRegExpandStr ${env_hkcu} SIMGRID_ROOT $INSTDIR
	WriteRegExpandStr ${env_hkcu} SIMGRID_VERSION 3.12.0
	
	WriteRegStr HKCU "SOFTWARE\SimGrid" "Version" "3.12.0"
	WriteRegStr HKCU "SOFTWARE\SimGrid" "InstallPath" "$INSTDIR"
	
	# create a popup box, with an OK button"
	messageBox MB_OK "WARNING! Please add to your environment variable PATH value '$INSTDIR\lib;$INSTDIR\GnuWin32\bin' before executing Simgrid binaries."
	
	SetRebootFlag false

# default section end
sectionEnd

LangString DESC_LibSection 		${LANG_ENGLISH} "Install Simgrid libraries with associated headers."
LangString DESC_BinSection 		${LANG_ENGLISH} "Install some useful tools for Simgrid."
LangString DESC_DocSection 		${LANG_ENGLISH} "Generated (doxygen) documentation."
LangString DESC_ExamplesSection ${LANG_ENGLISH} "Simgrid's HelloWorld example and some classical platforms."
LangString DESC_JAVASection 	${LANG_ENGLISH} "Install the Java binding and examples."
LangString DESC_SMPISection 	${LANG_ENGLISH} "Install the SMPI library, headers, and examples."


LangString DESC_LibSection 		${LANG_FRENCH} 	"Installer les librairies Simgrid et leurs Ent�tes."
LangString DESC_BinSection 		${LANG_FRENCH} 	"Installer les outils optionnels."
LangString DESC_DocSection 		${LANG_FRENCH} 	"Installer la documentation."
LangString DESC_ExamplesSection ${LANG_FRENCH} 	"Installer un exemple 'HelloWorld' et des fichiers de plate-formes types."
LangString DESC_JAVASection 	${LANG_FRENCH}  "Installer la librairie Simgrid-java et les exemples."
LangString DESC_SMPISection 	${LANG_FRENCH}  "Installer la librairie SMPI, ses en-t�tes, et ses exemples."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${LibSection} 		$(DESC_LibSection)
  !insertmacro MUI_DESCRIPTION_TEXT ${BinSection} 		$(DESC_BinSection)
  !insertmacro MUI_DESCRIPTION_TEXT ${DocSection} 		$(DESC_DocSection)
  !insertmacro MUI_DESCRIPTION_TEXT ${ExamplesSection} 	$(DESC_ExamplesSection)
  !insertmacro MUI_DESCRIPTION_TEXT ${JAVASection} 		$(DESC_JAVASection)
  !insertmacro MUI_DESCRIPTION_TEXT ${SMPISection} 		$(DESC_SMPISection)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

# create a section to define what the uninstaller does.
# the section will always be named "Uninstall"
section "Uninstall"

	# always delete uninstaller first
	delete $INSTDIR\uninstaller

	# delete installed libs
	delete $INSTDIR\lib\libsimgrid.dll
	delete $INSTDIR\lib\libsimgrid.def

	# delete installed bin
	delete $INSTDIR\bin\colorize
	delete $INSTDIR\bin\graphicator
	delete $INSTDIR\bin\simgrid_update_xml
	delete $INSTDIR\bin\smpicc
	delete $INSTDIR\bin\smpiff
	delete $INSTDIR\bin\smpirun
	delete $INSTDIR\bin\tesh
	
	# delete installed headers
	delete $INSTDIR\include\xbt.h
	delete $INSTDIR\include\simgrid_config.h
	delete $INSTDIR\include\xbt\misc.h
	delete $INSTDIR\include\xbt\sysdep.h
	delete $INSTDIR\include\xbt\virtu.h
	delete $INSTDIR\include\xbt\str.h
	delete $INSTDIR\include\xbt\strbuff.h
	delete $INSTDIR\include\xbt\hash.h
	delete $INSTDIR\include\xbt\function_types.h
	delete $INSTDIR\include\xbt\asserts.h 
	delete $INSTDIR\include\xbt\ex.h
	delete $INSTDIR\include\xbt\log.h
	delete $INSTDIR\include\xbt\module.h
	delete $INSTDIR\include\xbt\mallocator.h
	delete $INSTDIR\include\xbt\dynar.h
	delete $INSTDIR\include\xbt\dict.h
	delete $INSTDIR\include\xbt\set.h
	delete $INSTDIR\include\xbt\heap.h
	delete $INSTDIR\include\xbt\graph.h
	delete $INSTDIR\include\xbt\fifo.h
	delete $INSTDIR\include\xbt\swag.h
	delete $INSTDIR\include\xbt\lib.h
	delete $INSTDIR\include\xbt\matrix.h
	delete $INSTDIR\include\xbt\peer.h
	delete $INSTDIR\include\xbt\config.h
	delete $INSTDIR\include\xbt\cunit.h
	delete $INSTDIR\include\xbt\graphxml_parse.h
	delete $INSTDIR\include\xbt\graphxml.h
	delete $INSTDIR\include\xbt\time.h
	delete $INSTDIR\include\xbt\synchro.h
	delete $INSTDIR\include\xbt\synchro_core.h
	delete $INSTDIR\include\xbt\queue.h
	delete $INSTDIR\include\xbt\setset.h
	delete $INSTDIR\include\xbt\mmalloc.h
	delete $INSTDIR\include\xbt\replay_trace_reader.h
	delete $INSTDIR\include\xbt\parmap.h
	delete $INSTDIR\include\xbt\socket.h
	delete $INSTDIR\include\xbt\file_stat.h

	delete $INSTDIR\include\simgrid\platf.h
	delete $INSTDIR\include\simgrid\modelchecker.h
	delete $INSTDIR\include\simgrid\simix.h

	delete $INSTDIR\include\msg\msg.h
	delete $INSTDIR\include\msg\datatypes.h	
	delete $INSTDIR\include\simdag\simdag.h
	delete $INSTDIR\include\simdag\datatypes.h
	delete $INSTDIR\include\smpi\smpi.h
	delete $INSTDIR\include\smpi\mpi.h
	delete $INSTDIR\include\smpi\mpif.h
	delete $INSTDIR\include\smpi\smpi_cocci.h
	delete $INSTDIR\include\smpi\smpi_main.h
	delete $INSTDIR\include\surf\surfxml_parse.h
	delete $INSTDIR\include\surf\simgrid_dtd.h
	delete $INSTDIR\include\surf\surf_routing.h
	delete $INSTDIR\include\instr\instr.h
		
	# delete EXTRA FILES
	delete $INSTDIR\AUTHORS
	delete $INSTDIR\Changelog
	delete $INSTDIR\COPYING
	delete $INSTDIR\LICENSE-LGPL-2.1
	delete $INSTDIR\NEWS
	
	# now delete directories
	RMDir  "$INSTDIR\bin"
	RMDir  "$INSTDIR\lib"
	RMDir  "$INSTDIR\include\simix"
	RMDir  "$INSTDIR\include\instr"
	RMDir  "$INSTDIR\include\surf"
	RMDir  "$INSTDIR\include\smpi"
	RMDir  "$INSTDIR\include\simdag"
	RMDir  "$INSTDIR\include\msg"
	RMDir  "$INSTDIR\include\mc"
	RMDir  "$INSTDIR\include\xbt"
	RMDir  "$INSTDIR\include"
	RMDir  /r "$INSTDIR\doc"
	RMDir  /r "$INSTDIR\examples"
	RMDir  /r "$SMPROGRAMS\SimGrid 3.12.0"
	
	# Delete variable
	DeleteRegValue ${env_hkcu} SIMGRID_ROOT
	DeleteRegValue ${env_hkcu} SIMGRID_VERSION
    DeleteRegKey HKCU "SOFTWARE\SimGrid"
    
    # delete JAVA-bindings
    RMDir /r "$INSTDIR\simgrid-java"
    
    # delete INSTDIR
    RMDir  /r "$INSTDIR"
    
# uninstall section end
sectionEnd

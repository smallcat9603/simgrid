.. Copyright 2005-2018

.. _install:

Installing SimGrid
==================


SimGrid should work out of the box on Linux, Mac OSX, FreeBSD, and
Windows (under Windows, you need to install the Windows Subsystem
Linux to get more than the Java bindings).

Pre-compiled Packages
---------------------

Binaries for Linux
^^^^^^^^^^^^^^^^^^

On Debian or Ubuntu, simply type:

.. code-block:: shell

   apt install simgrid

If you build pre-compiled packages for other distributions, drop us an
email.

.. _install_java_precompiled:

Stable Java Package
^^^^^^^^^^^^^^^^^^^

The jar file can be retrieved from the `Release page
<https://framagit.org/simgrid/simgrid/tags>`_. This file is
self-contained, including the native components for Linux, Mac OS X and
Windows. Copy it to your project's classpath and you're set.

Nightly built Java Package
^^^^^^^^^^^^^^^^^^^^^^^^^^

For non-Windows systems (Linux, Mac OS X, or FreeBSD), head to `Jenkins <https://ci.inria.fr/simgrid/job/SimGrid>`_.
In the build history, pick the last green (or at least yellow) build that is not blinking (i.e., not currently under
build). In the list, pick a system that is close to yours, and click on the ball in the Debug row. The build artefact
will appear at the top of the resulting page.

For Windows, head to `AppVeyor <https://ci.appveyor.com/project/simgrid/simgrid>`_.
Click on the artefact link on the right, and grab your file. If the latest build failed, there will be no artefact. Then
you will need to first click on "History" at the top and look for the last successful build.

Binary Java Troubleshooting
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Here are some error messages that you may get when trying to use the
binary Java package.

Your architecture is not supported by this jarfile
   If your system is not supported, you should compile your
   own jarfile :ref:`by compiling SimGrid <install_src>` from the source.
Library not found: boost-context
   You should obviously install the ``boost-context`` library on your
   machine, for example with ``apt``.

.. _install_src:

Installing from the Source
--------------------------

Getting the Dependencies
^^^^^^^^^^^^^^^^^^^^^^^^

C++ compiler (either g++, clang, or icc).
  We use the C++11 standard, and older compilers tend to fail on
  us. It seems that g++ 5.0 or higher is required nowadays (because of
  boost).  SimGrid compiles well with `clang` or `icc` too.
Python 3.
  SimGrid should build without Python, that is only needed by our regresion test suite.
cmake (v2.8.8).
  ``ccmake`` provides a nicer graphical interface compared to ``cmake``.
  Press ``t`` in ``ccmake`` if you need to see absolutely all
  configuration options (e.g., if your python installation is not standard).
boost (at least v1.48, v1.59 recommended)
  - On Debian / Ubuntu: ``apt install libboost-dev libboost-context-dev``
  - On Max OS X with homebrew: ``brew install boost``
Java (optional):
  - Debian / Ubuntu: ``apt install default-jdk libgcj18-dev`` (or
    any version of libgcj)
  - Mac OS X or Windows: Grab a `full JDK <http://www.oracle.com/technetwork/java/javase/downloads>`_
Lua (optional -- must be v5.3)
  - SimGrid won't work with any other version of Lua.
  - Debian / Ubuntu: ``apt install liblua5.3-dev lua5.3``
  - Windows: ``choco install lua53``
  - From the source
      - You need to patch the sources to build dynamic libraries. First `download lua 5.3 <http://www.lua.org/download.html>`_
      - Open the archive: ``tar xvfz lua-5.3.*.tar.gz``
      - Enter the directory: ``cd lua-5.3*``
      - Patch the sources: ``patch -p1 < /path/to/simgrid/...../tools/lualib.patch``
      - Build and install lua: ``make linux && sudo make install``

For platform-specific details, please see below.

Getting the Sources
^^^^^^^^^^^^^^^^^^^

Grab the last **stable release** from `FramaGit
<https://framagit.org/simgrid/simgrid/tags>`_, and compile it as follows:

.. code-block:: shell

   tar xf SimGrid-3-XX.tar.gz
   cd SimGrid-*
   cmake -DCMAKE_INSTALL_PREFIX=/opt/simgrid .
   make
   make install

If you want to stay on the **bleeding edge**, get the current git version,
and recompile it as with stable archives. You may need some extra
dependencies.

.. code-block:: shell

   git clone git@framagit.org:simgrid/simgrid.git
   cd simgrid
   cmake -DCMAKE_INSTALL_PREFIX=/opt/simgrid .
   make
   make install

.. _install_src_config:
   
Build Configuration
^^^^^^^^^^^^^^^^^^^

This section is about **compile-time options**, that are very
different from :ref:`run-time options <options>`. Compile-time options
fall into two categories. **SimGrid-specific options** define which part
of the framework to compile while **Generic options** are provided by
cmake itself.

Generic build-time options
""""""""""""""""""""""""""

These options specify for example the path to various system elements
(Python path, compiler to use, etc). In most case, CMake automatically
discovers the right value for these ones, but you can set them
manually on need.  Notable such variables include ``CC`` and ``CXX``,
defining respectively the paths to the C and C++ compilers, ``CFLAGS``
and ``CXXFLAGS`` respectively specifying extra options to pass to the C
and C++ compilers, or ``PYTHON_EXECUTABLE`` specifying the path to the
python executable.

The best way to discover the exact name of the option that you need to
change is to press ``t`` in the ``ccmake`` graphical interface, as all
options are shown (and documented) in the advanced mode.

Once you know their name, there are several ways to change the values of
build-time options. You can naturally use the ccmake graphical
interface for that, or you can use environment variables, or you can
prefer the ``-D`` flag of ``cmake``.

For example, you can change the compilers with environment variables
by issuing these commands before launching cmake:

.. code-block:: shell

   export CC=gcc-5.1
   export CXX=g++-5.1

The same can be done by passing ``-D`` parameters to cmake, as follows.
Note that the ending dot is mandatory (see :ref:`install_cmake_outsrc`).

.. code-block:: shell

   cmake -DCC=clang -DCXX=clang++ .

SimGrid compilation options
"""""""""""""""""""""""""""

Here is the list of all SimGrid-specific compile-time options (the
default choice is in uppercase).

CMAKE_INSTALL_PREFIX (path)
  Where to install SimGrid (/opt/simgrid, /usr/local, or elsewhere).

enable_compile_optimizations (ON/off)
  Request the compiler to produce efficient code. You probably want to
  activate this option, unless you plan modify SimGrid itself:
  efficient code takes more time to compile, and appears mangled to debuggers.

enable_compile_warnings (on/OFF)
  Request the compiler to issue error messages whenever the source
  code is not perfectly clean. If you are a SimGrid developer, you
  have to activate this option to enforce the code quality. As a
  regular user, this option is of little use.

enable_debug (ON/off)
  Disabling this option discards all log messages of gravity
  debug or below at compile time (see @ref XBT_log). The resulting
  code is faster than if you discard these messages at
  runtime. However, it obviously becomes impossible to get any debug
  info from SimGrid if something goes wrong.

enable_documentation (ON/off)
  Generates the documentation pages.

enable_java (on/OFF)
  Generates the java bindings of SimGrid.

enable_jedule (on/OFF)
  Produces execution traces from SimDag simulations, that can then be visualized with the
  Jedule external tool.

enable_lua (on/OFF)
  Generate the lua bindings to the SimGrid internals (requires lua-5.3).

enable_lib_in_jar (ON/off)
  Embeds the native java bindings into the produced jar file.

enable_lto (ON/off)
  Enables the *Link Time Optimization* in the C++ compiler.
  This feature really speeds up the produced code, but it is fragile
  with older gcc versions.

enable_maintainer_mode (on/OFF)
  (dev only) Regenerates the XML parsers whenever the DTD is modified (requires flex and flexml).

enable_mallocators (ON/off)
  Activates our internal memory caching mechanism. This produces faster
  code, but it may fool the debuggers.

enable_model-checking (on/OFF)
  Activates the formal verification mode. This will **hinder
  simulation speed** even when the model-checker is not activated at
  run time.

enable_ns3 (on/OFF)
  Activates the ns-3 bindings. See section @ref pls_ns3.

enable_smpi (ON/off)
  Allows to run MPI code on top of SimGrid.

enable_smpi_ISP_testsuite (on/OFF)
  Adds many extra tests for the model-checker module.

enable_smpi_MPICH3_testsuite (on/OFF)
  Adds many extra tests for the MPI module.

Reset the build configuration
"""""""""""""""""""""""""""""

To empty the CMake cache (either when you add a new library or when
things go seriously wrong), simply delete your ``CMakeCache.txt``. You
may also want to directly edit this file in some circumstances.

.. _install_cmake_outsrc:

Out of Tree Compilation
^^^^^^^^^^^^^^^^^^^^^^^

By default, the files produced during the compilation are placed in
the source directory. It is however often better to put them all in a
separate directory: cleaning the tree becomes as easy as removing this
directory, and you can have several such directories to test several
parameter sets or architectures.

For that, go to the directory where the files should be produced, and
invoke cmake (or ccmake) with the full path to the SimGrid source as
last argument.

.. code-block:: shell

  mkdir build
  cd build
  cmake [options] ..
  make

Existing Compilation Targets
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In most cases, compiling and installing SimGrid is enough:

.. code-block:: shell

  make
  make install # try "sudo make install" if you don't have the permission to write

In addition, several compilation targets are provided in SimGrid. If
your system is well configured, the full list of targets is available
for completion when using the ``Tab`` key. Note that some of the
existing targets are not really for public consumption so don't worry
if some do not work for you.

- **make simgrid**: Build only the SimGrid library and not any example
- **make s4u-app-pingpong**: Build only this example (works for any example)
- **make java-all**: Build all Java examples and their dependencies
- **make clean**: Clean the results of a previous compilation
- **make install**: Install the project (doc/ bin/ lib/ include/)
- **make uninstall**: Uninstall the project (doc/ bin/ lib/ include/)
- **make dist**: Build a distribution archive (tar.gz)
- **make distcheck**: Check the dist (make + make dist + tests on the distribution)
- **make documentation**: Create SimGrid documentation

If you want to see what is really happening, try adding ``VERBOSE=1`` to
your compilation requests:

.. code-block:: shell

  make VERBOSE=1

.. _install_src_test:

Testing your build
^^^^^^^^^^^^^^^^^^

Once everything is built, you may want to test the result. SimGrid
comes with an extensive set of regression tests (as described in the
@ref inside_tests "insider manual"). The tests are run with ``ctest``,
that comes with CMake.  We run them every commit and the results are
on `our Jenkins <https://ci.inria.fr/simgrid/>`_.

.. code-block:: shell

  ctest	                    # Launch all tests
  ctest -R s4u              # Launch only the tests whose names match the string "s4u"
  ctest -j4                 # Launch all tests in parallel, at most 4 concurrent jobs
  ctest --verbose           # Display all details on what's going on
  ctest --output-on-failure # Only get verbose for the tests that fail

  ctest -R s4u -j4 --output-on-failure # You changed S4U and want to check that you didn't break anything, huh?
                                       # That's fine, I do so all the time myself.

.. _install_cmake_mac:

Mac OS X Specifics
^^^^^^^^^^^^^^^^^^

SimGrid compiles like a charm with clang (version 3.0 or higher) on Mac OS X:

.. code-block:: shell

  cmake -DCMAKE_C_COMPILER=/path/to/clang -DCMAKE_CXX_COMPILER=/path/to/clang++ .
  make


Troubleshooting your Mac OS X build.

CMake Error: Parse error in cache file build_dir/CMakeCache.txt. Offending entry: /SDKs/MacOSX10.8.sdk
  This was reported with the XCode version of clang 4.1. The work
  around is to edit the ``CMakeCache.txt`` file directly, to change
  the following entry:

  ``CMAKE_OSX_SYSROOT:PATH=/Applications/XCode.app/Contents/Developer/Platforms/MacOSX.platform/Developer``

  You can safely ignore the warning about "-pthread" not being used, if it appears.

/usr/include does not seem to exist
  This directory does not exist by default on modern Mac OS X versions,
  and you may need to create it with ``xcode-select -install``

.. _install_cmake_windows:

Windows Specifics
^^^^^^^^^^^^^^^^^

The best solution to get SimGrid working on windows is to install the
Ubuntu subsystem of Windows 10. All of SimGrid (but the model-checker)
works in this setting.

Native builds not very well supported. Have a look to our `appveypor
configuration file
<https://framagit.org/simgrid/simgrid/blob/master/.appveyor.yml>`_ to
see how we manage to use mingw-64 to build the DLL that the Java file
needs.

The drawback of MinGW-64 is that the produced DLL are not compatible
with MS Visual C. `clang-cl <http://clang.llvm.org/docs/MSVCCompatibility.html">`_
sounds promising to fix this. If you get something working or if you
have any other improvement, please @ref community_contact "tell us".

Java Specifics
^^^^^^^^^^^^^^

Once you have the `full JDK <http://www.oracle.com/technetwork/java/javase/downloads>`_ installed,
things should be as simple as:

.. code-block:: shell

   cmake -Denable_java=ON .
   make  simgrid-java_jar # Only build the jarfile

After the compilation, the file ```simgrid.jar``` is produced in the
root directory.

**Troubleshooting Java Builds**

Sometimes, the build system fails to find the JNI headers. First locate them as follows:

.. code-block:: shell

  $ locate jni.h
  /usr/lib/jvm/java-8-openjdk-amd64/include/jni.h
  /usr/lib/jvm/java-9-openjdk-amd64/include/jni.h
  /usr/lib/jvm/java-10-openjdk-amd64/include/jni.h


Then, set the JAVA_INCLUDE_PATH environment variable to the right
path, and relaunch cmake. If you have several versions of JNI installed
(as above), pick the one corresponding to the report of
``javac -version``

.. code-block:: shell

  export JAVA_INCLUDE_PATH=/usr/lib/jvm/java-8-openjdk-amd64/include/
  cmake -Denable_java=ON .
  make

Note that the filename ```jni.h``` was removed from the path.

Linux Multi-Arch Specifics
^^^^^^^^^^^^^^^^^^^^^^^^^^

On a multiarch x86_64 Linux, it should be possible to compile a 32-bit
version of SimGrid with something like:

.. code-block:: shell

  CFLAGS=-m32 \
  CXXFLAGS=-m32 \
  PKG_CONFIG_LIBDIR=/usr/lib/i386-linux-gnu/pkgconfig/ \
  cmake . \
  -DCMAKE_SYSTEM_PROCESSOR=i386 \
  -DCMAKE_Fortran_COMPILER=/some/path/to/i686-linux-gnu-gfortran \
  -DGFORTRAN_EXE=/some/path/to/i686-linux-gnu-gfortran \
  -DCMAKE_Fortran_FLAGS=-m32

If needed, implement ``i686-linux-gnu-gfortran`` as a script:

.. code-block:: shell

  #!/usr/bin/env sh
  exec gfortran -m32 "$@"


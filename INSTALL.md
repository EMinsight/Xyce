# Xyce Configure, Build and Installation Guide Using CMake

This guide describes the basic process for compiling and installing a Xyce
binary using the CMake build system. It is easiest to view these instructions
with full formatting on the [Xyce GitHub website](https://github.com/Xyce/Xyce/blob/master/INSTALL.md).
(For instructions on building Xyce with the Autotools system, see the
[Xyce Building Guide](https://xyce.sandia.gov/documentation-tutorials/building-guide/).)

If you do not want to build from source, binary installers for Windows, Mac and
Red Hat Linux are made available for every release of Xyce on the
[Xyce website](https://xyce.sandia.gov).

Xyce can be built in two variants: serial or with MPI parallelism. The variant
is determined by whether Trilinos is built in serial or with MPI. While the
parallel and serial variants can co-exist on a system, they must be installed
in different directories (for both Trilinos and Xyce).

If you have not yet done so, clone the [Xyce
repository](https://github.com/Xyce/Xyce) from GitHub, or download one of the
release tarballs either from GitHub or the [Xyce website](https://xyce.sandia.gov).

The main challenge in building Xyce involves properly obtaining and/or building
the third-party libraries (TPLs), particularly
[Trilinos](https://trilinos.github.io/). We are in the process of developing a
CMake "superbuild" capability that can automatically download and build many of
the TPLs. The [Using the Superbuild](#using-the-superbuild) section covers that
approach. The [Standard Build Approach](#standard-build-approach) section
covers the more traditional method. For either method, a certain minimal set of
dependencies are required, which is covered first.

Note that the install/uninstall commands may require the use of `sudo` on
Unix-like systems.

If many flags are being given to CMake on the command line (for Trilinos or
Xyce), you might want to create a configuration script. See the [Configuration
Scripts](#configuration-scripts) section for more detail.

Some systems require modifications to the following instructions. These are
covered in the [System-Specific Modifications](#system-specific-modifications)
section for the following systems:
- [Windows](#windows)
- [Cygwin](#cygwin)
- [Ubuntu](#ubuntu) (17.10, 18.x and 19.x)
- [MacOS](#macos)

## Obtaining the Dependencies

Most of the software needed to build Xyce is available via package management
systems. (See the Modifications section for [MacOS](#macos) and
[Windows](#windows) systems.)

You will need to obtain the following tools if they aren't already loaded on
your system:
- C/C++ compiler suite — C++11-compatible (e.g., gcc 4.9 or later, Clang 3.3 or
  later)\
  (These could be in separate packages on your system.)
- Fortran compiler (e.g., gfortran)
  + Some package managers bundle Fortran with the C and C++ compilers.
  + Trilinos has Fortran code, but leveraging it is technically
    [optional](https://docs.trilinos.org/files/TrilinosBuildReference.html#disabling-the-fortran-compiler-and-all-fortran-code).
    Not using the Fortran code, though, could result in slightly lower performance.
  + Xyce, itself, does not use Fortran.
- Build system (e.g., [Make](https://www.gnu.org/software/make/),
  [Ninja](https://ninja-build.org/), [Jom](https://wiki.qt.io/Jom))
- [CMake](https://cmake.org) (3.13 or later)
- [Bison](https://www.gnu.org/software/bison) (3.0.4 or later)
- [flex](https://github.com/westes/flex) (2.5.34 or later)

You will also need the following libraries:
- [BLAS](http://www.netlib.org/blas)
- [LAPACK](http://www.netlib.org/lapack)

On some systems, some of the tools and/or libraries also have "developer"
library packages associated with the primary packages. Those will also need to
be installed.

## Standard Build Approach

The standard build approach requires all the third-party software to be in
place prior to invoking CMake to configure Xyce. Be sure you have followed the
instructions in the [Obtaining the Dependencies](#obtaining-the-dependencies)
section, above. Then proceed through the following tasks, which are broken into
three steps:
- Obtaining the TPLs (except Trilinos)
- Building Trilinos
- Building Xyce

We encourage you to understand the steps below; but, _after obtaining the
TPLs_, the TL;DR steps for a serial build on a Unix-like system are:
```sh
cd <your-build-directory>
mkdir trilinos-build
cd trilinos-build

cmake \
-D CMAKE_INSTALL_PREFIX=/path/to/install \
-C path/to/Xyce/cmake/trilinos/trilinos-config.cmake \
path/to/Trilinos

cmake --build . -j 2 -t install
cd ..
mkdir xyce-build
cd xyce-build

cmake \
-D CMAKE_INSTALL_PREFIX=/path/to/install \
-D Trilinos_ROOT=/path/to/Trilinos_install \
path/to/Xyce

cmake --build . -j 2 -t install
```

### Obtaining the TPLs

The minimal third-party libraries needed for Xyce are:
- [SuiteSparse](http://faculty.cse.tamu.edu/davis/suitesparse.html) (2.1.1 or later)
- [FFTW](http://www.fftw.org) (3.x)

For parallel builds you will also need [Open MPI](http://www.open-mpi.org)
([MPICH](https://www.mpich.org/) should also work, but the Xyce team does not
regularly build with that library).

The above are available in most package managers. Again, on some systems, the
libraries might have "developer" packages that will need to be installed in
addition to the primary packages. Anything not available via a package manager
will need to be built from source.

#### Building SuiteSparse

Building SuiteSparse is not difficult. However, the only part of SuiteSparse
used by Xyce is AMD. As an alternative building process, we have provided a
"CMakeLists.txt" file in the `Xyce/cmake/trilinos/AMD/` directory. Using CMake,
the file will allow you to compile and install _only_ the AMD library. See the
comment block at the top of the [file](cmake/trilinos/AMD/CmakeLists.txt) for
instructions on its use.

### Building Trilinos

While [Trilinos](https://trilinos.github.io/) is available in some package
managers, the particular installation may not have all the features required by
Xyce. If you find Trilinos is available, you can try skipping to the Xyce
section to see if Xyce configures and builds. If not, continue with these
instructions.

The following process will produce a serial Trilinos installation that will
contain only the libraries needed by Xyce. For the parallel version, be sure to
understand the serial build process prior to reading the [Building Trilinos
with MPI Parallelism](#building-trilinos-with-mpi-parallelism) section, below.

First, download Trilinos version 12.12.1 from the [Trilinos GitHub
Page](https://github.com/trilinos/trilinos), or use this [direct
link](https://github.com/trilinos/Trilinos/archive/refs/tags/trilinos-release-12-12-1.tar.gz).
If you wish to use a git clone, you can obtain just the 12.12.1 files (and a
faster download) by running:
```sh
git clone --depth 1 --branch trilinos-release-12-12-1 https://github.com/trilinos/Trilinos.git
```
Again, be sure to compile version 12.12.1. Other versions will not work with
the following process, as they require slightly different build options.

In order to build Trilinos, first create a "build" directory in a convenient
location. (The name does not matter, and can be something simple like
`trilinos_build`.) On Unix-like systems, the default Trilinos installation
location is `/usr/local`. This can be changed by adding the following flag to
the CMake invocation.
```sh
-D CMAKE_INSTALL_PREFIX=/path/to/install
```
As with Xyce, both a parallel and serial build of Trilinos can exist on the
same system, but they must be in different directories. (We recommend
specifying unique sub-directories in `/usr/local`, such as
`/usr/local/trilinos_serial`.) If you install Trilinos in a temporary location,
you will need to rebuild it when building a new version of Xyce. (The
recommended version of Trilinos does not change often, so it can save time to
have an installed Trilinos library.)

If you have compilers or libraries in non-standard locations, see the [Other
Trilinos Options](#other-trilinos-options) section, below.

A CMake "initial cache" file, called `trilinos-config.cmake`, is included in
the Xyce repository in the `cmake/trilinos` directory. The file contains a
typical set of options for a Xyce-oriented serial Trilinos build.

To configure Trilinos (using the default `/usr/local` install location), enter
the build directory and run:
```sh
cmake -C path/to/Xyce/cmake/trilinos/trilinos-config.cmake path/to/Trilinos
```
Once the configuration step has completed, run the following in the build
directory to build and install Trilinos:
```sh
cmake --build . -j 2 -t install
```
The "-j 2" designates the number of processors to be used for compiling
Trilinos. Choose an appropriate number for your system.

After the Xyce installation is successful, and if you do not plan on building
Trilinos again, you may delete the Trilinos build directory (see the
[Uninstalling Xyce](#uninstalling-xyce) section first, however).

#### Other Trilinos Options

The following are various flags that might be needed when building Trilinos.
These can be added to the command line, or put in a [configuration
script](#configuration-scripts).

CMake will use the first compiler set it finds on your system. You can specify
the compilers by adding the following flags to the CMake invocation:
```sh
-D CMAKE_C_COMPILER=<C-compiler> \
-D CMAKE_CXX_COMPILER=<C++-compiler> \
-D CMAKE_Fortran_COMPILER=<Fortran-compiler> \
```
You may need to use a full path if they are not visible in your default paths.

Similarly, if the third-party libraries (AMD, BLAS and LAPACK) are not visible
in your default paths, use the following flags to help CMake find the
libraries:
```sh
-D AMD_LIBRARY_DIRS=/path/to/AMD/lib \
-D AMD_INCLUDE_DIRS=/path/to/AMD/include \
-D BLAS_LIBRARY_DIRS=/path/to/BLAS/lib \
-D LAPACK_LIBRARY_DIRS=/path/to/LAPACK/lib \
```
See the [Trilinos Build
Reference](https://docs.trilinos.org/files/TrilinosBuildReference.html) for
more details on the Trilinos build options.

#### Building Trilinos with MPI Parallelism

To enable MPI parallelism in Xyce, Trilinos must be built with MPI enabled. An
"initial cache" file, called `trilinos-config-MPI.cmake`, is provided for MPI
builds. In addition to using the MPI-oriented cache file, the MPI compilers
must be explicitly specified to CMake. The following CMake invocation should
work on most systems:
```sh
cmake \
-C path/to/Xyce/cmake/trilinos/trilinos-config-MPI.cmake \
-D CMAKE_C_COMPILER=mpicc \
-D CMAKE_CXX_COMPILER=mpicxx \
-D CMAKE_Fortran_COMPILER=mpifort \
path/to/Trilinos
```
Note that the exact compiler names may be different on your system. Also, the
above invocation will install Trilinos in `/usr/local`.

As with the serial build, once the configuration step is done, run the
following in the build directory to compile and install Trilinos:
```sh
cmake --build . -j 2 -t install
```

### Building Xyce

Once Trilinos is installed, you can build and install Xyce. By default, Xyce
will be installed in the `/usr/local/` directory. To specify a different
installation location, add the following flag to the CMake invocation:
```sh
-D CMAKE_INSTALL_PREFIX=/path/to/install
```
Again, if you plan to have both a parallel and serial build of Xyce on your
system, they must be in different directories. (We recommend specifying unique
sub-directories in `/usr/local`, such as `/usr/local/xyce_serial`.)

If Trilinos is not located in your path, add the following flag to the CMake
invocation:
```sh
-D Trilinos_ROOT=/path/to/Trilinos_install
```
Create a build directory for Xyce (such as `xyce_build`) and go into that
directory. Then run CMake using:
```sh
cmake [flags] path/to/Xyce
```
Then, to build and install Xyce, run:
```sh
cmake --build . -j 2 -t install
```
The "-j 2" designates the number of processors to be used for compiling Xyce.
Choose an appropriate number for your system.

#### Adding the Xyce/ADMS Verilog-A Model Compiler

Xyce has a Verilog-A model compiler capability, using the "Xyce/ADMS" compiler
tool. See the [Xyce/ADMS Users
Guide](https://xyce.sandia.gov/documentation-tutorials/xyce-adms-users-guide/)
for more information on the capability and for instructions on using Xyce/ADMS.

To enable the feature, install [ADMS](https://github.com/Qucs/ADMS) prior to
building Xyce. Then, to enable the capability in the Xyce build, add the
following flag to the Xyce CMake invocation:
```sh
-D Xyce_PLUGIN_SUPPORT=ON
```
The CMake support for the plugin capability is still being developed. As such,
there are some differences from the website:
- The "toys" example is installed in `/path/to/install/share/examples/toys`.
- The `buildxyceplugin.sh` script requires the absolute path to the `.va`
  files.
- The name of the plugin file can vary by system (e.g., on a Mac, the "toys"
  library will be called, "libtoys.dylib", not "toys.so").

## Using the Superbuild

While easy, this approach has not been thoroughly tested, so should be
considered a "beta" capability. Also, it currently installs everything into an
"install" directory in the build directory, which may not be ideal for most
users. Therefore, we encourage people to use the [standard build
approach](#standard-build-approach), which involves only a few more steps.

Assuming the dependencies have been installed, CMake will automatically build
the following components:
- ADMS
- FFTW
- SuiteSparse
- Trilinos

Then CMake will compile a serial build of Xyce that enables the (optional)
[Xyce/ADMS](https://xyce.sandia.gov/documentation-tutorials/xyce-adms-users-guide/)
model plugin capability. Note that, since CMake is building several packages,
the process could take a long time.

To perform a superbuild, follow this procedure:
- Create a "build" directory somewhere on your system (the location and
  directory name are not important)
- From the build directory, run the following command:
  ```sh
  cmake -D Xyce_USE_SUPERBUILD=ON path/to/Xyce
  ```
- Then run
  ```sh
  cmake --build . -j 2
  ```
  The "-j 2" designates the number of processors to be used for compiling.
  Choose an appropriate number for your system.

## Uninstalling Xyce

The Xyce CMake does not create an uninstall script. However, on installation it
does produce an "install manifest" file, which lists every installed file with
their full path. To remove a Xyce installation from a Unix-like system, simply
run:
```sh
xargs rm < install_manifest.txt
```
Note that the above method is immediate and permanent. You may also want to
uninstall Trilinos. As with Xyce, an `install_manifest.txt` file is produced
when Trilinos is installed.

If you do not want to keep the build directories, simply copy the
`install_manifest.txt` file(s) to a safe location (the file name can be
changed). Then Xyce and/or Trilinos can be uninstalled at any time using the
`xarg` command, above (be sure to specify the correct filename).

## Running the Test Suite

If you wish to test the Xyce installation, run the [Xyce Regression
Suite](https://github.com/Xyce/Xyce_Regression). See the [Running the Xyce Regression
Suite](https://xyce.sandia.gov/documentation-tutorials/running-the-xyce-regression-suite/)
documentation on the Xyce home page. Note that the test suite is controlled
with Perl and Bash scripts, and some tests require Python with Scipy and Numpy.

## Configuration Scripts

If many flags are being applied as part of a CMake invocation, you might want
to create a configuration script. This is essentially a shell script with the
CMake invocation. The following commands are for Unix-like systems. Analogous
scripts can be created for Windows systems.

As an example, to configure an MPI-enabled Trilinos build, one might create a
file containing:
```sh
#!/bin/sh

cmake \
-C path/to/Xyce/cmake/trilinos/trilinos-config.cmake \
-D CMAKE_C_COMPILER=mpicc \
-D CMAKE_CXX_COMPILER=mpicxx \
-D CMAKE_Fortran_COMPILER=mpifort \
-D CMAKE_INSTALL_PREFIX=$HOME/install/Trilinos/mpi/ \
path/to/Trilinos
```
More options can be added as needed. The file can be made executable by running
the following on the command line:
```sh
chmod a+x <filename>
```
Running the configuration script in the build directory is the same as running
the CMake configure line directly.

## System-Specific Modifications

The instructions above are applicable to most operating systems. Modifications
for specific systems are added below as we become aware of them.

### Windows

Compiling Xyce on Windows is not a small task at the moment, primarily because
Windows does not have a package manager equivalent. Internally, we use the
Intel compiler suite with the Intel [Math Kernel
Library](https://software.intel.com/content/www/us/en/develop/tools/oneapi/components/onemkl.html)
(MKL). The MKL provides the BLAS, LAPACK and FFT capabilities (removing the need
for FFTW). At the beginning of 2021, Intel rebranded their tool chains as the
[oneAPI](https://software.intel.com/content/www/us/en/develop/tools/oneapi.html)
Toolkits, and now make them available for free.

We have not yet tried using oneAPI, so we cannot give advice on how to use it.
One should be able to use the [oneAPI Base
Toolkit](https://software.intel.com/content/www/us/en/develop/tools/oneapi/base-toolkit.html),
which supplies C, C++ and Fortran compilers, along with the MKL. Installing the
standalone [Microsoft C++ Build
Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/) first (as
required by oneAPI) should provide the needed system libraries. The Microsoft
Build Tools will also supply CMake and the NMAKE build tool. If you decide to
try compiling with oneAPI, we welcome feedback on your experience.

On Windows, Bison and flex are available via the
[WinFlexBison](https://github.com/lexxmark/winflexbison) package, leaving
SuiteSparse as the remaining (non-Trilinos) requirement. For SuiteSparse, see
the [Building SuiteSparse](#building-suitesparse) section above, under
[Obtaining the TPLs](#obtaining-the-tpls). Once all the TPLs are in place,
continue with the standard Trilinos and Xyce build processes.

The Xyce regression suite must be run in a Unix-like environment with Perl.
Therefore, to use it in Windows, you will need to install Cygwin. It might also
be possible to use the Windows Subsystem for Linux (WSL), but we have no
experience with it.

### Cygwin

Cygwin installs a minimal set of packages by default, so all of the
dependencies will have to be explicitly added, including the AMD library. As of
this writing the required packages are:
- git (optional)
- gcc-core
- gcc-g++
- gcc-fortran
- make (or ninja)
- cmake
- bison
- flex
- liblapack0, liblapack-devel
- libsuitesparseconfig-devel, libamd-devel
- fftw, libfftw3-devel

Cygwin also needs additional flags to be applied when compiling Trilinos and
Xyce. For Trilinos, add the following to the CMake invocation:
```sh
-D CMAKE_CXX_FLAGS="-D_BSD_SOURCE -D_GNU_SOURCE" \
-D CMAKE_C_FLAGS="-D_BSD_SOURCE -D_GNU_SOURCE" \
-D AMD_INCLUDE_DIRS="/usr/include/suitesparse" \
```
For Xyce, add the following to the CMake invocation:
```sh
-D CMAKE_CXX_FLAGS="-D_BSD_SOURCE -D_GNU_SOURCE" \
-D CMAKE_C_FLAGS="-D_BSD_SOURCE -D_GNU_SOURCE" \
```

### Ubuntu

Ubuntu releases starting with 17.10 and through the 19.x series (including
18.04), have a broken version of Open MPI in their package repositories. Open
MPI is functional in Ubuntu 20.04. (In the problem releases, the version of
Open MPI in the repositories is compiled with the `--enable-heterogeneous`
option, which breaks MPI's standard compliance and causes Xyce to fail in some
situations.)

If you are running a version of Ubuntu that has this issue, the only workaround
is to uninstall the OpenMPI package and build OpenMPI from source, without the
`--enable-heterogeneous` option. Comment 11 of the [Launchpad bug
report](https://bugs.launchpad.net/ubuntu/+source/openmpi/+bug/1731938/comments/11)
contains instructions for how to rebuild and install Open MPI using the Debian
package building system.

If you wish to check whether your system's install of OpenMPI has this issue,
you can run a small test program from the Xyce source tree. To test your
OpenMPI package, copy the file,
[Xyce/src/test/MPITest/testBUG967.c](src/test/MPITest/testBUG967.c), into
temporary directory. Then compile and run it using the following commands:
```sh
mpicc -o testBUG967 testBUG967.c
mpirun -np 2 ./testBUG967
```
If the run produces any output with the word __BAD__, your OpenMPI install is
broken and cannot be used. You must instead use the workaround described above.

### MacOS

MacOS does not have a native package manager, but there are several third-party
package managers available. We have had success with
[MacPorts](https://www.macports.org/) and [Homebrew](https://brew.sh/).

BLAS and LAPACK are provided by Xcode and should give better performance than
the BLAS and LAPACK libraries provided by the package managers. Therefore, you
do not need to install those libraries, explicitly.

We have seen a rare failure when using the Apple Clang compilers _and not_
using a Fortran compiler. If the Trilinos build fails while compiling AztecOO,
then you will either need to use different C/C++ compilers, or install gfortran
with a package manager (it is usually supplied with the gcc package).

__Xyce/ADMS on MacOS__\
The `buildxyceplugin.sh` script relies on the behavior of Gnu "readlink", which
is very different than BSD readlink (MacOS is based on BSD Unix). You can
obtain Gnu readlink by installing the "coreutils" package from MacPorts or
Homebrew.

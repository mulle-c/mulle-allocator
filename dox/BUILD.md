# How to build mulle-allocator


## What you get

* `libmulle_allocator.a` and `libmulle_test_allocator.a`, the mulle-allocator
static libraries along with a bunch of headers.



## Prerequisites

#### mulle-build

[mulle-build](//www.mulle-kybernetik.com/software/git/mulle-build) is required
to assemble the dependencies together and build the library.

#### mulle-c11

[mulle-c11](//www.mulle-kybernetik.com/software/git/mulle-c11/) is a header
that abstracts a small set of non-standardized compiler features.

#### mulle-thread

[mulle-thread](//www.mulle-kybernetik.com/software/git/mulle-thread/) unifies
atomic and thread operations across multiple platforms.

#### mulle-configuration

[mulle-configuration](//www.mulle-kybernetik.com/software/git/mulle-c11/) are
configuration files for building with Xcode or cmake.


### Windows: Installing further prerequisites

You need to install some more prerequisites first.

* Install [Visual Studio 2015 Community Edition](//beta.visualstudio.com/downloads/)
or better (free). Make sure that you install Windows C++ support. Also add git support.
* [Git for Windows](//git-scm.com/download/win) is included in VS 2015, make sure it's there
* [Python 2 for Windows](//www.python.org/downloads/windows/). Make sure that python is installed in **PATH**, which is not the default.
* [CMake for Windows](//cmake.org/download/). CMake should also add itself to **PATH**.

Reboot, so that Windows picks up the **PATH** changes (Voodoo).

Now the tricky part is to get the "Git bash" shell running with the proper VS
environment.  Assuming you kept default settings the "Git bash" is
`C:\Program Files\Git\git-bash.exe`. Open the "Developer Command Prompt for VS 2015"
from the start menu and execute the git-bash from there. A second window with
the bash should open.

Check that you have the proper environment for VS compilation with `env`.


### OSX: Install mulle-build using homebrew

Install the [homebrew](//brew.sh/) package manager, then

```
brew tap mulle-kybernetik/software
brew install mulle-build
```

### Linux: Install mulle-build using linuxbrew

Install the [linuxbrew](//linuxbrew.sh/) package manager, then

```
brew tap mulle-kybernetik/software
brew install mulle-build
```

### All: Install mulle-build using git

```
git clone https://www.mulle-kybernetik.com/repositories/mulle-bootstrap
( cd mulle-bootstrap ; ./install.sh )
git clone https://www.mulle-kybernetik.com/repositories/mulle-build
( cd mulle-build ; ./install.sh )
```

## All: Install mulle-allocator using mulle-build


Grab the latest **mulle-allocator** release and go into the project directory:

```
git clone https://www.mulle-kybernetik.com/repositories/mulle-allocator
cd mulle-allocator
```

Then let **mulle-build** fetch the dependencies and build **mulle-allocator** in
debug mode:

```
mulle-build --debug
```

Build library in release mode and install into `tmp` :

```
mulle-clean ;
mulle-install --prefix /tmp
```


### Compile Flags

For development use no flags.

For production use NDEBUG

* DEBUG : turns on some compile time facilities to aid debugging `mulle-allocator` itself.
* MULLE_ATOMIC_TRACE : turns on a very detailed amount of tracing for some atomic operations.




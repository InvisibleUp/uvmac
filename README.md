# µvMac 0.37.0

Forked from Mini vMac v36.4 by Paul C. Pratt (http://www.gryphel.com/c/minivmac), which was forked from vMac by Philip "PC" Cummins (http://www.vmac.org/main.html)

µvMac (micro vMac) is a small, simple emulator for early Motorola 68000 based emulators. Currently we support systems from the original Macintosh 128K all the way up to the Macintosh II.

This fork was created to clean up and modernize the code base, make the project easier to compile and hack on, and allow for much easier user configuration. The intent of Mini vMac was to create a "emulator collection" of many very optimized "variations" of the same codebase. I consider this much more trouble than it's worth, and intend to focus more on maintainability and accuracy.

µvMac is undergoing substansial and sporadic development and is unlikely to be in an usable state at this time.

## Supported Platforms

µvMac *absolutely requires* SDL2. There are no plans to support platforms that SDL2 does not target. For 99% of users, this should not be a concern. Dropping support for esoteric platforms and exclusively using SDL2 vastly simplifies the complexity of the code.

So far µvMac has only been tested on Windows and Linux. No other operating systems are supported at this time.

# Building

µvMac uses CMake 3.21+ and [vcpkg](https://vcpkg.io/).

1. `git submodule update --init --recursive`
2. Install CMake and a suitable C compiler. Windows users can use Visual Studio. Ubuntu users may need to use the [Kitware APT repo](https://apt.kitware.com/).
3. Run `cmake -B [build directory] -S . --preset=debug` (or open the project your IDE). Note that on Linux the configuration may fail due to missing system packages. Install those as needed.
4. Enter your build directory and run `ninja` to build
5. Acquire a Macintosh Plus ROM and a system disk and place it in the build directory, named `vMac.ROM`
6. Start the application and drag the system disk to the window.

## Legal info

You can redistribute µvMac and/or modify it under the terms
of version 2 of the GNU General Public License as published by
the Free Software Foundation.  See the included file COPYING.txt

µvMac is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
license for more details.

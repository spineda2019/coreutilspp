# coreutilspp
CoreUtils++.

## What is it?
Ultimately, just a hobby project. I want to re-implement the GNU core utils,
but in C++ and make them portable out of the box.

## How is it?
This should use the zig build system and some small zig glue
where needed if I cannot avoid using libc. The goal is to use C++ only without
libc.

## Why is it?
I really just wanted to make a cross platform C++ project, and this is easy to
dog food.

## Licensing
All source code in this project, be it code that is built into binaries (e.g.
the coreutils themselves) or otherwise (e.g. the build script) is licensed under
the GPLv3 _or later_. See the LICENSE file and the headers in source code for
information.

## Documentation
Generated with Doxygen, and host on GitHub Pages

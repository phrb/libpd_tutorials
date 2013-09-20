libpd_tutorials:

This repository contains a copy of libpd source and makefile.

The examples directory contains a series of tutorials on implementing
various libpd functionalities, accompanied by their makefiles and
matching pd patches.

Solution for libpd not binding function pointers correctly:

    Add libpd/util/z_hook_util.c to Makefile.

For some reason, it's not added as default.

This provides the set functions for the function hooks to be called
by Pure Data patches.

For documentation of libpd, see the wiki: https://github.com/libpd/libpd/wiki

  * libpd_wrapper: This folder contains the source files that make up the core
      of libpd.
    
  $ make libpd: 
    - libpd, (default) builds if no target is specified, builds the libpd.so/dylib/dll
    - Makefile modified to provide set hook functions.

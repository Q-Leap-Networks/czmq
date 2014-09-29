# Experimental Area

This directory is for classes that are works in progress, or
experiments. The rules here are a little different. Experiments won't
be packaged and shipped in tarballs. The API header for each class
sits in this directory together with the source file. There is no
visibility of experiments in the CZMQ project structure: neither in
documentation, nor czmq.h.

If you configure czmq using --with-labs then experimental classes will
be compiled as well and their test-suite will be run alongside the
normal test-suite. This can be disabled by configuring using
--without-labs. Use of --with-labs is recommended if you change czmq
code to ensure you don't break labs code. This does not mean any
classes from the labs area are included in libczmq, it only compiles
and checks them to make sure they still work.

To use an experimental class in your code, you can either copy it
wholesale, or you can include it directly (include the header file and
include the source into a class in your project).

We will later find a better way to package experimental classes. Our
goal at present is to have a space in the git repository without any
effect on the formal CZMQ API.

All other rules, regarding process, patch quality, and code style
apply. Ugly or poorly-argued code risks being deleted. Most
importantly any class added to labs/Makefile.am MUST compile and test
cases added to labs/czmq_labs_selftest must complete. It is OK not to
add them if you don't believe the code is ready for it.

There is one extra rule for experimental classes. Since the header for
experimental classes are not included in czmq.h they MUST be included
manually in each class. This should happen in the following order:
First include czmq.h, secondly include the classes own header, third
include headers of experimental classes used by the class if any.

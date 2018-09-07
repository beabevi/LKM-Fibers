Fibers Testbed
==============

This package implements a simple testbed application for fibers usage,
which can be used to stress test, debug, and profile the assignment for
the Advanced Operating Systems and Virtualization 2017/2018 course.


Description
-----------

The application implements a simplistic data-separation model in which
multiple entities run a simulation of GSM-based mobile calls. Each fiber
implements a GSM cell, which are concurrently run by multiple threads.
The software discovers at startup the number of available cores and
spawns an equal number of threads. The number of fibers, i.e. the number
of GSM cells to be simulated, is passed as a command-line argument to
the compiled program.

The execution halts after 15000 calls are correctly simulated at each
GSM cell (this is the `COMPLETE_CALLS` macro in `model.h`).
There are plenty of floating-point calculations in the code, to test
whether your implementation correctly handles the CPU state.

You should vary the number of concurrent fibers to perform a performance
assessment to be handed out with the implementation of the assignment.
Find out a maximum number of fibers to be used, which allows to show
a significant trend in the execution results, on your machine.
Please note that you should test it with runs which last a bit to be
sure enough that everything is fine on your implementation side.

At the end of the execution, you will receive a message like:

    All fibers are done!
    Time to initialize fibers: 0.025000
    Time to run do the work (per-fiber): 0.391461

You can use these numbers for the assignmet, e.g. to prepare some
performance plot.


Usage and Customization
-----------------------

The package is configured to use the userspace version of User-Level
Threads. You can compile it using `make`. This generates the `test`
program, to be launched as `./test <num_fibers>`.

The entry point for your code is `fiber.h`. There, you will find the
global macro `USERSPACE` (which is defined in the downloaded package)
which activates the userspace implementation. You already have a code
skeleton into which you can plug your fiber implementation, when
`USERSPACE` is not defined.

If you need some (static or shared) library for your code to be used,
you can modify the `Makefile`, by adding to the `LIBS` variable any
relevant library for your project to work.


Problems?
---------

Should you have problems using this software, of if problems arise,
drop a line to the instructor or come to office hours!

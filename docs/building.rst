Building RocketPLC
==================

You can build rocketplc as a linux application or build it for the variety of
hardware supported by it.

Testing On Linux
----------------

To build for linux do this:

.. code-block:: bash 
    
    autogen.sh
    mkdir build
    ../configure --with-devicetree=../dts/linux.dts
    make

    # run: ./src/rocket

When you run the executable you will see several serial ports (pseudo
terminals) created by the linux UART emulation included into RocketPLC which
you can connect to using picocom just as though they were real serial ports. 

Depending on how many terminals you have already created on your system, the
numbering of the terminals will be different.

.. code-block:: bash

    picocom /dev/pts/34

    rocket #

Building For FlyingBergman PLC Board
------------------------------------

FlyingBergman board is a `PLC Board`_ developed by SwedishEmbedded for the
FlyingBergman camera crane system. You can purchase the board as a standalone
PLC and use it with RocketPLC to control up to four motors and read user input
from buttons.

.. _`PLC Board`: http://swedishembedded.com/flyingbergman

To build for the FlyingBergman PLC board:

.. code-block:: bash

    autogen.sh
    mkdir build
    ../configure --with-devicetree=../dts/flyingbergman.dts --host=arm-none-eabi
    make

    # to flash use: make flash



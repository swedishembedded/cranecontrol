Realtime Python Kernel With Device Tree
---------------------------------------

![](./docs/realtime-python-banner.png)

This is a realtime kernel with device tree support for microcontrollers and
bare metal electronics devices that is designed to run python applications on
microcontrollers. 

Through device tree it is very easy to extend and add new drivers. Through
python it is very easy to extend and add new applications. 

As opposed to other RTOS projects, instead of supporting infinite number of
boards and trying to be overly generic - we aim to have 100% support for a
finite number of versatile building blocks with which you can build your
projects.

This way we can be laser focused on a specific set of products - which makes
the system much more robust and easier and simpler to maintain. This also means
we can have a very well documented system as well!

You can build your own boards as well, according to the specification, and make
them available for sale through this project!

Building
--------

	./autogen.sh
	mkdir build && cd build
	../configure --host=<toolchain> --with-devicetree=../dts/<board>.dts
	make
	make flash

Boards That Support Realtime Python
-----------------------------------

- Swedish Embedded Motor Control Development Board
- STM32F407 Discovery Board
- STM32F429 Discovery Board


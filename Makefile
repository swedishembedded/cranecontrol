all: compile
	echo "Done"

test/compile:
	theboss/bossc -o flyingbergman-test flyingbergman-test.dts

test/reconfigure:
	theboss/bossc -c -o flyingbergman-test flyingbergman-test.dts

submodules:
	git submodule update --init --recursive .

reconfigure:
	theboss/bossc -c -o flyingbergman.bin flyingbergman.dts

compile:
	theboss/bossc -o flyingbergman.bin flyingbergman.dts

flash: compile
	theboss/bossc -w -o flyingbergman.bin flyingbergman.dts

.PHONY: submodumes compile flash

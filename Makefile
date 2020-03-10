all: compile
	echo "Done"

test/compile:
	rocket/bossc -o flyingbergman-test flyingbergman-test.dts

test/reconfigure:
	rocket/bossc -c -o flyingbergman-test flyingbergman-test.dts

submodules:
	git submodule update --init --recursive .

reconfigure:
	rocket/bossc -c -o flyingbergman.bin flyingbergman.dts

compile:
	rocket/bossc -o flyingbergman.bin flyingbergman.dts

flash: compile
	rocket/bossc -w -o flyingbergman.bin flyingbergman.dts

.PHONY: submodumes compile flash

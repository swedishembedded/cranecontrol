all:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/compile

flyingbergman.csv:
	arm-none-eabi-nm -S theboss/build_dir/flyingbergman/firmware/src/theboss | awk 'NF==4{print "=\"" $$2 "\"," $$3 "," $$4} NF==3{print "=\"00000000\"," $$2 "," $$3}' | sort -k3 -t,> flyingbergman.csv

flyingbergman:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/compile
	make flyingbergman.csv

flyingbergman-flash:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/flash-stlink
	make flyingbergman.csv

tags:
	rm -f tags
	ctags --exclude=*staging_dir* -R .

.PHONY: tags flyingbergman.csv

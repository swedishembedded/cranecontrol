all:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/compile

flyingbergman.csv:
	sh update_symbols.sh theboss/build_dir/flyingbergman/firmware/src/theboss

flyingbergman:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/compile
	make flyingbergman.csv

flyingbergman-flash:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/flash-stlink

tags:
	rm -f tags
	ctags --exclude=*staging_dir* -R .

.PHONY: tags flyingbergman.csv

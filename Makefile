all:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/compile

flyingbergman:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/compile

flyingbergman-flash:
	make -C theboss PROJECT=$(PWD) package/firmware/flyingbergman/flash-stlink

tags:
	rm -f tags
	ctags --exclude=*staging_dir* -R .

.PHONY: tags

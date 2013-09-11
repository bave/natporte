
all: natporte
.PHONY : all

natporte:
	make -C src
.PHONY : natporte

debug:
	make -C src debug
.PHONY : debug

clean:
	make -C src clean
.PHONY : clean


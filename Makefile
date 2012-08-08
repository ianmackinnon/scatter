SHELL := /bin/bash

.PHONY: all install uninstall clean

all: scatter-mean scatter-sigma

scatter-mean: scatter-mean.c logging.h logging.c io.h io.c opt.h opt.c image.h image.c
	gcc -Wall -o scatter-mean scatter-mean.c logging.c io.c opt.c image.c -ltiff -lm -ljpeg

scatter-sigma: scatter-sigma.c logging.h logging.c io.h io.c opt.h opt.c image.h image.c
	gcc -Wall -o scatter-sigma scatter-sigma.c logging.c io.c opt.c image.c -ltiff -lm -ljpeg

install: scatter-mean scatter-sigma
	cp scatter-mean scatter-sigma /usr/local/bin

uninstall:
	rm -f /usr/local/bin/scatter-mean /usr/local/bin/scatter-sigma

clean:
	echo Cleaning up... 
	rm -rf scatter-mean scatter-sigma


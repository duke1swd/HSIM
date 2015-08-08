#! /bin/sh
HSD=~/HSIM/hybrid
(
	cd $HSD
	./hsim -N system
) < $1 > $1.output

(
	cd $HSD
	./report
) < $1.output

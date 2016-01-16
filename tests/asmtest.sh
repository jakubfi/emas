#!/bin/bash

BASEDIR=../em400/tests
TESTDIRS="alu args cycle int mem mod multix ops registers vendor"

for tests in $TESTDIRS ; do
	for file in `ls -1 $BASEDIR/$tests/*.asm` ; do
		echo $file
		build/src/emas -Oraw -o /tmp/emas.bin -I $BASEDIR/include $file
		../emdas/build/src/emdas -c mx16 -na -o /tmp/emas.asm /tmp/emas.bin
		build/src/emas -Oraw -c mx16 -o /tmp/emas2.bin /tmp/emas.asm
		cmp /tmp/emas.bin /tmp/emas2.bin
	done
done

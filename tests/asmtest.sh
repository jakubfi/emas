#!/bin/bash

EMAS=build/emas
EMDAS=../emdas/build/emdas
BASEDIR=../em400/tests
TESTDIRS="alu args cycle int mem mod multix ops registers vendor"

for tests in $TESTDIRS ; do
	for file in `ls -1 $BASEDIR/functional/$tests/*.asm` ; do
		echo $file
		$EMAS -Oraw -o /tmp/emas.bin -I $BASEDIR/include $file
		$EMDAS -c mx16 -na -o /tmp/emas.asm /tmp/emas.bin
		$EMAS -Oraw -c mx16 -o /tmp/emas2.bin /tmp/emas.asm
		cmp /tmp/emas.bin /tmp/emas2.bin || exit 1
	done
done

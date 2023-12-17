#!/bin/bash

EMAS=../build/emas
DIFF="diff --color "
DIRS=$(find acceptance -mindepth 1 -maxdepth 1 -type d)

for d in $DIRS ; do
	echo "--- $d ------------------------------------------------"
	files=$(ls -1 $d/*.asm 2>/dev/null)
	if [ -z "$files" ] ; then
		echo "Missing tests"
		exit 1
	fi
	for f in $files ; do
		echo $f
		expected=$(echo $f | sed s/\.asm$/\.out/)
		$EMAS -O debug $f &> /tmp/acceptance.out
		$DIFF $expected /tmp/acceptance.out
		if [ $? != 0 ] ; then
			echo "Ooops."
			exit 1
		fi
	done
done

echo "PASSED"

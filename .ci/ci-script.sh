#!/bin/bash

ACTION=$1

case $ACTION in

	init)
		cmake -E make_directory "$BUILD_DIR"
		echo === Building EMAWP ============================================
		git clone https://github.com/jakubfi/emawp
		cmake -E make_directory emawp/build
		cmake emawp -G "Unix Makefiles" -B emawp/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/mingw64
		cmake --build emawp/build
		cmake --install emawp/build
		emawp -h
		echo === Building EMDAS ===========================================
		git clone https://github.com/jakubfi/emdas
		cmake -E make_directory emdas/build
		cmake emdas -G "Unix Makefiles" -B emdas/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/mingw64
		cmake --build emdas/build
		cmake --install emdas/build
		emdas -v
	;;

	configure)
		cmake "$SRC_DIR" -G "Unix Makefiles" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
	;;

	build)
		cmake --build "$BUILD_DIR"
	;;

	install)
		cmake --install "$BUILD_DIR"
		emas -v
	;;

	test)
		cd "$SRC_DIR"/tests
		git clone https://github.com/jakubfi/em400
		./asmtest.sh em400/tests
	;;
	acceptance)
		cd "$SRC_DIR"/tests
		./run_test.sh
	;;

	*)
		echo "Unknown action: $ACTION"
		exit 1
	;;

esac

# vim: tabstop=4

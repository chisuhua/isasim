#!/bin/bash
# -*- coding: utf-8 -*-

set -e

build_dbg() {
	rm -rf build && mkdir build && cd build && meson .. && ninja || exit 1
}

build_release() {
	rm -rf build && mkdir build && cd build && meson .. --buildtype release && ninja || exit 1
}

if [ ! -z "$1" ]; then
	if [ $1 == lint ]; then
		echo start link check ...
	elif [ $1 == dbg ]; then
		build_dbg;
	elif [ $1 == rel ]; then
		build_release;
	fi
else
	build_dbg;
fi


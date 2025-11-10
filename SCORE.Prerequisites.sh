#!/bin/sh
Tools="gcc g++ cmake mpiexec wget git"
for Tool in $Tools; do
	if command -v "$Tool" >/dev/null 2>&1; then
		$Tool --version
	else
		echo "$Tool is not installed"
		exit 1
	fi
done
if command -v "openssl" >/dev/null 2>&1; then
	openssl version -a
else
	echo "openssl is not installed"
	exit 1
fi

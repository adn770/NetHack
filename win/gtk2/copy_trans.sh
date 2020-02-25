#!/bin/bash

cd po
for i in *
do
	if [ -d $i ]
	then
		cp -r $i ../../../dat/locale
	fi
done

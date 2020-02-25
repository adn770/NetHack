#! /bin/bash

LINGUAS=( de_DE it_IT pt_BR)

cd po
for i in "${LINGUAS[@]}"
do
	if [ ! -e $i.po ]
	then
		msginit --no-translator -l $i -o $i.po -i gtk2hack.pot
	else
		msgmerge -U $i.po gtk2hack.pot
	fi
	if [ ! -e $i ]
	then
		mkdir $i
		mkdir $i/LC_MESSAGES
	fi
	msgfmt $i.po --statistics -o $i/LC_MESSAGES/gtk2hack.mo
done

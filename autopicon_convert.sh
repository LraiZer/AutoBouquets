#!/bin/sh
# AutoPicon Convert E2 28.2E by LraiZer for www.ukcvs.net
# rename pre made 28.2E picon set to the autopicon format.

AUTOBOUQUETS_PATH=/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets

if [ "$1" = "" ] || [ -e /tmp/picon ]; then
	PICON_PATH="/tmp/picon"
else
	PICON_PATH="$1"
fi

if [ -e $PICON_PATH ] && [ -e $AUTOBOUQUETS_PATH/autobouquets.csv ]; then
	if ls -F $PICON_PATH/1_0_*_2_11A*_0_0_0.png 2>/dev/null | grep -v '@' >/dev/null; then
		echo "Converting Picons, working.."
		OIFS=$IFS
		IFS=','
		while read POSITION EPG_ID TYPE SID TSID ENCRYPTION NAME; do
		 NSPACE="11A0000"; [ "$TSID" = "0x7e3" ] && NSPACE="11A2F26"
		 SERVICE=$(echo $TYPE|cut -d 'x' -f2)_$(echo $SID|cut -d 'x' -f2)_$(echo $TSID|cut -d 'x' -f2)
		 SERVICE=$(echo $SERVICE|sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/')
		 PICON="1_0_"$SERVICE"_2_"$NSPACE"_0_0_0.png"
		 [ ! -L $PICON_PATH/$PICON ] && mv $PICON_PATH/$PICON $PICON_PATH"/282E_"$EPG_ID".png" >/dev/null 2>&1
		done < $AUTOBOUQUETS_PATH/autobouquets.csv
		IFS=$OIFS
	fi
fi


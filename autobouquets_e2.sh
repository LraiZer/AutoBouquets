#!/bin/sh
# AutoBouquets E2 28.2E by LraiZer for www.ukcvs.net

###################
#  DEFAULT SETUP  #
###################
DVB_FRONTEND="-1" #
DVB_ADAPTER="0"   #
DVB_DEMUX="0"     #
NEW_LAMEDB="4"    #
###################

versiondate="17 December 2018"
echo "Script Version: $versiondate"
start_time=`date +%s`
date; echo

clean_tmp(){
	cd /tmp
	rm -f *.tv *.radio sdt*.txt bat*.txt autobouquets.*
	cd /
}
clean_tmp

fpath=/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
sab=/etc/enigma2
ldb=$sab/lamedb

cr=$(printf "\r")
for f in $fpath/*.txt; do
	grep -q "${cr}$" $f
	[ "$?" = "0" ] && dos2unix $f
done

get_options(){
	opt=$(echo "$1"|sed 'y/TRUFALSE/trufalse/'|cut -d "#" -f$2)
	case "$opt" in
	"false") opt="0";;
	"true")  opt="1";;
	esac
	echo "$opt"
}

set_options(){
#
#	DATA_SD#REGION#DATA_HD#BOUQUET_LIST#EXTRA#SORT#NUMBERED#NIT_SCAN#PLACEHOLDER#PARENTAL#DEFAULT#ORDERING#FREE_TO_AIR#CHECK_SCRIPT#STYLE#PICONLINK#PICONFOLDER#PICONSTYLE
# valid script paramater EXAMPLES:
#	./autobouquets_e2.sh 4097#01#4101#TRUE#FALSE#1#False#True#true#false#True#True#2#True#0#1#3#0
#	./autobouquets_e2.sh 4097#01#4101#1#1#1#0#1#1#1#1#1#2#1#0#1#3#0
#
# default PYTHON  args: 1-18:
	DATA_SD="4097"
	REGION="01"
	DATA_HD="4101"
	BOUQUET_LIST="0"
	EXTRA="1"
	SORT="1"
	NUMBERED="0"
	NIT_SCAN="1"
	PLACEHOLDER="1"
	PARENTAL="1"
	DEFAULT="1"
	ORDERING="1"
	FREE_TO_AIR="2"
	CHECK_SCRIPT="1"
	STYLE="0"
	PICONLINK="1"
	PICONFOLDER="3"
	PICONSTYLE="0"
}

if [ -z "$1" ]; then
	echo "No arguments supplied, using defaults!"
	set_options
elif [ $# -gt 1 ]; then
	echo "Too many arguments supplied, using defaults!"
	set_options
else
	args=$(($(echo "$1" | sed 's/#/\n&/g' | grep -c '#')+1))
	if [ $args -lt 18 ]; then
		echo "Not enough arguments supplied [$args/18], using defaults!"
		set_options
	else
		echo "Using supplied arguments [$args/18] "
		DATA_SD=$(get_options "$1" 1);
		REGION=$(get_options "$1" 2);
		DATA_HD=$(get_options "$1" 3);
		BOUQUET_LIST=$(get_options "$1" 4);
		EXTRA=$(get_options "$1" 5);
		SORT=$(get_options "$1" 6);
		NUMBERED=$(get_options "$1" 7);
		NIT_SCAN=$(get_options "$1" 8);
		PLACEHOLDER=$(get_options "$1" 9);
		PARENTAL=$(get_options "$1" 10);
		DEFAULT=$(get_options "$1" 11);
		ORDERING=$(get_options "$1" 12);
		FREE_TO_AIR=$(get_options "$1" 13);
		CHECK_SCRIPT=$(get_options "$1" 14);
		STYLE=$(get_options "$1" 15);
		PICONLINK=$(get_options "$1" 16);
		PICONFOLDER=$(get_options "$1" 17);
		PICONSTYLE=$(get_options "$1" 18);
	fi
fi

if [ "$BOUQUET_LIST" = "0" ]; then
	DATA="$DATA_SD"
else
	DATA="$DATA_HD"
fi

case "$FTA_ONLY" in
	"1") FTA_ONLY="1";;
	"2") FTA_ONLY="2";;
	  *) FTA_ONLY="0";;
esac

case "$STYLE" in
	"2") STY="";;
	"3") STY="";;
	"4") STY=" = =  ";;
	"5") STY="";;
	  *) STY=" - -  ";;
esac

case "$PICONLINK" in
	"1") PICON_LINK="/usr/share/enigma2/picon";;
	"2") PICON_LINK="/picon";;
	"3") PICON_LINK="/media/usb/picon";;
	"4") PICON_LINK="/media/hdd/picon";;
	"5") PICON_LINK="/media/cf/picon";;
	  *) PICONLINK="0";;
esac

case "$PICONFOLDER" in
	"1") PICON_FOLDER="/usr/share/enigma2/picon";;
	"2") PICON_FOLDER="/picon";;
	"3") PICON_FOLDER="/media/usb/picon";;
	"4") PICON_FOLDER="/media/hdd/picon";;
	"5") PICON_FOLDER="/media/cf/picon";;
	  *) PICONFOLDER="$PICON_LINK";;
esac

case "$PICONSTYLE" in
	"1") PICON_STYLE="1";;
	"2") PICON_STYLE="2";;
	  *) PICON_STYLE="0";;
esac

echo "
DATA_SD=$DATA_SD		DVB_FRONTEND=$DVB_FRONTEND
REGION=$REGION		DVB_ADAPTER=$DVB_ADAPTER
DATA_HD=$DATA_HD		DVB_DEMUX=$DVB_DEMUX
BOUQUET_LIST=$BOUQUET_LIST	EXTRA=$EXTRA
SORT=$SORT		NUMBERED=$NUMBERED
NIT_SCAN=$NIT_SCAN		PLACEHOLDER=$PLACEHOLDER
PARENTAL=$PARENTAL		DEFAULT=$DEFAULT
ORDERING=$ORDERING		FREE_TO_AIR=$FREE_TO_AIR
STYLE=$STYLE		CHECK_SCRIPT=$CHECK_SCRIPT
PICONSTYLE=$PICONSTYLE		PICON_STYLE=$PICON_STYLE
PICONLINK=$PICONLINK   PICON_LINK=$PICON_LINK
PICONFOLDER=$PICONFOLDER PICON_FOLDER=$PICON_FOLDER
"

check_update(){
if [ -e $sab/bouquets.$1 ]; then
	if [ "$DEFAULT" = "1" ] || ! grep -q 'userbouquet.ukcvs' $sab/bouquets.$1 >/dev/null 2>&1; then
		sed -i '$d' /tmp/bouquets.$1
		cat $sab/bouquets.$1 | sed '/userbouquet.ukcvs/d' | sed '1d' >>/tmp/bouquets.$1
	else
		rm -f /tmp/bouquets.$1
	fi
fi
}

fwget(){
	wget -q -O - http://127.0.0.1/web/$1
}

reload(){
	# 0=both, 1=lamedb only, 2=userbouqets only
	fwget "servicelistreload?mode=$1" >/dev/null 2>&1
	sleep 3
}

make_lamedb(){
echo 'eDVB services /4/
transponders
011a0000:07d4:0002
	s 11778000:27500000:1:0:282:2:0
/
end
services
1038:011a0000:07d4:0002:2:0
EPG Background Audio.
p:BSkyB
end
Created by AutoBouquets E2' >$ldb
}

edit_lamedb(){
sed -i '/^transponders/c\
transponders\
011a0000:07d4:0002\
	s 11778000:27500000:1:0:282:2:0\
\/
/^services/c\
services\
1038:011a0000:07d4:0002:2:0\
EPG Background Audio.\
p:BSkyB' $ldb
}

make_lamedb_five(){
echo 'eDVB services /5/
# Transponders: t:dvb_namespace:transport_stream_id:original_network_id,FEPARMS
#     DVBS  FEPARMS:   s:frequency:symbol_rate:polarisation:fec:orbital_position:inversion:flags
#     DVBS2 FEPARMS:   s:frequency:symbol_rate:polarisation:fec:orbital_position:inversion:flags:system:modulation:rolloff:pilot
#     DVBT  FEPARMS:   t:frequency:bandwidth:code_rate_HP:code_rate_LP:modulation:transmission_mode:guard_interval:hierarchy:inversion:flags:system:plp_id
#     DVBC  FEPARMS:   c:frequency:symbol_rate:inversion:modulation:fec_inner:flags:system
# Services    : s:service_id:dvb_namespace:transport_stream_id:original_network_id:service_type:0,"service_name"[,p:provider_name][,c:cached_pid]*[,C:cached_capid]*[,f:flags]
t:011a0000:07d4:0002,s:11778000:27500000:1:2:282:2:0
s:1038:011a0000:07d4:0002:2:0,"EPG Background Audio.",p:BSkyB
# done. 1 channels and 1 services' >$ldb
}

edit_lamedb_five(){
ret_done=`grep '^# done.' $ldb`
sed -i '/^# done./c\
t:011a0000:07d4:0002,s:11778000:27500000:1:2:282:2:0\
s:1038:011a0000:07d4:0002:2:0,"EPG Background Audio.",p:BSkyB\
'"$ret_done" $ldb
}

valid_chk(){
	valid="false"
	cch=`fwget "subservices"|grep 'servicereference'|sed 's/.*>\(.*\)<.*$/\1/'`
	echo "$cch"|grep -q ':11A0000:'
	[ "$?" = "0" ] && valid="true"
}

def_zap(){
	fwget "zap?sRef=$defaultservice" >/dev/null 2>&1
	sleep 10
	valid_chk
}

lamedb_version_chk(){
	if ! `grep -q -m 1 '^eDVB services /5/' $ldb >/dev/null 2>&1`; then
		LAMEDB_VERSION=4
	else
		LAMEDB_VERSION=5
	fi
}
lamedb_version_chk

if [ "$CHECK_SCRIPT" = "1" ]; then
	#wake from standby?
	pwr=`fwget "powerstate"`
	echo "$pwr"|grep -q 'instandby>true'
	if [ "$?" = "0" ]; then
		echo "autobouquets script is waking up from standby, wait..."
		fwget "powerstate?newstate=4" >/dev/null 2>&1
		scriptwasinstandby="true"
		sleep 10
	fi

	#set the default fallback service to "EPG Background Audio."
	defaultservice="1:0:2:1038:7D4:2:11A0000:0:0:0:"
	dfzap="false"

	#check if system has a populated lamedb - if not, create one
	init="false"
	new_ldb="false"
	if [ ! -e $ldb ]; then
		init="true"
		new_ldb="true"
		[ "$NEW_LAMEDB" = "5" ] && LAMEDB_VERSION="5"
		if [ "$LAMEDB_VERSION" = "5" ]; then
			make_lamedb_five
		else
			make_lamedb
		fi
	else
		count=`wc -l <$ldb`
		[ "$count" -lt "9" ] && new_ldb="true"
		#next check if default 28.2E service transponder exists?
		if [ "$LAMEDB_VERSION" = "5" ]; then
			if ! `grep -q -m 1 '^t:011a0000:07d4:0002' $ldb`; then
				init="true"
				edit_lamedb_five
			fi
		else
			if ! `grep -q -m 1 '^011a0000:07d4:0002' $ldb`; then
				init="true"
				edit_lamedb
			fi
		fi
	fi

	#get current service (if it has one) for conditional zapback later
	pss=`fwget "subservices"|grep 'servicereference'|sed 's/.*>\(.*\)<.*$/\1/'`
	if [ "$pss" = "N/A" -o "$pss" = "" -o "$new_ldb" = "true" ]; then
	# "Sky News"
		pss="1:0:1:1260:7EA:2:11A0000:0:0:0"
		dfzap="true"
	fi

	if [ "$init" = "true" ]; then
		echo -e "AutoBouquets is initializing your system..\n"
	# reload lamedb only!
		reload 1
		dfzap="true"
		def_zap
	else
		#check current channel is an ACTIVE 28.2E service?
		valid_chk
		if [ "$valid" = "false" ]; then
			echo -e "autobouquets is zapping to the default\n28.2E service..\n"
			dfzap="true"
			def_zap
		fi
	fi

	#check we are FINALLY now on a valid 28.2E service!
	valid_chk
	if [ "$valid" = "false" ]; then
		echo "Aborting! Cannot find an ACTIVE 28.2E channel, sorry..."
		[ "$new_ldb" = "false" ] && fwget "zap?sRef=$pss" >/dev/null 2>&1
		exit 0
	fi
fi

if [ ! "$PICON_STYLE" = "0" ]; then
	e2restart="false"
	if [ ! -d $PICON_LINK ]; then
		mkdir -p "$PICON_LINK"
		e2restart="true"
	fi
	if [ ! -d $PICON_FOLDER ]; then
		mkdir -p "$PICON_FOLDER"
		e2restart="true"
	fi
	if [ "$PICON_STYLE" = "1" ]; then
		cd $fpath && . autopicon_convert.sh "$PICON_FOLDER"
		cd $PICON_FOLDER && rm -f 1_0_*_2_11A*_0_0_0.png
		cd $PICON_LINK && rm -f 1_0_*_2_11A*_0_0_0.png
	fi
	if [ "$e2restart" = "true" ]; then
		echo -e "\nNew Picon folder detected"
		echo -e "Enigma 2 restart required"
		echo -e "to recognize these picons!\n"
	fi
fi

echo -e "Downloading 28.2E Bouquets, please wait...\n"
date >/tmp/autobouquets.log

# READER args: 18 paramaters
cd $fpath
./autobouquetsreader \
"$DATA" \
"$REGION" \
"$EXTRA" \
"$SORT" \
"$NUMBERED" \
"$NIT_SCAN" \
"$PLACEHOLDER" \
"$PARENTAL" \
"$ORDERING" \
"$FREE_TO_AIR" \
"$STYLE" \
"$PICONLINK" \
"$PICONFOLDER" \
"$PICONSTYLE" \
"$LAMEDB_VERSION" \
"$DVB_FRONTEND" \
"$DVB_ADAPTER" \
"$DVB_DEMUX"

if [ "$?" != "0" ]; then
	clean_tmp
	echo "This channel has no valid data! Aborting..."
	fwget "zap?sRef=$pss" >/dev/null 2>&1
	exit 0
fi

check_update "tv"
check_update "radio"

if [ "$NIT_SCAN" = "1" ]; then
	tldb=/tmp/lamedb
	tnit=/tmp/nit_transponders.txt
	tps=/tmp/lamedb_transponders.txt
	tldbst=/tmp/lamedb_strip.txt
	tldbsv=/tmp/lamedb_services.txt
	#update lamedb with services from BAT, transponders from NIT
	echo "Updating 28.2E Transponders and Services.."
	if [ "$LAMEDB_VERSION" = "5" ]; then
		sed '/^t:.*:.*:0002,.*$/d;/^# done.*$/d;/^s.*$/d' $ldb >$tps
		cat $tnit >>$tps
		sed -n '/^s.*$/p' $ldb | sed '/^s:.*:.*:.*:0002:.*/d' >$tldbst
		cat $tps $tldbst $tldbsv >$tldb
		channels_cnt=`grep -c '^t' $tldb`
		services_cnt=`grep -c '^s' $tldb`
		echo "# done. $channels_cnt channels and $services_cnt services" >>$tldb
	else
		end=`grep -n '^end' $ldb | sed 'q' | cut -d ":" -f1`
		sed -n "1,$end{p}" $ldb | sed '$d' |\
		sed '/^.*:.*:0002$/,/\//d' >$tps
		cat $tnit >>$tps
		echo "end" >>$tps
		sed -n "/^services/,/^end/p" $ldb | sed '$d' |\
		sed '/.*:.*:.*:0002:.*/,/^p:/d' >$tldbst
		cat $tps $tldbst $tldbsv >$tldb
		echo "end" >>$tldb
		echo "Updated by AutoBouquets on `date`" >>$tldb
	fi
	rm -f $tps $tldbst $tldbsv $tnit
	mv $tldb $ldb
fi

#write autobouquets info to last bouquet
SD='#SERVICE 1:0:1:2331:7ee:2:11a0000:0:0:0:
#DESCRIPTION'
echo "#NAME $STY AutoBouquets E2 28.2E $STY Credits
#SERVICE 1:64:1:0:0:0:0:0:0:0:
#DESCRIPTION AutoBouquets E2 28.2E - - $versiondate - -
$SD -
$SD Bouquets created on:
$SD `date`
$SD -
$SD Main Developer: LraiZer
$SD -
$SD Enhancements: PaphosAL
$SD Testers: Lincsat, Bazinga, Abu Baniaz
$SD - - - http://www.ukcvs.net - - -
$SD -
$SD Credits for DVB scanner binary:
$SD - - Sandro Cavazzoni aka Skaman - -
$SD http://www.sifteam.eu
$SD - - Andrew Blackburn aka AndyBlac - -
$SD http://www.world-of-satellite.com" >/tmp/userbouquet.ukcvs_about.tv

# conditionally nuke Adult/Gaming bouquets if PC is active
if [ "$PARENTAL" = "1" ]; then
	rm -f $sab/*0d.tv $sab/*0f.tv
	rm -f /tmp/*0d.tv /tmp/*0f.tv
fi

# update new bouquets
mv /tmp/*.tv $sab
mv /tmp/*.radio $sab
echo -e "Updating Bouquets, lamedb version $LAMEDB_VERSION\n"
if [ "$NIT_SCAN" = "1" ]; then
# reload both lamedb and Userbouquets
	reload 0
else
# E2 service scan created lamedb, reload Userbouquets only
	reload 2
fi

date
stop_time=$(expr `date +%s` - $start_time)
log_time="Process Time: "$(expr $stop_time / 60)" minutes "$(expr $stop_time % 60)" seconds"
echo -e "$log_time\n"
echo "lamedb version $LAMEDB_VERSION, "`date` >>/tmp/autobouquets.log
echo "$log_time" >>/tmp/autobouquets.log

mv /tmp/autobouquets.* $fpath/

if [ "$CHECK_SCRIPT" = "1" ]; then
	#zap back to pre scan service?
	[ "$dfzap" = "true" ] && fwget "zap?sRef=$pss" >/dev/null 2>&1
	#awake to standby?
	if [ "$scriptwasinstandby" = "true" ]; then
		sleep 60
		fwget "powerstate?newstate=5" >/dev/null 2>&1
		echo "autobouquets is going back to sleep in standby!"
	fi
fi

exit 0


#!/bin/sh
# AutoBouquetsIptvEPG for 28.2E by LraiZer
# script tool for use after running BMX,JMX and AutoBouquets
# auto assigns jmx,bmx iptv bouquets with live UK stream epg
# service references generated by AutoBouquets E2 for 28.2E.

VERSIONDATE="15 Apr 2024"

# assign a channel id for alias names that change often
# or are not automatically found by the detection logic
# channelid=iptvname,iptvname,iptvname,iptvname,...

CHANNEL_ID_ALIAS="
1121=FRANCE24HD,FranceEng
1136=SkyDocmntrsHD,SkyDocumentariesHD
1199=SkyAnimationHD
1289=Viaplay1HD,ViaplaySports1
1290=Viaplay2HD,ViaplaySports2,ViaplayExtra
1336=TNTUltimateUHD,TNTSportsUltimateUHD
1409=SkyPremiere
1849=Nicktoons
1872=Together,togetherTV
2053=BBCRB1HD,BBCRedButton
2402=AnimalPlanet,AnimalPlntHD
3000=BBCOneLonHD,BBCOneHD
3007=BBCOneNWHD,BBCOneNorthWest
3020=BBCParlHD,BBCParliament
3352=TRUECRIME,CBSReality
3354=JewelleryMaker,JewelryMaker
3592=FoodNetwrk+1,FoodNetwork+1
3602=TRUECRIME+1,CBSReality+1
3605=LEGENDXTRA,horrorxtra
3617=TRUECRIMEX,Realityxtra
3636=RacingTV,RacingUK
3751=Gromance+1,GreatRomance+1
4014=SkyActionHD
4015=SkyGreatsHD
4016=SkyDramaHD,SkyDramaRomHD
4017=SkySciFi/HorrorHD
4018=SkyFamilyHD
4019=SkyComedyHD
4020=SkySelectHD
4021=SkyPremiereHD
4033=SkyHitsHD
4062=SkyThrillerHD
4262=POPMax,TinyPop
4263=POPMax+1,TinyPop+1
4502=LEGENDXTRA+1,horrorxtra+1
5272=RacingTVHD,RacingUKHD
5601=CartoonNetwrk,CartoonNetwork
6504=ITV1LondonHD,ITV1HD
7211=SkySpMEUHD,SkySpMainEvUHD
7221=SkySpUltraHD01,SkySpUltraHD02
"

start_time=`date +%s`
E2BQ_PATH="/etc/enigma2"
IEPG_PATH="/tmp/iepg"
AUTOBOUQUETS_PATH=/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
echo "AutoBouquets IptvEPG for 28.2E - version $VERSIONDATE"
echo -e "`date`\n\nCHANNEL_ID_ALIAS missing services log\n" > /tmp/autoiepg_log.txt
if [ -e $AUTOBOUQUETS_PATH/autobouquets.csv ]; then
	BOUQUETS_CSV=`sed 1d $AUTOBOUQUETS_PATH/autobouquets.csv | sed "s/[ ~!#-']//g"`
	COUNT=0
	OIFS=$IFS
	IFS='
'
	echo "Updating live UK IPTV EPG, working..."
	for BOUQUET in `ls $E2BQ_PATH/*_live_*_UK_*.tv`; do
		SERVICES=`grep -i '^#SERVICE.*http*' $BOUQUET`
		if [ $(echo "$SERVICES"|wc -l) -lt 2 ]; then
		 continue
		fi
		COUNT=$(($COUNT + 1))
		if [ $COUNT -eq 1 ]; then
			mkdir -p $IEPG_PATH
		fi
		cp $BOUQUET $IEPG_PATH
	done
	if [ $COUNT -eq 0 ]; then
		echo "No live UK services"
		exit 0
	fi
	for BOUQUET in `ls $IEPG_PATH/*_live_*_UK_*.tv`; do
		SERVICES=`grep -i '^#SERVICE.*http*' $BOUQUET`
		for SERVICE_IPTV in $SERVICES; do
			SERVICE_NAME=`echo "$SERVICE_IPTV" | grep -o '[^:]\+$' |\
			sed "s/ITV /ITV1/g;s/FHD/HD/g;s/4k\|4K/UHD/g;s/Sky Sports/SkySp/g;s/[ ~!#-']//g"`
			if ! `echo "$BOUQUETS_CSV" | grep -i -q -m 1 "$SERVICE_NAME"`; then
				if `echo "$CHANNEL_ID_ALIAS" | grep -i -q -m 1 "$SERVICE_NAME"`; then
					CHANNEL_ID=`echo "$CHANNEL_ID_ALIAS" | grep -i -m 1 "$SERVICE_NAME"  | cut -d '=' -f 1`
					SERVICE_CSV=`echo "$BOUQUETS_CSV" | grep -m 1 ",$CHANNEL_ID,"`
					SERVICE_REF=`echo "$SERVICE_CSV" | cut -d "," -f 8 | cut -c 2-`
				else
					echo "$SERVICE_NAME" >> /tmp/autoiepg_log.txt
					continue
				fi
			else
				SERVICE_CSV=`echo "$BOUQUETS_CSV" | grep -i -m 1 "$SERVICE_NAME"`
				SERVICE_REF=`echo "$SERVICE_CSV" | cut -d "," -f 8 | cut -c 2-`
			fi
			SERVICE_STREAM=`echo "$SERVICE_IPTV" | sed 's/^#SERVICE *....\(.*\)http.*$/\1/'`
			sed -i s"/$SERVICE_STREAM/$SERVICE_REF/" "$BOUQUET"
			#echo "service updated:$SERVICE_STREAM $SERVICE_NAME" >> /tmp/autoiepg_log.txt
		done
	done
	IFS=$OIFS
	mv $IEPG_PATH/*_live_*_UK_*.tv $E2BQ_PATH
	rm -rf $IEPG_PATH
else
	echo -e "missing autobouquets.csv:\nAutoBouquets IptvEPG requires AutoBouquets to run first!"
fi
stop_time=$(expr `date +%s` - $start_time)
log_time="Process Time: "$(expr $stop_time / 60)" minutes "$(expr $stop_time % 60)" seconds"
echo "$log_time"; wget -q -O - http://127.0.0.1/web/servicelistreload?mode=2 >/dev/null 2>&1

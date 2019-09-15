# for localized messages
from . import _

from Screens.InfoBar import InfoBar
from Screens.Screen import Screen
from Screens.Console import Console
from Screens.MessageBox import MessageBox
from Components.MenuList import MenuList
from Components.ActionMap import ActionMap
from Components.Button import Button
from Tools.Directories import fileExists
from Plugins.Plugin import PluginDescriptor
from Components.ScrollLabel import ScrollLabel
from Components.ConfigList import ConfigListScreen
from Components.config import config
from Components.Sources.StaticText import StaticText
from Components.Label import Label
from Components.Pixmap import MultiPixmap, Pixmap
from Screens.Standby import Standby, inStandby
from enigma import eDVBDB, eTimer, eServiceReference, eConsoleAppContainer
from datetime import date
from time import localtime, time, strftime, mktime, sleep
from os import stat, path
from sys import modules

from Screens.ServiceScan import ServiceScan
from Components.NimManager import nimmanager, getConfigSatlist
from enigma import eComponentScan
from Screens.ScanSetup import getInitialTransponderList

ab_version = "17 December 2018"
defaultservice = "1:0:1:1260:7EA:2:11A0000:0:0:0:"	# "Sky News"
initscanservice = "1:0:2:1038:7D4:2:11A0000:0:0:0:"	# "EPG Background Audio."

arealist = []
arealist.append(("4097#19#4101", "Atherstone"))
arealist.append(("4097#12#4101", "Border England"))
arealist.append(("4098#36#4102", "Border Scotland"))
arealist.append(("4099#65#4103", "Brighton"))
arealist.append(("4097#03#4101", "Central Midlands"))
arealist.append(("4100#34#4104", "Channel Isles"))
arealist.append(("4098#39#4102", "Dundee"))
arealist.append(("4097#20#4101", "East Midlands"))
arealist.append(("4097#02#4101", "Essex"))
arealist.append(("4097#24#4101", "Gloucester"))
arealist.append(("4098#35#4102", "Grampian"))
arealist.append(("4097#07#4101", "Granada"))
arealist.append(("4099#70#4103", "Henley On Thames"))
arealist.append(("4099#43#4103", "HTV Wales"))
arealist.append(("4097#04#4101", "HTV West"))
arealist.append(("4099#63#4103", "HTV West / Thames Valley"))
arealist.append(("4097#29#4101", "Humber"))
arealist.append(("4097#01#4101", "London"))
arealist.append(("4097#18#4101", "London / Essex"))
arealist.append(("4099#66#4103", "London / Thames Valley"))
arealist.append(("4099#64#4103", "London Kent"))
arealist.append(("4097#11#4101", "Meridian East"))
arealist.append(("4099#68#4103", "Meridian North"))
arealist.append(("4097#05#4101", "Meridian South"))
arealist.append(("4097#10#4101", "Meridian South East"))
arealist.append(("4099#45#4103", "Merseyside"))
arealist.append(("4097#21#4101", "Norfolk"))
arealist.append(("4099#62#4103", "North East Midlands"))
arealist.append(("4097#08#4101", "North West Yorkshire"))
arealist.append(("4097#26#4101", "North Yorkshire"))
arealist.append(("4100#33#4104", "Northern Ireland"))
arealist.append(("4099#71#4103", "Oxford"))
arealist.append(("4100#50#4104", "Republic of Ireland"))
arealist.append(("4099#41#4103", "Ridge Hill"))
arealist.append(("4099#61#4103", "Scarborough"))
arealist.append(("4098#37#4102", "Scottish East"))
arealist.append(("4098#38#4102", "Scottish West"))
arealist.append(("4099#60#4103", "Sheffield"))
arealist.append(("4097#28#4101", "South Lakeland"))
arealist.append(("4099#72#4103", "South Yorkshire"))
arealist.append(("4099#69#4103", "Tees"))
arealist.append(("4097#09#4101", "Thames Valley"))
arealist.append(("4097#27#4101", "Tring"))
arealist.append(("4097#13#4101", "Tyne"))
arealist.append(("4100#32#4104", "Wales"))
arealist.append(("4097#25#4101", "West Anglia"))
arealist.append(("4099#67#4103", "West Dorset"))
arealist.append(("4097#06#4101", "Westcountry"))

bouquetlist = []
bouquetlist.append(("False", _("SD")))
bouquetlist.append(("True", _("HD")))

stylelist = []
stylelist.append(("0", _("default")))
stylelist.append(("1", _("style 1")))
stylelist.append(("2", _("style 2")))
stylelist.append(("3", _("style 3")))
stylelist.append(("4", _("style 4")))
stylelist.append(("5", _("style 5")))

sortlist = []
sortlist.append(("0", _("Large 1st Bouquet")))
sortlist.append(("1", _("HD First")))
sortlist.append(("2", _("Sort 1")))
sortlist.append(("3", _("Sort 2")))
sortlist.append(("4", _("Sort 3")))

piconlink = []
piconlink.append(("1", _("/usr/share/enigma2/picon")))
piconlink.append(("2", _("/picon")))
piconlink.append(("3", _("/media/usb/picon")))
piconlink.append(("4", _("/media/hdd/picon")))
piconlink.append(("5", _("/media/cf/picon")))

piconfolder = []
piconfolder.append(("1", _("/usr/share/enigma2/picon")))
piconfolder.append(("2", _("/picon")))
piconfolder.append(("3", _("/media/usb/picon")))
piconfolder.append(("4", _("/media/hdd/picon")))
piconfolder.append(("5", _("/media/cf/picon")))

piconstyle = []
piconstyle.append(("0", _("no")))
piconstyle.append(("1", _("Ordinary")))
piconstyle.append(("2", _("ServiceNamePicons")))

freetoair = []
freetoair.append(("0", _("All Channels")))
freetoair.append(("1", _("FTA Only")))
freetoair.append(("2", _("Available HD")))

from Components.config import config, configfile, ConfigSubsection, ConfigYesNo, ConfigSelection, ConfigText, ConfigNumber, NoSave, ConfigClock, getConfigListEntry
config.autobouquets = ConfigSubsection()
config.autobouquets.area = ConfigSelection(default = None, choices = arealist)
config.autobouquets.bouquetlist = ConfigSelection(default = "False", choices = bouquetlist)
config.autobouquets.extra = ConfigYesNo(default = True)
config.autobouquets.sort = ConfigSelection(default = "1", choices = sortlist)
config.autobouquets.numbered = ConfigYesNo(default = False)
config.autobouquets.nitscan = ConfigYesNo(default = True)
config.autobouquets.placeholder = ConfigYesNo(default = True)
config.autobouquets.parental = ConfigYesNo(default = True)
config.autobouquets.default = ConfigYesNo(default = True)
config.autobouquets.ordering = ConfigYesNo(default = False)
config.autobouquets.freetoair = ConfigSelection(default = "2", choices = freetoair)
config.autobouquets.checkscript = ConfigYesNo(default = True)
config.autobouquets.style = ConfigSelection(default = "0", choices = stylelist)
config.autobouquets.piconlink = ConfigSelection(default = "1", choices = piconlink)
config.autobouquets.piconfolder = ConfigSelection(default = "3", choices = piconfolder)
config.autobouquets.piconstyle = ConfigSelection(default = "0", choices = piconstyle)
config.autobouquets.schedule = ConfigYesNo(default = False)
config.autobouquets.scheduletime = ConfigClock(default = 0) # 1:00
config.autobouquets.repeattype = ConfigSelection(default = "daily", choices = [("daily", _("Daily")), ("weekly", _("Weekly")), ("monthly", _("30 Days"))])
config.autobouquets.retry = ConfigNumber(default = 30)
config.autobouquets.retrycount = NoSave(ConfigNumber(default = 0))
config.autobouquets.nextscheduletime = NoSave(ConfigNumber(default = 0))
config.autobouquets.lastlog = ConfigText(default=' ', fixed_size=False)

scriptwasinstandby = False
autoAutoBouquetsTimer = None

def AutoBouquetsautostart(reason, session=None, **kwargs):
	"called with reason=1 to during /sbin/shutdown.sysvinit, with reason=0 at startup?"
	global autoAutoBouquetsTimer
	global _session
	now = int(time())
	if reason == 0:
		print "[AutoBouquets] AutoStart Enabled"
		if session is not None:
			_session = session
			if autoAutoBouquetsTimer is None:
				autoAutoBouquetsTimer = AutoAutoBouquetsTimer(session)
	else:
		print "[AutoBouquets] Stop"
		autoAutoBouquetsTimer.stop()

class AutoAutoBouquetsTimer:
	def __init__(self, session):
		self.session = session
		self.autobouquetstimer = eTimer()
		self.autobouquetstimer.callback.append(self.AutoBouquetsonTimer)
		self.autobouquetsactivityTimer = eTimer()
		self.autobouquetsactivityTimer.timeout.get().append(self.autobouquetsdatedelay)
		now = int(time())
		global AutoBouquetsTime
		if config.autobouquets.schedule.value:
			print "[AutoBouquets] AutoBouquets Schedule Enabled at ", strftime("%c", localtime(now))
			if now > 1262304000:
				self.autobouquetsdate()
			else:
				print "[AutoBouquets] AutoBouquets Time not yet set."
				AutoBouquetsTime = 0
				self.autobouquetsactivityTimer.start(36000)
		else:
			AutoBouquetsTime = 0
			print "[AutoBouquets] AutoBouquets Schedule Disabled at", strftime("(now=%c)", localtime(now))
			self.autobouquetsactivityTimer.stop()

	def autobouquetsdatedelay(self):
		self.autobouquetsactivityTimer.stop()
		self.autobouquetsdate()

	def getAutoBouquetsTime(self):
		backupclock = config.autobouquets.scheduletime.value
		nowt = time()
		now = localtime(nowt)
		return int(mktime((now.tm_year, now.tm_mon, now.tm_mday, backupclock[0], backupclock[1], 0, now.tm_wday, now.tm_yday, now.tm_isdst)))

	def autobouquetsdate(self, atLeast = 0):
		self.autobouquetstimer.stop()
		global AutoBouquetsTime
		AutoBouquetsTime = self.getAutoBouquetsTime()
		now = int(time())
		if AutoBouquetsTime > 0:
			if AutoBouquetsTime < now + atLeast:
				if config.autobouquets.repeattype.value == "daily":
					AutoBouquetsTime += 24*3600
					while (int(AutoBouquetsTime)-30) < now:
						AutoBouquetsTime += 24*3600
				elif config.autobouquets.repeattype.value == "weekly":
					AutoBouquetsTime += 7*24*3600
					while (int(AutoBouquetsTime)-30) < now:
						AutoBouquetsTime += 7*24*3600
				elif config.autobouquets.repeattype.value == "monthly":
					AutoBouquetsTime += 30*24*3600
					while (int(AutoBouquetsTime)-30) < now:
						AutoBouquetsTime += 30*24*3600
			next = AutoBouquetsTime - now
			self.autobouquetstimer.startLongTimer(next)
		else:
			AutoBouquetsTime = -1
		print "[AutoBouquets] AutoBouquets Time set to", strftime("%c", localtime(AutoBouquetsTime)), strftime("(now=%c)", localtime(now))
		return AutoBouquetsTime

	def backupstop(self):
		self.autobouquetstimer.stop()

	def AutoBouquetsonTimer(self):
		self.autobouquetstimer.stop()
		now = int(time())
		wake = self.getAutoBouquetsTime()
		# If we're close enough, we're okay...
		atLeast = 0
		if wake - now < 60:
			print "[AutoBouquets] AutoBouquets onTimer occured at", strftime("%c", localtime(now))
			from Screens.Standby import inStandby
			if not inStandby:
				message = _("You are about to update your bouquets,\nDo you want to allow this?")
				ybox = self.session.openWithCallback(self.doAutoBouquets, MessageBox, message, MessageBox.TYPE_YESNO, timeout = 30)
				ybox.setTitle('Scheduled AutoBouquets.')
			else:
				print "[AutoBouquets] in Standby, so just running backup", strftime("%c", localtime(now))
				self.doAutoBouquets(True)
		else:
			print '[AutoBouquets] Where are not close enough', strftime("%c", localtime(now))
			self.autobouquetsdate(60)

	def doAutoBouquets(self, answer):
		now = int(time())
		if answer is False:
			if config.autobouquets.retrycount.value < 2:
				print '[AutoBouquets] Number of retries',config.autobouquets.retrycount.value
				print "[AutoBouquets] AutoBouquets delayed."
				repeat = config.autobouquets.retrycount.value
				repeat += 1
				config.autobouquets.retrycount.value = repeat
				AutoBouquetsTime = now + (int(config.autobouquets.retry.value) * 60)
				print "[AutoBouquets] AutoBouquets Time now set to", strftime("%c", localtime(AutoBouquetsTime)), strftime("(now=%c)", localtime(now))
				self.autobouquetstimer.startLongTimer(int(config.autobouquets.retry.value) * 60)
			else:
				atLeast = 60
				print "[AutoBouquets] Enough Retries, delaying till next schedule.", strftime("%c", localtime(now))
				self.session.open(MessageBox, _("Enough Retries, delaying till next schedule."), MessageBox.TYPE_INFO, timeout = 10)
				config.autobouquets.retrycount.value = 0
				self.autobouquetsdate(atLeast)
		else:
			print "[AutoBouquets] Running AutoBouquets", strftime("%c", localtime(now))
			from Screens.Standby import inStandby
			global scriptwasinstandby
			if inStandby:
				scriptwasinstandby = True
				self.wasinstandby = True
				inStandby.Power()
			else:
				scriptwasinstandby = False
				self.wasinstandby = False
			self.timer = eTimer()
			self.timer.callback.append(self.doautostartscan)
			self.timer.start(10000, 1)

	def doautostartscan(self):
		self.AutoBouquets = AutoBouquets(self.session)
		postScanService = self.session.nav.getCurrentlyPlayingServiceReference()
		self.timer = eTimer()
		if config.autobouquets.nitscan.getValue():
			self.timer.callback.append(self.AutoBouquets.go())
		else:
			self.timer.callback.append(self.AutoBouquets.startservicescan(postScanService, self.wasinstandby))
		self.timer.start(5000, 1)

class AutoBouquets(Screen):
	skin = """
		<screen position="center,center" size="600,550" title="AutoBouquets E2 for 28.2E">
			<widget name="area" position="0,10" size="300,30" font="Regular; 22" halign="right" zPosition="2" transparent="0" />
			<widget name="areavalue" position="310,10" size="280,30" font="Regular;22" zPosition="2" />
			<widget name="fta" position="0,45" size="300,30" font="Regular; 22" halign="right" zPosition="2" transparent="0" />
			<widget name="ftavalue" position="310,45" size="300,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="sort" position="0,80" size="300,30" font="Regular; 22" halign="right" zPosition="2" transparent="0" />
			<widget name="sortvalue" position="310,80" size="300,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="status" position="0,115" size="300,30" font="Regular; 22" halign="right" zPosition="2" transparent="0"/>
			<widget name="status2" position="310,115" size="205,30" font="Regular;22" zPosition="5"/>
			<widget name="picons" position="0,150" size="300,30" font="Regular; 22" halign="right" zPosition="2" transparent="0" />
			<widget name="piconstyle" position="310,150" size="205,30" font="Regular;22" zPosition="5"/>
			<widget name="orderingcheck" position="15,250" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="ordering" position="50,250" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="numcheck" position="315,250" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="num" position="350,250" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="otherextra" position="15,280" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="otherx" position="50,280" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="parentalcheck" position="315,280" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="parental" position="350,280" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="nitcheck" position="15,310" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="nit" position="50,310" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="scriptcheck" position="315,310" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="scriptc" position="350,310" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="placehcheck" position="15,340" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="placeh" position="50,340" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<widget name="defcheck" position="315,340" size="32,32" alphatest="on" zPosition="1" pixmaps="skin_default/icons/lock_off.png,skin_default/icons/lock_on.png"/>
			<widget name="def" position="350,340" size="250,30" font="Regular; 22" halign="left" zPosition="2" transparent="0" />
			<ePixmap pixmap="skin_default/buttons/key_info.png" position="207,470" size="40,40" alphatest="blend" transparent="1" zPosition="3" />
			<ePixmap pixmap="skin_default/buttons/key_menu.png" position="353,470" size="40,40" alphatest="blend" transparent="1" zPosition="3" />
			<widget name="key_red" position="10,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="red" position="10,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/red.png" transparent="1" alphatest="on" />
			<widget name="key_green" position="157,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="green" position="157,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/green.png" transparent="1" alphatest="on" />
			<widget name="key_yellow" position="304,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="yellow" position="304,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/yellow.png" transparent="1" alphatest="on" />
			<widget name="key_blue" position="451,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="blue" position="451,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/blue.png" transparent="1" alphatest="on" />
		</screen>"""

	def __init__(self, session, args = 0):
		self.session = session
		Screen.__init__(self, session)
		self['area'] = Label(_('Area:'))
		self['areavalue'] = Label()
		self['otherx'] = Label(_('Other Extra'))
		self['otherextra'] = MultiPixmap()
		self['picons'] = Label(_('Auto Picon:'))
		self['piconstyle'] = Label()
		self['num'] = Label(_('Add No. to Name'))
		self['numcheck'] = MultiPixmap()
		self['nit'] = Label(_('NIT Scan'))
		self['nitcheck'] = MultiPixmap()
		self['placeh'] = Label(_('Hide Placeholder'))
		self['placehcheck'] = MultiPixmap()
		self['parental'] = Label(_('Hide Adult'))
		self['parentalcheck'] = MultiPixmap()
		self['def'] = Label(_('Default Bouquets'))
		self['defcheck'] = MultiPixmap()
		self['ordering'] = Label(_('Channel Swap'))
		self['orderingcheck'] = MultiPixmap()
		self['fta'] = Label(_('Scan Mode:'))
		self['ftavalue'] = Label()
		self['scriptc'] = Label(_('Script Check'))
		self['scriptcheck'] = MultiPixmap()
		self["status"] = Label(_("Next Update:"))
		self["status2"] = Label()
		self['sort'] = Label(_('Sort Option:'))
		self['sortvalue'] = Label()
		self["key_red"] = Button(_("Cancel"))
		self["key_green"] = Button(_("Start"))
		self["key_yellow"] = Button(_("Help file"))
		self["key_blue"] = Button(_("About"))
		self["myActionMap"] = ActionMap(["SetupActions", "ColorActions", "MenuActions", "TimerEditActions"],
		{
			"red": self.cancel,
			"green": self.question,
			"yellow": self.help,
			"blue": self.showAbout,
			"cancel": self.cancel,
			"ok": self.question,
			'log': self.showLog,
			"menu": self.createSetup,
			"1": self.cancel,
			"2": self.question,
			"3": self.help,
			"4": self.showAbout,
			"5": self.showLog,
			"6": self.createSetup,
		}, -2)
		self.wasinstandby = False
		self.onLayoutFinish.append(self.doneConfiguring)

	def createSetup(self):
		self.session.openWithCallback(self.setupDone, AutoBouquetsMenu)

	def setupDone(self):
		self.doneConfiguring()

	def doneConfiguring(self):
		self['areavalue'].setText(config.autobouquets.area.getText())
		if config.autobouquets.extra.getValue():
			self['otherextra'].setPixmapNum(1)
		else:
			self['otherextra'].setPixmapNum(0)
		self['otherextra'].show
		self['piconstyle'].setText(config.autobouquets.piconstyle.getText())
		if config.autobouquets.numbered.getValue():
			self['numcheck'].setPixmapNum(1)
		else:
			self['numcheck'].setPixmapNum(0)
		self['numcheck'].show
		if config.autobouquets.nitscan.getValue():
			self['nitcheck'].setPixmapNum(1)
		else:
			self['nitcheck'].setPixmapNum(0)
		self['nitcheck'].show
		if config.autobouquets.placeholder.getValue():
			self['placehcheck'].setPixmapNum(1)
		else:
			self['placehcheck'].setPixmapNum(0)
		self['placehcheck'].show
		if config.autobouquets.parental.getValue():
			self['parentalcheck'].setPixmapNum(1)
		else:
			self['parentalcheck'].setPixmapNum(0)
		self['parentalcheck'].show
		if config.autobouquets.default.getValue():
			self['defcheck'].setPixmapNum(1)
		else:
			self['defcheck'].setPixmapNum(0)
		self['defcheck'].show
		if config.autobouquets.ordering.getValue():
			self['orderingcheck'].setPixmapNum(1)
		else:
			self['orderingcheck'].setPixmapNum(0)
		self['orderingcheck'].show
		self['ftavalue'].setText(config.autobouquets.freetoair.getText())
		if config.autobouquets.checkscript.getValue():
			self['scriptcheck'].setPixmapNum(1)
		else:
			self['scriptcheck'].setPixmapNum(0)
		self['scriptcheck'].show
		self['sortvalue'].setText(config.autobouquets.sort.getText())
		now = int(time())
		global AutoBouquetsTime
		if config.autobouquets.schedule.value:
			if autoAutoBouquetsTimer is not None:
				print "[AutoBouquets] AutoBouquets Schedule Enabled at", strftime("%c", localtime(now))
				autoAutoBouquetsTimer.autobouquetsdate()
		else:
			if autoAutoBouquetsTimer is not None:
				AutoBouquetsTime = 0
				print "[AutoBouquets] AutoBouquets Schedule Disabled at", strftime("%c", localtime(now))
				autoAutoBouquetsTimer.backupstop()
		try:
			if AutoBouquetsTime > 0:
				t = localtime(AutoBouquetsTime)
				autobouquetstext = strftime(_("%a %e %b  %-H:%M"), t)
			else:
				autobouquetstext = ""
		except:
			AutoBouquetsTime = 0
			autobouquetstext = ""
		self["status2"].setText(str(autobouquetstext))

	def showLog(self):
		self.session.open(AutoBouquetsLogView)

	def showAbout(self):
		self.session.open(AutoBouquetsAbout)

	def question(self):
		try:
			self.postScanService = self.session.nav.getCurrentlyPlayingServiceReference()
		except:
			self.postScanService = eServiceReference(defaultservice)
		returnValue = config.autobouquets.area.getValue()
		if returnValue != "None":
			self.channelupdate()
		else:
			question = self.session.open(MessageBox,_('Please first setup, by pressing menu button'), MessageBox.TYPE_INFO)
			question.setTitle(_("AutoBouquets E2 for 28.2E"))

	def channelupdate(self):
		if config.autobouquets.nitscan.getValue():
			message = _("Do you want to perform a NIT scan")
		else:
			message = _("Do you want to perform a service scan")
		self.question = self.session.openWithCallback(self.updatecallback,MessageBox,message, MessageBox.TYPE_YESNO)
		self.question.setTitle(_("AutoBouquets E2 for 28.2E"))

	def updatecallback(self, val):
		if val:
			if config.autobouquets.nitscan.getValue():
				self.go()
			else:
				self.startservicescan(self.postScanService)
		else:
			self.go()

	def startservicescan(self, postScanService=None, wasinstandby=False):
		self.wasinstandby = wasinstandby
		self.postScanService = postScanService

		tlist = []
		known_networks = [ ]
		nims_to_scan = [ ]

		for nim in nimmanager.nim_slots:
			# collect networks provided by this tuner
			need_scan = False
			networks = self.getNetworksForNim(nim)

			# we only need to scan on the first tuner which provides a network.
			# this gives the first tuner for each network priority for scanning.
			for x in networks:
				if x not in known_networks:
					need_scan = True
					print x, "not in ", known_networks
					known_networks.append(x)

# 			print "nim %d provides" % nim.slot, networks
# 			print "known:", known_networks
#
			# don't offer to scan nims if nothing is connected
			if not nimmanager.somethingConnected(nim.slot):
				need_scan = False

			if need_scan:
				nims_to_scan.append(nim)

		# we save the config elements to use them on keyGo
		self.nim_enable = [ ]

		if len(nims_to_scan):
			for nim in nims_to_scan:
				nimconfig = ConfigYesNo(default = True)
				nimconfig.nim_index = nim.slot
				self.nim_enable.append(nimconfig)

		self.scanList = []
		self.known_networks = set()
		self.nim_iter=0
		self.buildTransponderList()

	def getNetworksForNim(self, nim):
		networks = [ ]
		if nim.isCompatible("DVB-S"):
			tmpnetworks = nimmanager.getSatListForNim(nim.slot)
			for x in tmpnetworks:
				if x[0] == 282:
					networks.append(x)
		else:
			# empty tuners provide no networks.
			networks = [ ]
		return networks

	def buildTransponderList(self): # this method is called multiple times because of asynchronous stuff
		APPEND_NOW = 0
		SEARCH_CABLE_TRANSPONDERS = 1
		action = APPEND_NOW

		n = self.nim_iter < len(self.nim_enable) and self.nim_enable[self.nim_iter] or None
		self.nim_iter += 1
		if n:
			if n.value: # check if nim is enabled
				flags = 0
				nim = nimmanager.nim_slots[n.nim_index]
				networks = set(self.getNetworksForNim(nim))
				networkid = 0
				# don't scan anything twice
				networks.discard(self.known_networks)

				tlist = [ ]
				if nim.isCompatible("DVB-S"):
					# get initial transponders for each satellite to be scanned
					for sat in networks:
						getInitialTransponderList(tlist, sat[0])
				else:
					assert False
				flags |= eComponentScan.scanNetworkSearch #FIXMEEE.. use flags from cables / satellites / terrestrial.xml
				flags |= eComponentScan.scanRemoveServices

				if action == APPEND_NOW:
					self.scanList.append({"transponders": tlist, "feid": nim.slot, "flags": flags})
				elif action == SEARCH_CABLE_TRANSPONDERS:
					self.flags = flags
					self.feid = nim.slot
					self.networkid = networkid
					self.startCableTransponderSearch(nim.slot)
					return
				else:
					assert False

			self.buildTransponderList() # recursive call of this function !!!
			return
		# when we are here, then the recursion is finished and all enabled nims are checked
		# so we now start the real transponder scan
		self.startScan(self.scanList)

	def startScan(self, scanList):
		if len(scanList):
			self.timer = eTimer()
			self.start()
			self.RunServiceScan = self.session.openWithCallback(self.finished_cb, ServiceScan, scanList)

	def start(self):
		if self.finish_check not in self.timer.callback:
			self.timer.callback.append(self.finish_check)
		self.timer.startLongTimer(60)

	def stop(self):
		if self.finish_check in self.timer.callback:
			self.timer.callback.remove(self.finish_check)
		self.timer.stop()

	def finish_check(self):
		try:
			if not self.RunServiceScan["scan"].isDone():
				self.timer.startLongTimer(10)
			else:
				self.stop()
				self.RunServiceScan.close()
				self.RunServiceScan = None
		except:
			self.RunServiceScan = None
			self.stop

	def finished_cb(self, postScanService=None):
		self.session.nav.playService(self.postScanService)
		self.go()

	def keyCancel(self):
		self.session.nav.playService(self.postScanService)
		self.close()

	def Satexists(self, tlist, pos):
		for x in tlist:
			if x == pos:
				return 1
		return 0

	def go(self):
		self.postScanService = self.session.nav.getCurrentlyPlayingServiceReference()
		try:
			if self.postScanService.toString().find(':11A0000:') == -1:
				ref = eServiceReference(initscanservice)
				self.session.nav.playService(ref)
				self.timer = eTimer()
				self.timer.callback.append(self.doScan)
				self.timer.start(5000, 1)
			else:
				self.doScan()
		except:
			print "[AutoBouquets] script will first initialize your system!"
			self.postScanService = eServiceReference(defaultservice)
			self.doScan()

	def doScan(self):
		com = ("cd %s; /bin/busybox ash ./autobouquets_e2.sh " % (path.dirname(modules[__name__].__file__)))
		com += config.autobouquets.area.getValue()
		com += "#" + config.autobouquets.bouquetlist.getValue()
		com += "#" + str(config.autobouquets.extra.getValue())
		com += "#" + config.autobouquets.sort.getValue()
		com += "#" + str(config.autobouquets.numbered.getValue())
		com += "#" + str(config.autobouquets.nitscan.getValue())
		com += "#" + str(config.autobouquets.placeholder.getValue())
		com += "#" + str(config.autobouquets.parental.getValue())
		com += "#" + str(config.autobouquets.default.getValue())
		com += "#" + str(config.autobouquets.ordering.getValue())
		com += "#" + config.autobouquets.freetoair.getValue()
		com += "#" + str(config.autobouquets.checkscript.getValue())
		com += "#" + config.autobouquets.style.getValue()
		com += "#" + config.autobouquets.piconlink.getValue()
		com += "#" + config.autobouquets.piconfolder.getValue()
		com += "#" + config.autobouquets.piconstyle.getValue()
		self.shcom(com)

	def shcom(self, com):
		if fileExists("%s/autobouquetsreader" % (path.dirname(modules[__name__].__file__))):
			self.session.openWithCallback(self.scancomplete,Console,_("AutoBouquets E2 for 28.2E"), ["%s" % com], closeOnSuccess=True)
		else:
			self.session.open(MessageBox,"autobouquetsreader not found!",MessageBox.TYPE_ERROR)

	def scancomplete(self):
		infobox = self.session.open(MessageBox,_("Reloading Bouquets and Services..."), MessageBox.TYPE_INFO, timeout=5)
		infobox.setTitle(_("AutoBouquets E2"))
		InfoBar.instance.servicelist.setModeTv()
		eDVBDB.getInstance().reloadBouquets()
		eDVBDB.getInstance().reloadServicelist()
		try:
			if self.session.nav.getCurrentlyPlayingServiceReference() != self.postScanService:
				self.session.nav.playService(self.postScanService)
		except:
			self.postScanService = eServiceReference(defaultservice)
			self.session.nav.playService(self.postScanService)
		if self.wasinstandby or scriptwasinstandby:
			from Tools import Notifications
			Notifications.AddNotification(Standby)
		else:
			self.timer = eTimer()
			self.timer.callback.append(self.close(None))
			self.timer.start(5000, 1)

	def help(self):
		self.session.open(Console,_("Showing AutoBouquets helpfile.txt"),[("cat %s/helpfile.txt" % (path.dirname(modules[__name__].__file__)))])

	def cancel(self):
		self.close(None)

class AutoBouquetsMenu(ConfigListScreen, Screen):
	skin = """
		<screen name="AutoBouquetsMenu" position="center,center" size="600,550" title="AutoBouquets Setup">
			<widget name="config" position="30,5" size="540,400" scrollbarMode="showOnDemand" />
			<ePixmap pixmap="skin_default/div-h.png" position="30,410" zPosition="1" size="540,2" />
			<widget name="description" render="Label" position="30,425" size="540,80" font="Regular;18" />
			<widget name="key_red" position="10,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="red" position="10,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/red.png" transparent="1" alphatest="on" />
			<widget name="key_green" position="157,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="green" position="157,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/green.png" transparent="1" alphatest="on" />
			<widget name="key_yellow" position="304,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="yellow" position="304,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/yellow.png" transparent="1" alphatest="on" />
			<widget name="key_blue" position="451,510" size="140,40" valign="center" halign="center" zPosition="4" foregroundColor="white" font="Regular;18" transparent="1"/>
			<ePixmap name="blue" position="451,510" zPosition="2" size="140,40" pixmap="skin_default/buttons/blue.png" transparent="1" alphatest="on" />
		</screen>"""

	def __init__(self, session):
		Screen.__init__(self, session)
		self.session = session
		Screen.setTitle(self, _("AutoBouquets Setup") + " - " + ab_version)

		self.onChangedEntry = [ ]
		self.list = []
		ConfigListScreen.__init__(self, self.list, session = self.session, on_change = self.changedEntry)
		self.createSetup()

		self["actions"] = ActionMap(["SetupActions", 'ColorActions', 'VirtualKeyboardActions', "MenuActions"],
		{
			"ok": self.keySave,
			"cancel": self.keyCancel,
			"red": self.keyCancel,
			"green": self.keySave,
		}, -2)

		self["key_red"] = Button(_("Cancel"))
		self["key_green"] = Button(_("OK"))
		self["key_yellow"] = Button("")
		self["key_blue"] = Button("")
		self["help"] = StaticText()
		self["description"] = Label(_(""))

		def selectionChanged():
			if self["config"].current:
				self["config"].current[1].onDeselect(self.session)
			self["config"].current = self["config"].getCurrent()
			if self["config"].current:
				self["config"].current[1].onSelect(self.session)
			for x in self["config"].onSelectionChanged:
				x()

		self["config"].selectionChanged = selectionChanged
		self["config"].onSelectionChanged.append(self.configHelp)

	def createSetup(self):
		self.editListEntry = None
		self.list = []
		self.list.append(getConfigListEntry(_("Local Area Selection"), config.autobouquets.area,
			_("Select your region/area")))
		self.list.append(getConfigListEntry(_("Numbering System"), config.autobouquets.bouquetlist,
			_("SD: Channel numbers like a Sky SD box\nHD: Channel numbers like a Sky HD box")))
		self.list.append(getConfigListEntry(_("Scan Mode"), config.autobouquets.freetoair,
			_("Available HD = Skips unviewable HD channels\nFTA Only = Free channels only\nAll Channels = All channels")))
		self.list.append(getConfigListEntry(_("Bouquet Channel Sort"), config.autobouquets.sort,
			_("HD First = First bouquet contains HD channels\nLarge 1st bouquet = 1st one has all channels\nSort 3 = For users to customise contents")))
		self.list.append(getConfigListEntry(_("Add Ch. Number to Name"), config.autobouquets.numbered,
			_("Adds the official number to the channel name")))
		self.list.append(getConfigListEntry(_("Hide Adult/Gaming"), config.autobouquets.parental,
			_("This hides the Gaming and Adult bouquets")))
		self.list.append(getConfigListEntry(_("Channel Swap"), config.autobouquets.ordering,
			_("This is primarily to swap SD channels for their HD equivalent\nRefer to custom_swap.txt for swaps\nThere is no error checking")))
		self.list.append(getConfigListEntry(_("Make All Default Bouquets"), config.autobouquets.default,
			_("You should disable this if you want to move or delete bouquets")))
		self.list.append(getConfigListEntry(_("Other Extra Channels"), config.autobouquets.extra,
			_("This allows for test/out of region channels. They will be created in the Other bouquet\nSome services are not written to lamedb")))
		self.list.append(getConfigListEntry(_("Use Internal NIT Scan"), config.autobouquets.nitscan,
			_("Disabling this offers option to perform a conventional scan\nBest to leave it as it is!")))
		self.list.append(getConfigListEntry(_("Hide Blank Placeholders"), config.autobouquets.placeholder,
			_("Hides the blanks used to make official numbers\nNot all images support this")))
		self.list.append(getConfigListEntry(_("Enable Script Checks"), config.autobouquets.checkscript,
			_("Some images do not allow the scripts being used\nOnly disable this if you experience problems")))
		self.list.append(getConfigListEntry(_("Bouquet Display Style"), config.autobouquets.style,
			_("Choose your Bouquet Display Style")))
		self.list.append(getConfigListEntry(_("Create AutoPicon Links"), config.autobouquets.piconstyle,
			_("Do you want AB 28.2 to maintain the links to picons?\nOrdinary = conventional picons")))
		if config.autobouquets.piconstyle.value != '0':
			self.list.append(getConfigListEntry(_("Picon Link Location"), config.autobouquets.piconlink,
				_("Specify location for picon links\nLinks must be on an ext file system")))
			self.list.append(getConfigListEntry(_("Picon Folder Location"), config.autobouquets.piconfolder,
				_("Where are  picons located?\nIf using SNPs, this must be same as Link location.")))
		self.list.append(getConfigListEntry(_("Automatic Updates"), config.autobouquets.schedule,
			_("Do you want AB 28.2 to run automatically?")))
		if config.autobouquets.schedule.value:
			self.list.append(getConfigListEntry(_("Time of Update to start"), config.autobouquets.scheduletime,
				_("Specify time you want for automated AB 28.2 updates")))
			self.list.append(getConfigListEntry(_("Repeat how often"), config.autobouquets.repeattype,
				_("How often do you want AB 28.2 to run?")))
		self["config"].list = self.list
		self["config"].setList(self.list)

	def configHelp(self):
		cur = self["config"].getCurrent()
		self["help"].text = cur[2]

	# for summary:
	def changedEntry(self):
		if self["config"].getCurrent()[0] == _("Automatic Updates") or self["config"].getCurrent()[0] == _("Create AutoPicon Links"):
			self.createSetup()
		for x in self.onChangedEntry:
			x()

	def getCurrentEntry(self):
		return self["config"].getCurrent()[0]

	def getCurrentValue(self):
		return str(self["config"].getCurrent()[1].getText())

	def getCurrentDescription(self):
		return self["config"].getCurrent() and len(self["config"].getCurrent()) > 2 and self["config"].getCurrent()[2] or ""

	def saveAll(self):
		for x in self["config"].list:
			x[1].save()
		configfile.save()

	# keySave and keyCancel are just provided in case you need them.
	# you have to call them by yourself.
	def keySave(self):
		self.saveAll()
		self.close()

	def cancelConfirm(self, result):
		if not result:
			return
		for x in self["config"].list:
			x[1].cancel()
		self.close()

	def keyCancel(self):
		if self["config"].isChanged():
			self.session.openWithCallback(self.cancelConfirm, MessageBox, _("Really close without saving settings?"))
		else:
			self.close()

class AutoBouquetsLogView(Screen):
	skin = """
		<screen name="AutoBouquetsLogView" position="center,center" size="700,550" title="Update Database Log" >
			<widget name="list" position="0,0" size="700,550" font="Regular;16" />
		</screen>"""

	def __init__(self, session):
		self.session = session
		Screen.__init__(self, session)
		Screen.setTitle(self, _("AutoBouquets Database Log"))
		filename = ("%s/autobouquets.log" % (path.dirname(modules[__name__].__file__)))
		if path.exists("%s/autobouquets.log" % (path.dirname(modules[__name__].__file__))):
			filedate = str(date.fromtimestamp(stat(filename).st_mtime))
			log = _('Last update') + ': ' + filedate + '\n\n'
			tmpfile = file(filename).read()
			contents = str(tmpfile)
			log = log + contents
		else:
			log = _('Last update') + ': '
		self["list"] = ScrollLabel(str(log))
		self["setupActions"] = ActionMap(["SetupActions", "ColorActions", "DirectionActions", "MenuActions"],
		{
			"cancel": self.cancel,
			"ok": self.cancel,
			"up": self["list"].pageUp,
			"down": self["list"].pageDown,
			"menu": self.closeRecursive,
			"1": self.cancel,
			"6": self.closeRecursive,
		}, -2)

	def cancel(self):
		self.close()

	def closeRecursive(self):
		self.close(True)

class AutoBouquetsAbout(Screen):
	skin = """
		<screen position="center,center" size="600,380" title="AutoBouquets E2 - About">
			<widget name="about" position="10,10" size="580,360" font="Regular;15" />
			<widget name="key_red" position="10,320" size="140,40" valign="center" halign="center" zPosition="5" transparent="1" foregroundColor="white" font="Regular;18"/>
			<ePixmap name="red" pixmap="skin_default/buttons/red.png" position="10,320" size="140,40" zPosition="4" transparent="1" alphatest="on"/>
			<widget name="oealogo" position="400,225" size="200,135"  zPosition="4" transparent="1" alphatest="blend" />
		</screen>"""

	def __init__(self, session):
		self.session = session
		Screen.__init__(self, session)
		Screen.setTitle(self, _("AutoBouquets E2") + " - " + _("About"))

		self["about"] = Label("")
		self["oealogo"] = Pixmap()

		self["actions"] = ActionMap(["SetupActions", "ColorActions", "MenuActions"],
		{
			"red": self.quit,
			"cancel": self.quit,
			"menu": self.quit,
			"1": self.quit,
		}, -2)

		self["key_red"] = Button(_("Close"))

		credit = " AutoBouquets E2 for 28.2E\n"
		credit += " Version date - " + ab_version + "\n\n"
		credit += " Main Application Developer: LraiZer\n"
		credit += " frontend script, backend c++ coding, GUI python coding\n"
		credit += " Development Forum and Sources: http://www.ukcvs.net\n\n"
		credit += " Thanks: AndyBlac, ViX Team - GUI python code enhancments\n\n"
		credit += " Sources Credits: OE-Alliance AutoBouquetsMaker dvbscanner\n"
		credit += " (used OE-Alliance dvbscanner library as a starting point)\n"
		credit += " Application credits:\n"
		credit += " - Sandro Cavazzoni aka Skaman (SifTeam developer)\n"
		credit += " - Andrew Blackburn aka AndyBlac (ViX Team developer)\n"
		credit += "  http://github.com/oe-alliance\n"
		credit += "  http://www.sifteam.eu\n"
		credit += "  http://www.world-of-satellite.com"
		self["about"].setText(credit)
		self.onFirstExecBegin.append(self.setImages)

	def setImages(self):
		self["oealogo"].instance.setPixmapFromFile("%s/oea-logo.png" % (path.dirname(modules[__name__].__file__)))

	def quit(self):
		self.close()

class AutoBouquetsDownloader(Screen):
	skin = """
		<screen position="40,30" size="260,70" title="AutoBouquets E2 for 28.2E" flags="wfNoBorder" >
			<widget name="action" halign="center" valign="center" position="10,10" size="240,20" font="Regular;18" backgroundColor="#11000000" transparent="1" />
			<widget name="status" halign="center" valign="center" position="10,35" size="240,25" font="Regular;18" backgroundColor="#11000000" transparent="1" />
		</screen>"""

	def __init__(self, session):
		Screen.__init__(self, session)
		self.session = session
		self["action"] = Label(_("AutoBouquets E2 28.2E"))
		self["status"] = Label(_("downloading bouquets"))
		self.goBackgroundScan()

	def goBackgroundScan(self):
		self.postScanService = self.session.nav.getCurrentlyPlayingServiceReference()
		try:
			if self.postScanService.toString().find(':11A0000:') == -1:
				ref = eServiceReference(initscanservice)
				self.session.nav.playService(ref)
				self.timer = eTimer()
				self.timer.callback.append(self.doBackgroundScan)
				self.timer.start(5000, 1)
			else:
				self.doBackgroundScan()
		except:
			print "[AutoBouquets] script will first initialize your system!"
			self.postScanService = eServiceReference(defaultservice)
			self.doBackgroundScan()

	def doBackgroundScan(self):
		com = ("cd %s; /bin/busybox ash ./autobouquets_e2.sh " % (path.dirname(modules[__name__].__file__)))
		com += config.autobouquets.area.getValue()
		com += "#" + config.autobouquets.bouquetlist.getValue()
		com += "#" + str(config.autobouquets.extra.getValue())
		com += "#" + config.autobouquets.sort.getValue()
		com += "#" + str(config.autobouquets.numbered.getValue())
		com += "#" + "True" #always use NIT scan in background scan mode!
		com += "#" + str(config.autobouquets.placeholder.getValue())
		com += "#" + str(config.autobouquets.parental.getValue())
		com += "#" + str(config.autobouquets.default.getValue())
		com += "#" + str(config.autobouquets.ordering.getValue())
		com += "#" + config.autobouquets.freetoair.getValue()
		com += "#" + str(config.autobouquets.checkscript.getValue())
		com += "#" + config.autobouquets.style.getValue()
		com += "#" + config.autobouquets.piconlink.getValue()
		com += "#" + config.autobouquets.piconfolder.getValue()
		com += "#" + config.autobouquets.piconstyle.getValue()
		self.shcomBackground(com)

	def shcomBackground(self, com):
		try:
			global container
			def appClosed(retval):
				global container
				print "[AutoBouquets Downloader] FINISHED: ", retval
				container = None
				self.scanBackgroundcomplete()
			container = eConsoleAppContainer()
			if container.execute(com):
				raise Exception, "Script Failed: " + com
			container.appClosed.append(appClosed)
		except Exception, e:
			print "[AutoBouquets Downloader] FAILED: ", e

	def scanBackgroundcomplete(self):
		InfoBar.instance.servicelist.setModeTv()
		eDVBDB.getInstance().reloadBouquets()
		eDVBDB.getInstance().reloadServicelist()
		try:
			if self.session.nav.getCurrentlyPlayingServiceReference() != self.postScanService:
				self.session.nav.playService(self.postScanService)
		except:
			self.postScanService = eServiceReference(defaultservice)
			self.session.nav.playService(self.postScanService)
		self.timer = eTimer()
		self.timer.callback.append(self.close(None))
		self.timer.start(5000, 1)

###########################################################################

def main(session, **kwargs):
	session.open(AutoBouquets)

def maindownloader(session, **kwargs):
	session.open(AutoBouquetsDownloader)

def mainscan(menuid, **kwargs):
	if menuid == "scan":
		return [(_("AutoBouquets (28.2E)"), main, _("28.2e stream bouquet downloader"), None)]
	else:
		return []

###########################################################################

def Plugins(**kwargs):
	plist = [PluginDescriptor(name=_("AutoBouquets E2"),description=_("28.2e stream bouquet downloader"),where = PluginDescriptor.WHERE_PLUGINMENU,icon="autobouquets.png", fnc=main)]
	plist.append(PluginDescriptor(name=_("AutoBouquets Downloader"),description=_("28.2e stream bouquet downloader"),where=PluginDescriptor.WHERE_EXTENSIONSMENU, fnc=maindownloader))
	plist.append(PluginDescriptor(where=PluginDescriptor.WHERE_SESSIONSTART, fnc=AutoBouquetsautostart))
	plist.append(PluginDescriptor(where=PluginDescriptor.WHERE_MENU, fnc=mainscan))
	return plist


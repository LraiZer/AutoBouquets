DESCRIPTION = "28.2E stream bouquet downloader"
SUMMARY = "scan dvb data for automatic bouquets creation on Enigma2 STB"
MAINTAINER = "LraiZer"
HOMEPAGE = "https://github.com/LraiZer/AutoBouquets"
SECTION = "extra"
PRIORITY = "optional"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "\
    file://LICENSE;md5=a23a74b3f4caf9616230789d94217acb \
    file://COPYING;md5=036b9f2d884ff3a35bed6ab09bafff32 \
"

inherit gitpkgv

AUTOBOUQUETS_BRANCH ?= "release"
SRCREV = "${AUTOREV}"
PV = "1.0+git${SRCPV}"
PKGV = "1.0+git${GITPKGV}"
PR = "r0"

SRC_URI="git://github.com/LraiZer/AutoBouquets.git;branch=${AUTOBOUQUETS_BRANCH}"

S = "${WORKDIR}/git"

FILES_${PN} = "/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets"

EXTRA_OECONF = ""

do_install() {
    install -d ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -d ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets/locale
    install -m 644 ${S}/autobouquets.png ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 755 ${S}/autobouquets_e2.sh ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 755 ${S}/autobouquetsreader ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 755 ${S}/autopicon_convert.sh ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/changelog.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/COPYING ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/custom_sort_1.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/custom_sort_2.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/custom_sort_3.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/custom_swap.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/helpfile.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/__init__.py ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/LICENSE ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/oea-logo.png ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/plugin.py ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/readme.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
    install -m 644 ${S}/supplement.txt ${D}/usr/lib/enigma2/python/Plugins/Extensions/AutoBouquets
}

do_package_prepend() {
    PREINST_path = "${S}/CONTROL/preinst"
    POSTINST_path = "${S}/CONTROL/postinst"
    POSTRM_path = "${S}/CONTROL/postrm"
    PREINST = open(PREINST_path, "r")
    POSTINST = open(POSTINST_path, "r")
    POSTRM = open(POSTRM_path, "r")
    d.setVar("pkg_preinst", PREINST.read())
    d.setVar("pkg_postinst", POSTINST.read())
    d.setVar("pkg_postrm", POSTRM.read())
}


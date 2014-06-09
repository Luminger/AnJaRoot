BUILDDIR := native
APKDIR := $(BUILDDIR)/updatezip-apk-install
INSTALLDIR := $(BUILDDIR)/updatezip-install
UNINSTALLDIR := $(BUILDDIR)/updatezip-uninstall

SCRIPTDIR := scripts
INSTALLSCRIPT := $(SCRIPTDIR)/update-binary.install.sh
UNINSTALLSCRIPT := $(SCRIPTDIR)/update-binary.uninstall.sh

MANIFEST := AndroidManifest.xml
VERSIONCODE := $(shell grep -Po "(?<=versionCode=\")([0-9]+)" $(MANIFEST))
VERSIONNAME := $(shell grep -Po "(?<=versionName=\")([^\"]*)" $(MANIFEST))

RELEASEDIR := ../../build/outputs/zip
APKRELEASE := $(RELEASEDIR)/AnJaRoot-$(VERSIONNAME)-apk-install-update.zip
INSTALLRELEASE := $(RELEASEDIR)/AnJaRoot-$(VERSIONNAME)-install-update.zip
UNINSTALLRELEASE := $(RELEASEDIR)/AnJaRoot-$(VERSIONNAME)-uninstall-update.zip
ASSETFILE := assets/AnJaRoot.zip
APKFILE := ../../build/outputs/apk/app-release.apk

INSTALLERFILES := anjarootinstaller anjarootd libanjaroot.so

include jni/Application.mk
FILES := $(foreach abi,$(APP_ABI), \
         $(foreach file,$(INSTALLERFILES), \
             obj/local/$(abi)/$(file)))

SOURCES := $(shell find jni -type f -print)

.PHONY: all clean apkrelease release

all: $(APKRELEASE) $(INSTALLRELEASE) $(UNINSTALLRELEASE)

apkrelease: $(APKRELEASE)

release: $(INSTALLRELEASE) $(UNINSTALLRELEASE)

clean:
	ndk-build clean
	rm -rf obj libs $(BUILDDIR) $(RELEASE) $(ASSETFILE) .*.stamp

.buildNative.stamp: $(SOURCES)
	ndk-build
	touch $@

$(APKRELEASE): .buildNative.stamp $(SCRIPTS)
	mkdir -p $(dir $(patsubst obj/local/%,$(APKDIR)/%,$(FILES)))
	$(foreach file, $(FILES), \
		cp $(file) $(APKDIR)/$(patsubst obj/local/%,%,$(file));)
	
	mkdir -p $(APKDIR)/META-INF/com/google/android/
	cp $(INSTALLSCRIPT) $(APKDIR)/META-INF/com/google/android/update-binary
	
	echo $(VERSIONNAME) > $(APKDIR)/version
	
	mkdir -p $(RELEASEDIR)
	rm -f $(APKRELEASE)
	cd $(APKDIR) && zip -r $(CURDIR)/$(APKRELEASE) .
	cp $(CURDIR)/$(APKRELEASE) $(ASSETFILE)

$(INSTALLRELEASE): .buildNative.stamp $(SCRIPTS)
	mkdir -p $(dir $(patsubst obj/local/%,$(INSTALLDIR)/%,$(FILES)))
	$(foreach file, $(FILES), \
		cp $(file) $(INSTALLDIR)/$(patsubst obj/local/%,%,$(file));)
	
	mkdir -p $(INSTALLDIR)/META-INF/com/google/android/
	cp $(INSTALLSCRIPT) $(INSTALLDIR)/META-INF/com/google/android/update-binary
	cp $(APKFILE) $(INSTALLDIR)/AnJaRoot.apk
	
	echo $(VERSIONNAME) > $(INSTALLDIR)/version
	
	mkdir -p $(RELEASEDIR)
	rm -f $(INSTALLRELEASE)
	cd $(INSTALLDIR) && zip -r $(CURDIR)/$(INSTALLRELEASE) .

$(UNINSTALLRELEASE): .buildNative.stamp $(SCRIPTS)
	mkdir -p $(UNINSTALLDIR)/META-INF/com/google/android/
	cp $(UNINSTALLSCRIPT) $(UNINSTALLDIR)/META-INF/com/google/android/update-binary
	
	echo $(VERSIONNAME) > $(UNINSTALLDIR)/version
	
	mkdir -p $(RELEASEDIR)
	rm -f $(UNINSTALLRELEASE)
	cd $(UNINSTALLDIR) && zip -r $(CURDIR)/$(UNINSTALLRELEASE) .

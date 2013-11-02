#!/sbin/sh

# $0: our name (obviously...)
# $1: recovery version (applies restrictions to $2 usage)
# $2: communication fd (see comment below)
# $3: path to the update.zip

# Documentation from the calling install.c:
# // When executing the update binary contained in the package, the
# // arguments passed are:
# //
# //   - the version number for this interface
# //
# //   - an fd to which the program can write in order to update the
# //     progress bar.  The program can write single-line commands:
# //
# //        progress <frac> <secs>
# //            fill up the next <frac> part of of the progress bar
# //            over <secs> seconds.  If <secs> is zero, use
# //            set_progress commands to manually control the
# //            progress of this segment of the bar
# //
# //        set_progress <frac>
# //            <frac> should be between 0.0 and 1.0; sets the
# //            progress bar within the segment defined by the most
# //            recent progress command.
# //
# //        firmware <"hboot"|"radio"> <filename>
# //            arrange to install the contents of <filename> in the
# //            given partition on reboot.
# //
# //            (API v2: <filename> may start with "PACKAGE:" to
# //            indicate taking a file from the OTA package.)
# //
# //            (API v3: this command no longer exists.)
# //
# //        ui_print <string>
# //            display <string> on the screen.
# //
# //   - the name of the package zip file.
# //

# GLOBAL VARIABLES
SELF=$0
RECOVERY_VERSION=$1
COMMUNICATION_FD=$2
UPDATEZIP_PATH=$3

CPU_ABI=''
TMPDIR=/tmp/AnJaRoot
APK_TARGET=/system/app/AnJaRoot.apk
APK_ZIP=$TMPDIR/AnJaRoot.apk
APK_SYS='/data/app/org.failedprojects.anjaroot-*.apk'

OLD_INSTALLER="/system/bin/anjarootinstaller"

ANJAROOT_STDOUT_PATH="/cache/AnJaRoot.stdout.log"
ANJAROOT_STDERR_PATH="/cache/AnJaRoot.stderr.log"
ANJAROOT_DEBUG_PATH="/cache/AnJaRoot.debug.log"
export ANJAROOT_LOG_PATH="/cache/AnJaRoot.installer.log"

# redirect stdout and stderr to logfiles
exec 1> $ANJAROOT_STDOUT_PATH
exec 2> $ANJAROOT_STDERR_PATH

# open debug log
exec 64> $ANJAROOT_DEBUG_PATH

# UTILITY FUNCTIONS
printnl(){
    echo -e "ui_print $1\n" >&$COMMUNICATION_FD
    echo -e 'ui_print\n' >&$COMMUNICATION_FD
}

debug(){
    echo $1 >&64
}

cleanup(){
    printnl 'Cleanup...'
    rm -r $TMPDIR
}

abort(){
    debug "Failed command returned: $?"
    cleanup
    printnl '************************'
    printnl ' ERROR: Install failed!'
    printnl '************************'
    exit 1
}

# From: http://stackoverflow.com/a/8811800/180242
# contains(string, substring)
#
# Returns 0 if the specified string contains the specified substring,
# otherwise returns 1.
contains() {
    string="$1"
    substring="$2"
    if test "${string#*$substring}" != "$string"
    then
        return 0    # $substring is in $string
    else
        return 1    # $substring is not in $string
    fi
}

discoverArch(){
    # copied from libc/include/sys/_system_properties.h (bionic repo), stable
    # since Gingerbread release.
    FILES_TO_SEARCH="/default.prop
                     /system/build.prop
                     /system/default.prop
                     /data/local.prop
                     /factory/factory.prop"

    for FILE in $FILES_TO_SEARCH
    do
        while read LINE
        do
            contains $LINE "=mips" && CPU_ABI="mips"
            contains $LINE "=armeabi" && CPU_ABI="armeabi"
            contains $LINE "=x86" && CPU_ABI="x86"
        done < $FILE
    done

    if [ -z "$CPU_ABI" ]
    then
        printnl 'No valid ABI found!'
        abort
    fi
}

# INSTALLER
debug "SELF: $SELF"
debug "RECOVERY_VERSION: $RECOVERY_VERSION"
debug "UPDATEZIP_PATH: $UPDATEZIP_PATH"
debug "TMPDIR: $TMPDIR"
debug "APK_TARGET: $APK_TARGET"
debug "APK_ZIP: $APK_ZIP"
debug "APK_SYS: $APK_SYS"

debug "OLD_INSTALLER: $OLD_INSTALLER"

printnl 'Getting device ABI...'
discoverArch

INSTALLER="$TMPDIR/$CPU_ABI/anjarootinstaller"
DAEMON="$TMPDIR/$CPU_ABI/anjarootd"
LIBRARY="$TMPDIR/$CPU_ABI/libanjaroot.so"
debug "INSTALLER: $INSTALLER"
debug "DAEMON: $DAEMON"
debug "LIBRARY: $LIBRARY"

if [ -d "$TMPDIR" ]
then
    printnl "Remoing leftover tmp directory..."
    rm -r $TMPDIR || abort
fi

printnl 'Mounting /system...'
mount /system

printnl 'Unpacking data...'
mkdir $TMPDIR || abort
unzip -o $UPDATEZIP_PATH -d $TMPDIR || abort

if [ -f "$OLD_INSTALLER" ]
then
    printnl 'Preparing old installer...'
    chmod 755 $OLD_INSTALLER || abort

    printnl 'Cleaning away previous installation...'
    "$OLD_INSTALLER" --uninstall || abort
fi

if [ -f "$APK_ZIP" ]
then
    APK=$APK_ZIP
else
    printnl 'Mounting /data...'
    mount /data

    APK_SYS_FILE=$(ls $APK_SYS 2> /dev/null )
    debug "APK_SYS_FILE: $APK_SYS_FILE"
    if [ ! -z "$APK_SYS_FILE" ]
    then
        APK=$APK_SYS_FILE
    else
        printnl '*************************************'
        printnl ' ERROR: Unable to find AnJaRoot.apk.'
        printnl '*************************************'
        abort
    fi
fi

debug "APK: $APK"

printnl 'Preparing installer...'
chmod 755 $INSTALLER || abort

printnl 'Running installer...'
"$INSTALLER" --install --srclibpath="$LIBRARY" --daemonpath="$DAEMON" \
    --apkpath="$APK" || abort

cleanup

printnl 'Done!'

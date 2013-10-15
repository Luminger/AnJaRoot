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
DEBUG=0

SELF=$0
RECOVERY_VERSION=$1
COMMUNICATION_FD=$2
UPDATEZIP_PATH=$3

OLD_INSTALLER="/system/bin/anjarootinstaller"
export ANJAROOT_LOG_PATH="/cache/AnJaRoot.log"

# UTILITY FUNCTIONS
printnl(){
    echo -e "ui_print $1\n" > /proc/self/fd/$COMMUNICATION_FD
    echo -e 'ui_print\n' > /proc/self/fd/$COMMUNICATION_FD
}

debug(){
    if [ "$DEBUG" -eq '1' ]
    then
        printnl "DEBUG: $1"
    fi
}

abort(){
    debug "Failed command returned: $?"
    cleanup
    printnl '**************************'
    printnl ' ERROR: Uninstall failed!'
    printnl '**************************'
    exit 1
}

# UNINSTALLER
debug "SELF: $SELF"
debug "RECOVERY_VERSION: $RECOVERY_VERSION"
debug "UPDATEZIP_PATH: $UPDATEZIP_PATH"
debug "OLD_INSTALLER: $OLD_INSTALLER"

printnl 'Mounting /system...'
mount /system

printnl 'Preparing installer...'
chmod 755 $OLD_INSTALLER || abort

printnl 'Cleaning away previous installation...'
"$OLD_INSTALLER" --uninstall || abort

printnl 'Done!'

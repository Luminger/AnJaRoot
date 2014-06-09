#!/system/bin/sh

mountSystemReadWrite() {
    if [ -z "$MOUNTSYSTEMRW" ]
    then
        mount -orw,remount /system
        if [ $? -ne 0 ]
        then
            exit 1
        fi
    fi
}

mountSystemReadOnly() {
    if [ -z "$MOUNTSYSTEMRW" ]
    then
        mount -orw,remount /system
        if [ $? -ne 0 ]
        then
            exit 2
        fi
    fi
}

mountSystemReadWrite

%COMMAND%
if [ $? -ne 0 ]
then
    mountSystemReadOnly
    exit 3
fi

mountSystemReadOnly
exit 0

#!/bin/sh
#
# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# This wrapper script is used to launch a native debugging session
# on a given NDK application. The application must be debuggable, i.e.
# its android:debuggable attribute must be set to 'true' in the
# <application> element of its manifest.
#
# See docs/NDK-GDB.TXT for usage description. Essentially, you just
# need to launch ndk-gdb from your application project directory
# after doing ndk-build && ant install && <start-application-on-device>
#
. `dirname $0`/build/core/ndk-common.sh

force_32bit_binaries

find_program ADB_CMD adb
ADB_FLAGS=

AWK_CMD=awk

DEBUG_PORT=5039

PARAMETERS=
OPTION_HELP=no
OPTION_PROJECT=
OPTION_FORCE=no
OPTION_ADB=
OPTION_EXEC=
OPTION_START=no
OPTION_LAUNCH=
OPTION_LAUNCH_LIST=no

check_parameter ()
{
    if [ -z "$2" ]; then
        echo "ERROR: Missing parameter after option '$1'"
        exit 1
    fi
}

check_adb_flags ()
{
    if [ -n "$ADB_FLAGS" ] ; then
        echo "ERROR: Only one of -e, -d or -s <serial> can be used at the same time!"
        exit 1
    fi
}

get_build_var ()
{
    if [ -z "$GNUMAKE" ] ; then
        GNUMAKE=make
    fi
    $GNUMAKE --no-print-dir -f $ANDROID_NDK_ROOT/build/core/build-local.mk -C $PROJECT DUMP_$1
}

get_build_var_for_abi ()
{
    if [ -z "$GNUMAKE" ] ; then
        GNUMAKE=make
    fi
    $GNUMAKE --no-print-dir -f $ANDROID_NDK_ROOT/build/core/build-local.mk -C $PROJECT DUMP_$1 APP_ABI=$2
}

# Used to run an awk script on the manifest
run_awk_manifest_script ()
{
    $AWK_CMD -f $AWK_SCRIPTS/$1 $PROJECT/$MANIFEST
}

VERBOSE=no
while [ -n "$1" ]; do
    opt="$1"
    optarg=`expr "x$opt" : 'x[^=]*=\(.*\)'`
    case "$opt" in
        --help|-h|-\?)
            OPTION_HELP=yes
            ;;
        --verbose)
            VERBOSE=yes
            ;;
        -s)
            check_parameter $1 $2
            check_adb_flags
            ADB_FLAGS=" -s $2"
            shift
            ;;
        -s*)
            check_adb_flags
            optarg=`expr -- "$opt" : '-s\(.*\)'`
            ADB_FLAGS=" -s $optarg"
            ;;
        -p)
            check_parameter $1 $2
            OPTION_PROJECT="$2"
            shift
            ;;
        -p*)
            optarg=`expr -- "$opt" : '-p\(.*\)'`
            OPTION_PROJECT="$optarg"
            ;;
        --exec=*)
            OPTION_EXEC="$optarg"
            ;;
        -x)
            check_parameter $1 $2
            OPTION_EXEC="$2"
            shift
            ;;
        -x*)
            optarg=`expr -- "$opt" : '-x\(.*\)'`
            OPTION_EXEC="$optarg"
            ;;
        -e)
            check_adb_flags
            ADB_FLAGS=" -e"
            ;;
        -d)
            check_adb_flags
            ADB_FLAGS=" -d"
            ;;
        --adb=*) # specify ADB command
            OPTION_ADB="$optarg"
            ;;
        --awk=*)
            AWK_CMD="$optarg"
            ;;
        --project=*)
            OPTION_PROJECT="$optarg"
            ;;
        --port=*)
            DEBUG_PORT="$optarg"
            ;;
        --force)
            OPTION_FORCE="yes"
            ;;
        --launch-list)
            OPTION_LAUNCH_LIST="yes"
            ;;
        --launch=*)
            OPTION_LAUNCH="$optarg"
            ;;
        --start)
            OPTION_START=yes
            ;;
        -*) # unknown options
            echo "ERROR: Unknown option '$opt', use --help for list of valid ones."
            exit 1
        ;;
        *)  # Simply record parameter
            if [ -z "$PARAMETERS" ] ; then
                PARAMETERS="$opt"
            else
                PARAMETERS="$PARAMETERS $opt"
            fi
            ;;
    esac
    shift
done

if [ "$OPTION_HELP" = "yes" ] ; then
    echo "Usage: $PROGNAME [options]"
    echo ""
    echo "Setup a gdb debugging session for your Android NDK application."
    echo "Read $$NDK/docs/NDK-GDB.TXT for complete usage instructions."
    echo ""
    echo "Valid options:"
    echo ""
    echo "    --help|-h|-?      Print this help"
    echo "    --verbose         Enable verbose mode"
    echo "    --force           Kill existing debug session if it exists"
    echo "    --start           Launch application instead of attaching to existing one"
    echo "    --launch=<name>   Same as --start, but specify activity name (see below)"
    echo "    --launch-list     List all launchable activity names from manifest"
    echo "    --project=<path>  Specify application project path"
    echo "    -p <path>         Same as --project=<path>"
    echo "    --port=<port>     Use tcp:localhost:<port> to communicate with gdbserver [$DEBUG_PORT]"
    echo "    --exec=<file>     Execute gdb initialization commands in <file> after connection"
    echo "    -x <file>         Same as --exec=<file>"
    echo "    --adb=<file>      Use specific adb command [$ADB_CMD]"
    echo "    --awk=<file>      Use specific awk command [$AWK_CMD]"
    echo "    -e                Connect to single emulator instance"
    echo "    -d                Connect to single target device"
    echo "    -s <serial>       Connect to specific emulator or device"
    echo ""
    exit 0
fi

log "Android NDK installation path: $ANDROID_NDK_ROOT"

if [ -n "$OPTION_EXEC" ] ; then
    if [ ! -f "$OPTION_EXEC" ]; then
        echo "ERROR: Invalid initialization file: $OPTION_EXEC"
        exit 1
    fi
fi

# Check ADB tool version
if [ -n "$OPTION_ADB" ] ; then
    ADB_CMD="$OPTION_ADB"
    log "Using specific adb command: $ADB_CMD"
else
    if [ -z "$ADB_CMD" ] ; then
        echo "ERROR: The 'adb' tool is not in your path."
        echo "       You can change your PATH variable, or use"
        echo "       --adb=<executable> to point to a valid one."
        exit 1
    fi
    log "Using default adb command: $ADB_CMD"
fi

ADB_VERSION=`$ADB_CMD version`
if [ $? != 0 ] ; then
    echo "ERROR: Could not run ADB with: $ADB_CMD"
    exit 1
fi
log "ADB version found: $ADB_VERSION"

ADB_CMD="${ADB_CMD}${ADB_FLAGS}"
log "Using final ADB command: '$ADB_CMD'"

adb_shell ()
{
    # Run an adb shell command and return its output.
    #
    # We need to filter weird control characters like \r that are
    # included in the output.
    #
    $ADB_CMD shell $@ | sed -e 's![[:cntrl:]]!!g'
}

# Check the awk tool
AWK_SCRIPTS=$ANDROID_NDK_ROOT/build/awk
AWK_TEST=`$AWK_CMD -f $AWK_SCRIPTS/check-awk.awk`
if [ $? != 0 ] ; then
    echo "ERROR: Could not run '$AWK_CMD' command. Do you have it installed properly?"
    exit 1
fi
if [ "$AWK_TEST" != "Pass" ] ; then
    echo "ERROR: Your version of 'awk' is obsolete. Please use --awk=<file> to point to Nawk or Gawk!"
    exit 1
fi

# Name of the manifest file
MANIFEST=AndroidManifest.xml

# Find the root of the application project.
if [ -n "$OPTION_PROJECT" ] ; then
    PROJECT=$OPTION_PROJECT
    log "Using specified project path: $PROJECT"
    if [ ! -d "$PROJECT" ] ; then
        echo "ERROR: Your --project option does not point to a directory!"
        exit 1
    fi
    if [ ! -f "$PROJECT/$MANIFEST" ] ; then
        echo "ERROR: Your --project does not point to an Android project path!"
        echo "       It is missing a $MANIFEST file."
        exit 1
    fi
else
    # Assume we are in the project directory
    if [ -f "$MANIFEST" ] ; then
        PROJECT=.
    else
        PROJECT=
        CURDIR=`pwd`
        while [ "$CURDIR" != "/" ] ; do
            if [ -f "$CURDIR/$MANIFEST" ] ; then
                PROJECT="$CURDIR"
                break
            fi
            CURDIR=`dirname $CURDIR`
        done
        if [ -z "$PROJECT" ] ; then
            echo "ERROR: Launch this script from an application project directory, or use --project=<path>."
            exit 1
        fi
    fi
    log "Using auto-detected project path: $PROJECT"
fi

# Extract the package name from the manifest
PACKAGE_NAME=`run_awk_manifest_script extract-package-name.awk`
log "Found package name: $PACKAGE_NAME"
if [ $? != 0 -o "$PACKAGE_NAME" = "<none>" ] ; then
    echo "ERROR: Could not extract package name from $PROJECT/$MANIFEST."
    echo "       Please check that the file is well-formed!"
    exit 1
fi

# If --launch-list is used, list all launchable activities, and be done with it
if [ "$OPTION_LAUNCH_LIST" = "yes" ] ; then
    log "Extracting list of launchable activities from manifest:"
    run_awk_manifest_script extract-launchable.awk
    exit 0
fi

# Check that the application is debuggable, or nothing will work
DEBUGGABLE=`run_awk_manifest_script extract-debuggable.awk`
log "Found debuggable flag: $DEBUGGABLE"
if [ $? != 0 -o "$DEBUGGABLE" != "true" ] ; then
    echo "ERROR: Package $PACKAGE_NAME is not debuggable ! Please fix your manifest,"
    echo "       rebuild your application and re-install it to fix this."
    exit 1
fi

APP_ABIS=`get_build_var APP_ABI`
log "ABIs targetted by application: $APP_ABIS"

# Check the ADB command, and that we can connect to the device/emulator
ADB_TEST=`$ADB_CMD shell ls`
if [ $? != 0 ] ; then
    echo "ERROR: Could not connect to device or emulator!"
    echo "       Please check that an emulator is running or a device is connected"
    echo "       through USB to this machine. You can use -e, -d and -s <serial>"
    echo "       in case of multiple ones."
    exit 1
fi

# Check that the device is running Froyo (API Level 8) or higher
#
API_LEVEL=`adb_shell getprop ro.build.version.sdk`
if [ $? != 0 -o -z "$API_LEVEL" ] ; then
    echo "ERROR: Could not find target device's supported API level !"
    echo "ndk-gdb will only work if your device is running Android 2.2 or higher."
    exit 1
fi
log "Device API Level: $API_LEVEL"
if [ "$API_LEVEL" -lt "8" ] ; then
    echo "ERROR: ndk-gdb requires a target device running Android 2.2 (API level 8) or higher."
    echo "The target device is running API level $API_LEVEL !"
    exit 1
fi

# Get the target device's supported ABI(s)
# And check that they are supported by the application
#
COMPAT_ABI=none
CPU_ABI=`adb_shell getprop ro.product.cpu.abi`
echo "$APP_ABIS" | grep -q -F "$CPU_ABI"
if [ $? = 0 ] ; then
    COMPAT_ABI=$CPU_ABI
fi

CPU_ABI2=`adb_shell getprop ro.product.cpu.abi2`
if [ -z "$CPU_ABI2" ] ; then
    log "Device CPU ABI: $CPU_ABI"
else
    log "Device CPU ABIs: $CPU_ABI $CPU_ABI2"
    echo "$APP_ABIS" | grep -q -F "$CPU_ABI2"
    if [ $? = 0 ] ; then
        COMPAT_ABI=$CPU_ABI2
    fi
fi
if [ "$COMPAT_ABI" = none ] ; then
    echo "ERROR: The device does not support the application's targetted CPU ABIs!"
    if [ "$CPU_ABI2" = "$CPU_ABI" ] ; then
        CPU_ABI2=
    fi
    echo "       Device supports:  $CPU_ABI $CPU_ABI2"
    echo "       Package supports: $APP_ABIS"
    exit 1
fi
log "Compatible device ABI: $COMPAT_ABI"

# Let's check that the user didn't change the debuggable flag in
# the manifest without calling ndk-build afterwards.
if [ ! -f $PROJECT/libs/$COMPAT_ABI/gdbserver ] ; then
    echo "ERROR: Could not find gdbserver binary under $PROJECT/libs/$COMPAT_ABI"
    echo "       This usually means you modified your AndroidManifest.xml to set"
    echo "       the android:debuggable flag to 'true' but did not rebuild the"
    echo "       native binaries. Please call 'ndk-build' to do so,"
    echo "       *then* re-install to the device !"
    exit 1
fi

# Let's check that 'gdbserver' is properly installed on the device too. If this
# is not the case, the user didn't install the proper package after rebuilding.
#
DEVICE_GDBSERVER=`adb_shell ls /data/data/$PACKAGE_NAME/lib/gdbserver`
log "Found device gdbserver: $DEVICE_GDBSERVER"
if pattern_match "No such file or directory" "$DEVICE_GDBSERVER" ] ; then
    echo "ERROR: Non-debuggable application installed on the target device."
    echo "       Please re-install the debuggable version !"
    exit 1
fi

# Get information from the build system
GDBSETUP_INIT=`get_build_var_for_abi NDK_APP_GDBSETUP $COMPAT_ABI`
log "Using gdb setup init: $GDBSETUP_INIT"

TOOLCHAIN_PREFIX=`get_build_var_for_abi TOOLCHAIN_PREFIX $COMPAT_ABI`
log "Using toolchain prefix: $TOOLCHAIN_PREFIX"

APP_OUT=`get_build_var_for_abi TARGET_OUT $COMPAT_ABI`
log "Using app out directory: $APP_OUT"

# Find the <dataDir> of the package on the device
DATA_DIR=`adb_shell run-as $PACKAGE_NAME /system/bin/sh -c pwd`
log "Found data directory: '$DATA_DIR'"
if [ $? != 0 -o -z "$DATA_DIR" ] ; then
    echo "ERROR: Could not extract package's data directory. Are you sure that"
    echo "       your installed application is debuggable?"
    exit 1
fi

# Launch the activity if needed
if [ -n "$OPTION_START" ] ; then
    # If --launch is used, ignore --start, otherwise extract the first
    # launchable activity name from the manifest and use it as if --launch=<name>
    # was used instead.
    #
    if [ -z "$OPTION_LAUNCH" ] ; then
        OPTION_LAUNCH=`run_awk_manifest_script extract-launchable.awk | sed 2q`
        if [ $? != 0 ] ; then
            echo "ERROR: Could not extract name of launchable activity from manifest!"
            echo "       Try to use --launch=<name> directly instead as a work-around."
            exit 1
        fi
        log "Found first launchable activity: $OPTION_LAUNCH"
        if [ -z "$OPTION_LAUNCH" ] ; then
            echo "ERROR: It seems that your Application does not have any launchable activity!"
            echo "       Please fix your manifest file and rebuild/re-install your application."
            exit 1
        fi
    fi
fi

if [ -n "$OPTION_LAUNCH" ] ; then
    log "Launching activity: $PACKAGE_NAME/$OPTION_LAUNCH"
    run $ADB_CMD shell am start -n $PACKAGE_NAME/$OPTION_LAUNCH
    if [ $? != 0 ] ; then
        echo "ERROR: Could not launch specified activity: $OPTION_LAUNCH"
        echo "       Use --launch-list to dump a list of valid values."
        exit 1
    fi
    # Sleep a bit, it sometimes take one second to start properly
    # Note that we use the 'sleep' command on the device here.
    run $ADB_CMD shell sleep 1
fi

# Find the PID of the application being run
PID=`$ADB_CMD shell ps | $AWK_CMD -f $AWK_SCRIPTS/extract-pid.awk -v PACKAGE=$PACKAGE_NAME`
log "Found running PID: $PID"
if [ $? != 0 -o "$PID" = "0" ] ; then
    echo "ERROR: Could not extract PID of application on device/emulator."
    if [ -n "$OPTION_LAUNCH" ] ; then
        echo "       Weird, this probably means one of these:"
        echo ""
        echo "         - The installed package does not match your current manifest."
        echo "         - The application process was terminated."
        echo ""
        echo "       Try using the --verbose option and look at its output for details."
    else
        echo "       Are you sure the application is already started?"
        echo "       Consider using --start or --launch=<name> if not."
    fi
    exit 1
fi

# Check that there is no other instance of gdbserver running
GDBSERVER_PS=`$ADB_CMD shell ps | grep lib/gdbserver`
if [ -n "$GDBSERVER_PS" ] ; then
    if [ "$OPTION_FORCE" = "no" ] ; then
        echo "ERROR: Another debug session running, Use --force to kill it."
        exit 1
    fi
    log "Killing existing debugging session"
    GDBSERVER_PID=`echo $GDBSERVER_PS | $AWK_CMD -f $AWK_SCRIPTS/extract-pid.awk -v PACKAGE=lib/gdbserver`
    if [ $GDBSERVER_PID != 0 ] ; then
        run $ADB_CMD shell kill -9 $GDBSERVER_PID
    fi
fi

# Launch gdbserver now
DEBUG_SOCKET=debug-socket
run $ADB_CMD shell run-as $PACKAGE_NAME lib/gdbserver +$DEBUG_SOCKET --attach $PID &
if [ $? != 0 ] ; then
    echo "ERROR: Could not launch gdbserver on the device?"
    exit 1
fi
log "Launched gdbserver succesfully."

# Setup network redirection
log "Setup network redirection"
run $ADB_CMD forward tcp:$DEBUG_PORT localfilesystem:$DATA_DIR/$DEBUG_SOCKET
if [ $? != 0 ] ; then
    echo "ERROR: Could not setup network redirection to gdbserver?"
    echo "       Maybe using --port=<port> to use a different TCP port might help?"
    exit 1
fi

# Get the app_server binary from the device
APP_PROCESS=$APP_OUT/app_process
run adb pull /system/bin/app_process $APP_PROCESS
log "Pulled $APP_BINARY from device/emulator."

echo "Attached; pid = $PID"

# Now launch the appropriate gdb client with the right init commands
#
GDBCLIENT=${TOOLCHAIN_PREFIX}gdb
GDBSETUP=$APP_OUT/gdb.setup
cp -f $GDBSETUP_INIT $GDBSETUP
# echo "target remote :$DEBUG_PORT" >> $GDBSETUP
if [ -n "$OPTION_EXEC" ] ; then
    cat $OPTION_EXEC >> $GDBSETUP
fi

echo "C/C++ Application: ${APP_PROCESS}"
echo "GDB debugger: ${GDBCLIENT}"
echo "GDB command file: ${GDBSETUP}"
echo "Connection: TCP at localhost:${DEBUG_PORT}"


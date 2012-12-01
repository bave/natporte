#!/bin/sh

# start option
LAN_IF=ix1
WNA_IF=ix0
FILTER_NUM=150
DIVERT_PORT_IO=8668

startDivert() {
    RETVAL=0
    if [ 'FreeBSD' = `uname -s` ]; then
        sysctl -w kern.kq_calloutmax=409600 > /dev/null
        RETVAL=${?}
    fi
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    sysctl -w net.inet.ip.forwarding=1 > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi
    sysctl -w net.inet.ip.fastforwarding=1 > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    ipfw add ${FILTER_NUM} allow ip from any to any via lo0 > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    INC_NUM=`expr ${FILTER_NUM} + 10` > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    ipfw add ${INC_NUM} divert ${DIVERT_PORT_IO} ip from any to any > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    echo "done system setting"
    return 0
}

stopDivert() {
    RETVAL=0
    if [ 'FreeBSD' = `uname -s` ]; then
        sysctl -w kern.kq_calloutmax=4096 > /dev/null
        RETVAL=${?}
    fi
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi
    sysctl -w net.inet.ip.forwarding=0 > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    sysctl -w net.inet.ip.fastforwarding=0 > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    ipfw delete ${FILTER_NUM} > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    INC_NUM=`expr ${FILTER_NUM} + 10` > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    ipfw delete ${INC_NUM} > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    echo "done system unsetting"
    return 0
}

listDivert() {
    ipfw list | grep divert
}

startService() {
    RETVAL=0
    startDivert
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi
    forever start pool_server > /dev/null > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi
    sleep 1 > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi
    ./natporte -d ${DIVERT_PORT_IO} -l ${LAN_IF} -w ${WNA_IF}
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    echo "NAT service started"
    return 0
}

stopService() {
    forever stop pool_server > /dev/null
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    stopDivert
    RETVAL=${?}
    if [ ${RETVAL} != 0 ]; then
        echo "command fail"
        return 1
    fi

    echo "NAT service stopped"
    return 0
}

restartService() {
    stopService
    startService
}

Usage ()
{
    echo "${0}:"
    echo "nat scripts/programs require root(0) UID."
    echo " 	${0} start"
    echo " 	${0} stop"
    echo " 	${0} restart"
    echo " 	${0} divon"
    echo " 	${0} divoff"
    exit 1
}

RunService ()
{
    ID=`id -u`
    if [ ${ID} != 0 ]
    then
        Usage
    fi
    case $1 in 
      divon    ) startDivert    ;;
      divoff   ) stopDivert     ;;
      divlist  ) listDivert     ;;
      start    ) startService   ;;
      stop     ) stopService    ;;
      restart  ) restartService ;;
      *        ) Usage ;;
    esac

    return 0
}

RunService ${1}

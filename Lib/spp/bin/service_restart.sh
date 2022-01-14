#!/bin/bash

LOCAL_IP=$(ifconfig eth1 | grep -w inet | sed 's/^.*addr://g' | sed 's/\s\+Bcast.*$//g')
SERVICE=$(find spp_*_worker | sed 's/spp_//g' |sed 's/_worker$//g')
PACKAGE=$(pwd | xargs dirname | xargs basename)
SET_ID=$(cat /tmp/.nest_set_file |grep SET_ID | awk '{print $2}')
NEST_TOOL=../bin/nest_tool

TMPFILE=./tmp$$

echo "Start restart worker/proxy on "$LOCAL_IP", service "$SERVICE "..."   

$NEST_TOOL 7 $LOCAL_IP $SET_ID $SERVICE $PACKAGE > $TMPFILE 2>&1
RSP_OK=$(cat $TMPFILE |grep success | grep -v grep| wc -l)
if [ $RSP_OK -gt 0 ]; then
	echo "Restart OK! "$(cat $TMPFILE |grep success | sed 's/.*://g')" processes restarted. "
else
	echo "Restart failed!!"
fi

rm -rf $TMPFILE

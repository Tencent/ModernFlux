#!/bin/sh
#You Can set the program name and config file name below:
spp_ctrl_progname='spp_ctrl'
spp_proxy_progname='spp_proxy'
spp_worker_progname='spp_worker'

spp_ctrl_confname='../etc/spp_ctrl.xml'
spp_proxy_confname='../etc/spp_proxy.xml'
spp_worker_confname='../etc/spp_worker1.xml'


RED=\\e[1m\\e[31m
DARKRED=\\e[31m
GREEN=\\e[1m\\e[32m
DARKGREEN=\\e[32m
BLUE=\\e[1m\\e[34m
DARKBLUE=\\e[34m
YELLOW=\\e[1m\\e[33m
DARKYELLOW=\\e[33m
MAGENTA=\\e[1m\\e[35m
DARKMAGENTA=\\e[35m
CYAN=\\e[1m\\e[36m
DARKCYAN=\\e[36m
RESET=\\e[m

if [ $# != 1 ]
then
	echo -e "$RED USAGE: $0 $YELLOW option [start | stop | force_stop | reload]$RESET"
	exit 0;
fi

if [ $1 = "start" ]
then
	pidnum=`ps -ef|grep "./$spp_ctrl_progname $spp_ctrl_confname"|grep -v grep|wc -l`
	if [ $pidnum -lt 1 ]
	then
		./$spp_ctrl_progname $spp_ctrl_confname
	else
		for pid in `ps -ef|grep "./$spp_ctrl_progname $spp_ctrl_confname"|grep -v grep|awk '{print $2}'`
		do
			target_exe=`readlink /proc/$pid/exe | awk '{print $1}'`
			#如果target_exe非空字符串（存在运行中的spp_ctrl）
			if [ -n "$target_exe" ]
			then
				local_exe=`pwd`"/$spp_ctrl_progname"
				#比较运行中的spp_ctrl是否为当前目录的spp_ctrl
				if [ $target_exe -ef $local_exe ]
				then
					echo "program already started."
				exit
				fi
			fi
		done
		#当前实例未启动，启动spp
		./$spp_ctrl_progname $spp_ctrl_confname
	fi
fi

if [ $1 = "stop" ]
then
	pidnum=`ps -ef|grep "./$spp_ctrl_progname $spp_ctrl_confname"|grep -v grep|wc -l`
	if [ $pidnum -lt 1 ]
	then
		echo "no program killed."
	else
		for pid in `ps -ef|grep "./$spp_ctrl_progname $spp_ctrl_confname"|grep -v grep|awk '{print $2}'`
		do
			target_exe=`readlink /proc/$pid/exe | awk '{print $1}'`
			#如果target_exe非空字符串（存在运行中的spp_ctrl）
			if [ -n "$target_exe" ]
			then
				local_exe=`pwd`"/$spp_ctrl_progname"
				#比较运行中的spp_ctrl是否为当前目录的spp_ctrl
				if [ $target_exe -ef $local_exe ]
				then
					#发信号10安全退出
					kill -10 $pid
				fi
			fi
		done
		sleep 1
		rm -rf /tmp/mq_comsumer_*.lock
		echo "program stoped."
	fi
fi

if [ $1 = "force_stop" ]
then
	for pid in `ps -ef|egrep "$spp_ctrl_progname|$spp_proxy_progname|$spp_worker_progname"|grep -v grep|awk '{print $2}'`
	do
		target_exe=`readlink /proc/$pid/exe | awk '{print $1}'`
		local_exe=`pwd`"/$spp_ctrl_progname"
		if [ $target_exe -ef $local_exe ]
		then
			kill -9 $pid
			continue
		fi
		local_exe=`pwd`"/$spp_proxy_progname"
		if [ $target_exe -ef $local_exe ]
		then
			kill -9 $pid
			continue
		fi
		local_exe=`pwd`"/$spp_worker_progname"
		if [ $target_exe -ef $local_exe ]
		then
			kill -9 $pid
			continue
		fi
	done
	echo "program force stoped."
fi

if [ $1 = "reload" ]
then
for pid in `ps -ef|grep "./$spp_ctrl_progname $spp_ctrl_confname"|grep -v grep|awk '{print $2}'`
do
	target_exe=`readlink /proc/$pid/exe | awk '{print $1}'`
	local_exe=`pwd`"/$spp_ctrl_progname"
	if [ $target_exe -ef $local_exe ]
	then
		kill -12 $pid
	fi      
	done
	echo "program reloaded."
fi


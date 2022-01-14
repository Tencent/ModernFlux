#!/bin/sh

#入参检查
if [ $# -ne 2 ];then
    echo "USAGE: $0 [core|gstack] remainfilenum"
    echo "e.g. : $0 core 10"
    echo "e.g. : $0 pkg  10"
    exit 1
fi

#删除的文件个数=总的文件个数-保留的文件个数
if [ $1 = "gstack" ];then
    total=`ls /data/spp_exception_log/spp_gstack* 2>/dev/null | wc -l`
fi

if [ $1 = "pkg" ]; then
    total=`ls /data/spp_exception_log/spp_pack* 2>/dev/null | wc -l`
fi

if [ $total -gt $2 ]
then
    rmnum=$(($total-$2))
else
    exit 0
fi

#删除多余的文件
if [ $1 = "gstack" ];then
    for file in `ls -t /data/spp_exception_log/spp_gstack* 2>/dev/null | tail -n $rmnum`
    do
        rm $file
    done
else
    for file in `ls -t /data/spp_exception_log/spp_pack* 2>/dev/null | tail -n $rmnum`
    do
        rm $file
    done
fi


exit 0

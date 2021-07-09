#!/bin/bash
filepath="../../../publish/flux/bin/qagent"
if [ -s $filepath ];then
ls -l $filepath && md5sum $filepath
cp qagent $filepath
ls -l $filepath && md5sum $filepath
fi

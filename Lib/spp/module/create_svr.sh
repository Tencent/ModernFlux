#!/bin/sh

if [ $# != 2 ] ; then 
	echo "usage: $0 ServerName Port" 
	exit 1; 
fi 

function replace()
{
	sed -i "s/$2/$1/g" $3
}

if [ -d $1 ]; then
	echo -e "\033[31m \033[05m Please backup and remove $1 DIR !!!\033[0m"
	exit 1
fi 

cp demo Test -rpf

cd Test

replace $1 "Example" "Example.cpp"
replace $1 "Example" "ExampleMsg.cpp"
replace $1 "Example" "ExampleMsg.h"
replace $1 "Example" "Makefile"

low_name=$(echo $1 | tr '[A-Z]' '[a-z]') 
replace $low_name "example" "Makefile"

mv Example.cpp $1.cpp
mv ExampleMsg.cpp $1Msg.cpp
mv ExampleMsg.h $1Msg.h

cd ../
mv Test $1

cd $1
make

cd ../../etc/

replace $2 "5574" "service.yaml"
replace $2 "5575" "service.yaml"
replace $low_name "module_test" "service.yaml"

cd ../bin/
./yaml_tool x

cd ../module 

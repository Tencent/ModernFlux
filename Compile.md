# 使用说明

## 一、编译环境

该项目需在gcc-4.6和protoc-3.5.1(-fPIC编译)的环境下编译运行，如需修改.proto文件并重新编译需要安装protobuf

### 1、gcc-4.6

查询gcc版本命令：

~~~ linux
gcc -v
~~~

如版本不符，可按如下步骤更换到gcc-4.6版本：

将shell texinfo降级到4.13：

~~~
wget http://ftp.gnu.org/gnu/texinfo/texinfo-4.13a.tar.gz
~~~

~~~
tar -zxvf texinfo-4.13a.tar.gz
~~~

~~~
cd texinfo-4.13
~~~

~~~
./configure
~~~

~~~ 
make
~~~

~~~ 
sudo make install
~~~

下载gcc、gmp和mpfr：

~~~
wget http://ftp.gnu.org/gnu/gcc/gcc-4.6.1/gcc-4.6.1.tar.bz2
~~~

~~~
wget ftp://gcc.gnu.org/pub/gcc/infrastructure/{gmp-4.3.2.tar.bz2,mpc-0.8.1.tar.gz,mpfr-2.4.2.tar.bz2}
~~~

编译gmp

~~~
tar -jxf gmp-4.3.2.tar.bz2 
~~~

~~~
cd gmp-4.3.2
~~~

~~~
./configure --prefix=/usr/local/gmp
~~~

~~~ 
make &&make install
~~~

编译mpfr

~~~
cd ..
~~~

~~~
tar -jxf mpfr-2.4.2.tar.bz2
~~~

~~~
cd mpfr-2.4.2
~~~

~~~
./configure --prefix=/usr/local/mpfr -with-gmp=/usr/local/gmp
~~~

~~~
make &&make install
~~~

编译mpc

~~~
cd ..
~~~

~~~
tar -zxvf mpc-0.8.1.tar.gz
~~~

~~~ 
cd mpc-0.8.1
~~~

~~~
./configure --prefix=/usr/local/mpc -with-mpfr=/usr/local/mpfr -with-gmp=/usr/local/gmp
~~~

~~~
make &&make install
~~~

编译gcc-4.6

~~~
cd ..
~~~

~~~ 
tar -jxf gcc-4.6.1.tar.bz2
~~~

~~~
cd gcc-4.6.1
~~~

~~~
./configure --prefix=/usr/local/gcc --enable-threads=posix --disable-checking --disable-multilib --enable-languages=c,c++ --with-gmp=/usr/local/gmp --with-mpfr=/usr/local/mpfr/ --with-mpc=/usr/local/mpc/
~~~

根据自己系统情况绑定库的路径

~~~
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/mpc/lib:/usr/local/gmp/lib:/usr/local/mpfr/lib/
~~~

~~~
make && make install
~~~

make成功，make install  成功。

~~~
make[4]: Leaving directory `/root/gcc-4.6.1/x86_64-unknown-linux-gnu/libgomp'
make[3]: Leaving directory `/root/gcc-4.6.1/x86_64-unknown-linux-gnu/libgomp'
make[2]: Leaving directory `/root/gcc-4.6.1/x86_64-unknown-linux-gnu/libgomp'
make[1]: Leaving directory `/root/gcc-4.6.1'
~~~

根据自己系统情况，新建软链接

~~~
mkdir -p /usr/bin/gcc_backup/
~~~

~~~
mv /usr/bin/{gcc,g++} /usr/bin/gcc_backup/
~~~

~~~ 
ln -s /usr/local/gcc/bin/gcc /usr/bin/gcc
~~~

~~~
ln -s /usr/local/gcc/bin/g++ /usr/bin/g++
~~~

查看gcc版本

~~~
[root@master gcc-4.6.1]#  gcc --version
gcc (GCC) 4.6.1
Copyright (C) 2011 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
~~~



### 2、protoc-3.5.1(-fPIC编译)

查询gcc版本命令：

~~~ linux
protoc --version
~~~

如版本不符或没有使用-fPIC选项编译出protobuf静态库libprotobuf.a，可按如下步骤更换到protoc-3.5.1版本:

卸载protoc：

~~~
which protoc 
~~~

根据自己系统情况找到的安装路径

~~~
rm /usr/local/bin/protoc 
~~~

下载完整包：

~~~
wget https://github.com/protocolbuffers/protobuf/releases/download/v3.5.1/protobuf-all-3.5.1.tar.gz
~~~

安装依赖：

~~~
yum -y install gcc-c++
~~~

解压

~~~
tar -zxvf protobuf-all-3.5.1.tar.gz 
~~~

编译/安装

~~~
cd protobuf-3.5.1
~~~

~~~
./autogen.sh
~~~

根据自己系统情况设置安装路径，同时使用-fPIC选项编译出protobuf静态库libprotobuf.a

~~~
./configure --prefix=/usr/local CFLAGS="-fPIC"  CXXFLAGS="-fPIC"
~~~

~~~
make
~~~

~~~
make install
~~~

make成功，make install  成功。

查看版本：

~~~
root@homestead:/usr# protoc --version

libprotoc 3.5.1
~~~

根据自己系统情况,移动src/google/到usr/local/include:

~~~
cp -r /src/google/ /usr/local/include
~~~



### 3、protobuf

下载 protobuf安装包

~~~
git clone https://github.com/protocolbuffers/protobuf.git
~~~

安装依赖库

~~~
sudo apt-get install autoconf automake libtool curl make g++ unzip libffi-dev -y
~~~

安装

~~~
cd protobuf/
~~~

~~~
./autogen.sh
~~~

~~~
./configure
~~~

~~~
make
~~~

~~~
sudo make install
~~~

~~~
sudo ldconfig
~~~





## 二、Protocol

pb协议，如需修改串行化的数据结构信息，请先编辑文件夹内的open_app_desc.proto，再重新编译

open_app_desc.pb.cc和open_app_desc.pb.h：

~~~linux
make clean
make
~~~

如需搭配go语言使用，编译命令例如：

~~~ linux
protoc -I=/data/ModernFlux/Protocol/ --go_out=/data/ModernFlux/Protocol/ /data/ModernFlux/Protocol/open_app_desc.proto
~~~

以上三个路径参数分别是：

-I=“.proto文件存放的路径”

　　　　-go_out= “输出的go文件存放的路径”

　　　　　　　　“.proto的源码路径”

## 三、QuaAgent

小配额agent,针对于qps不高的活动进行负载均衡。

环境依赖：Linux x64 环境，建议CentOS 7.x；Go 1.12及以上版本。

编译：进入QuaAgent,本项目依赖go mod进行包管理。

~~~
go mod tidy
~~~

~~~
cd src/flux
~~~

生成二进制文件qagent

~~~
go build -o qagent
~~~

~~~
./qagent
~~~

提交依赖

~~~
go mod verify
~~~





## 四、QuaServer

配额计算Server,统计计算并分配各节点配额。

进入QuaServer

~~~
make
~~~

 即可在lib文件夹内生成aquota.so

移动aquota.so到Lib下的spp/bin，可搭配脚本spp.sh启动server端

~~~
mv aquota.so ../../Lib/spp/bin
~~~

~~~
sh spp.sh start
~~~


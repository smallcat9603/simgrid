#!/bin/bash

SIMGRIDPATH="../../simgrid/inst/"
CONFIGPATH="../../../config/common/test_64"
sudo apt-get install ant-optional

export JAVA_HOME=/usr/lib/jvm/java-6-openjdk-amd64/
#Customized X10 build
#http://x10-lang.org/x10-development/building-x10-from-source.html
#http://x10-lang.org/documentation/practical-x10-programming/x10rt-implementations.html

MPIPROP="simgrid"
#MPIPROP="ompi"
cp x10rt_${MPIPROP}.properties SX10/x10.dist/etc/x10rt_mpi.properties
#corresponding property file should be in etc/x10rt_mpi.properties
ANTOPTIONS=" -DX10RT_MPI=true -DNO_CHECKS=true -Doptimize=true -Davailable.procs=3" # 

git clone https://github.com/scalegraph/sx10.git SX10
cd SX10
SX10PATH=${PWD}
git checkout develop
cd x10.dist
#ant ${ANTOPTIONS} dist

export PATH=${SX10PATH}/x10.dist/bin:$PATH
export LD_LIBRARY_PATH=${SX10PATH}/x10.dist/lib:${SX10PATH}/x10.dist/stdlib/lib:$LD_LIBRARY_PATH
export C_INCLUDE_PATH=${SX10PATH}/x10.dist/include:$C_INCLUDE_PATH

cd ../../

git clone https://github.com/scalegraph/scalegraph.git ScaleGraph
cd ScaleGraph
# 今一番安定しているバージョンが"hotfix/xpregel"ブランチなのでこれを使ってください。
git checkout hotfix/xpregel
SCALEGRAPHPATH=${PWD}

if [[ ! -f "pagerank" ]]; then
	echo "Compiling the X10 bench, may take a while.."
	x10c++  -x10rt mpi -sourcepath ${SCALEGRAPHPATH}/src -cxx-prearg -I${SCALEGRAPHPATH}/include -O -o pagerank -make -make-arg "--jobs=3" ${SCALEGRAPHPATH}/src/test/PageRankTest.x10
fi


export X10_NTHREADS=1 #number of processes per place (i.e. node)
export GC_NPROCS=64 #number of nodes

../${SIMGRIDPATH}/bin/smpirun -np 64 --cfg=smpi/privatize_global_variables:yes  -platform ${CONFIGPATH}.xml -hostfile ${CONFIGPATH}.txt   ./pagerank rmat 20 - perf write ./tmp

#X10_NTHREADSは１プロセスあたりのスレッド数、プログラムの引数 20 は問題サイズ（頂点数が2^20という意味）です。

#今日のお話ではコンパイラを変更する必要があるということでしたので少し調べてみました。
#以下の２つのファイルを変更すればいいかと思います（すみません、こちらでは試していません）
#- X10ビルド時のコンパイラ
#SX10/x10.runtime/Make.rules の 177行目: override MPICXX = mpicxx -g

#- ScaleGraphのサンプルビルド時のコンパイラ
#SX10ビルド後にできるファイル SX10/x10.dist/etc/x10rt_mpi.properties の 2行目:
#X10LIB_CXX=mpicxx -g

#※通信が関係ないコードはg++を使ってビルドするようです。

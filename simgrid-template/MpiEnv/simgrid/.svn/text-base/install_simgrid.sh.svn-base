#!/bin/bash

if [ $# -lt 1 ]; then
    echo "You need to give an argument: Either <Release> for full-speed simulations or <Debug> for debugging simulation issues"
    exit
fi


#sudo apt-get install cmake f2c
sudo apt-get -y install git cmake build-essential libboost-dev libconfig++-dev libconfig8-dev libgtk2.0-dev freeglut3-dev asciidoc libqt4-dev




NODENAME=`uname -n`
BASE_PATH=`pwd`
#SIMGRID_VERSION=3.11
#SIMGRID_INSTALL=${BASE_PATH}/inst${SIMGRID_VERSION}
SIMGRID_GIT_INSTALL=${BASE_PATH}/inst-git
SIMGRID_DEBUG_OPTIONS="-DCMAKE_BUILD_TYPE=Debug  -Denable_compile_optimizations=off -Denable_smpi=on -Denable_ns3=on -Denable_debug=on -Denable_documentation=off -Dns3_path=${NS3_INSTALL}"
SIMGRID_RELEASE_OPTIONS="-DCMAKE_BUILD_TYPE=Release  -Denable_compile_optimizations=on -Denable_smpi=on -Denable_ns3=on -Denable_debug=off -Denable_documentation=off -Dns3_path=${NS3_INSTALL}"


NS3_VERSION=3.19
NS3_INSTALL=${BASE_PATH}/instns3/
mkdir ${NS3_INSTALL}


#mkdir  ${SIMGRID_INSTALL}
#if [ ! -f "SimGrid-${SIMGRID_VERSION}.tar.gz" ]; then
#    wget -v http://gforge.inria.fr/frs/download.php/33124/SimGrid-${SIMGRID_VERSION}.tar.gz
#fi
#if [ ! -d "SimGrid-${SIMGRID_VERSION}" ]; then
#    tar -xzf SimGrid-${SIMGRID_VERSION}.tar.gz
#fi

if [[ ${NODENAME} == "calc"* ]]; then
    #NII/calc* hack
    echo "Detected NII Calc computers -> GIT-free procedure"
    if [ ! -d "Simgrid-git" ]; then
	tar -xzf /home/fabien/git_archives/simgrid.tar.gz 
	mv simgrid Simgrid-git
    fi
    if [ ! -d "pajeng" ]; then
	tar -xzf /home/fabien/git_archives/pajeng.tar.gz 
    fi
    if [ ! -d "viva" ]; then
	tar -xzf /home/fabien/git_archives/viva.tar.gz 
    fi
else
    if [ ! -d "Simgrid-git" ]; then
	#Install the simgrid Git 
	rm -rf simgrid
	# use http protocol since NII filters GIT port 	
	git clone http://gforge.inria.fr/git/simgrid/simgrid.git/  
	mv simgrid Simgrid-git
    else
	#Update it, if git port is open
	cd Simgrid-git
	git pull
	cd ..
    fi
    if [ ! -d "pajeng" ]; then
	git clone git://github.com/schnorr/pajeng.git 
    fi
    if [ ! -d "viva" ]; then
	git clone git://github.com/schnorr/viva.git 
    fi
fi


if [ ! -f "ns-allinone-${NS3_VERSION}.tar.bz2" ]; then
    wget --no-check-certificate https://www.nsnam.org/release/ns-allinone-${NS3_VERSION}.tar.bz2
fi
if [ ! -d "ns-allinone-${NS3_VERSION}" ]; then
    tar -xjf  ns-allinone-${NS3_VERSION}.tar.bz2
fi

#Build the NS3
if [ ! -d ${NS3_INSTALL} ]; then
    cd ns-allinone-${NS3_VERSION}
    ./build.py > build.log
    cd ns-${NS3_VERSION}
    ./waf configure --prefix=${NS3_INSTALL}
    ./waf
    ./waf install
    cd ../..
    echo "export LD_LIBRARY_PATH=\${LD_LIBRARY_PATH}:${NS3_INSTALL}">>~/.bashrc
fi

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${NS3_INSTALL}

#if [ ! -d ${SIMGRID_INSTALL} ]; then
#    #Build the simgrid classic version
#    cd SimGrid-${SIMGRID_VERSION}
#    
#    if [[ $1 = "Debug" ]]; then
#	#Debug configuration
#	cmake -DCMAKE_INSTALL_PREFIX=${SIMGRID_INSTALL} ${SIMGRID_DEBUG_OPTIONS} .
#    else
#	#Release configuration
#	cmake -DCMAKE_INSTALL_PREFIX=${SIMGRID_INSTALL}  ${SIMGRID_RELEASE_OPTIONS} .
#    fi
#    
#    make -j 4
#    make -j 4 install    
#    cd ..
#fi


cd Simgrid-git

if [ ! -d ${SIMGRID_GIT_INSTALL} ]; then

    if [[ $1 = "Debug" ]]; then
	#Debug configuration
	cmake  -DCMAKE_INSTALL_PREFIX=${SIMGRID_GIT_INSTALL} ${SIMGRID_DEBUG_OPTIONS} .
    else
	#Release configuration
	cmake  -DCMAKE_INSTALL_PREFIX=${SIMGRID_GIT_INSTALL}  ${SIMGRID_RELEASE_OPTIONS} .
    fi
    
    make -j 4
    make -j 4 install

else
    echo "WARNING:!!!!Remove folder ${SIMGRID_GIT_INSTALL} to force recompile of git-version of Simgrid, if you want it!!!!"
fi
cd ..
#Create a symlink in case we want to try other versions of simgrid
ln -s inst-git inst


# Install PajeNG
cd pajeng 
cmake -DCMAKE_INSTALL_PREFIX=${BASE_PATH}/inst-pajeng -DLATEX_OUTPUT_PATH=${BASE_PATH}/inst-pajeng/latex -DCMAKE_BUILD_TYPE=${1} .
make -j 3
make install
cd ..

pwd 
# Install Viva
cd viva 
cmake -DCMAKE_INSTALL_PREFIX=${BASE_PATH}/inst-viva  -DPAJENG_PATH:PATH=${BASE_PATH}/inst-pajeng -DCMAKE_BUILD_TYPE=${1} . 
make -j 3
make install

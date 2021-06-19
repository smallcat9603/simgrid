#!/bin/sh
NODENAME=`uname -n`

if [[ ${NODENAME} == "calc"* ]]; then
	echo "Detected Calc machine -> GIT-fre install.."
	cp /home/fabien/git_archives/graph500.tar.gz ./
	tar -xzf graph500.tar.gz
else
	git clone https://gitorious.org/graph500/graph500.git
	cd graph500
	git pull 
	cd ..
fi


cd graph500
git apply ../mpienv.patch

cp ../make.inc ./

#Due to silly relative directory issues, let the compile for later 
#make

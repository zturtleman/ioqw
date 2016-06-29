#!/bin/bash
localPATH=`pwd`
export BUILD_CLIENT="0"
export BUILD_SERVER="1"
export USE_CURL="1"
export USE_CODEC_OPUS="1"
export USE_VOIP="1"
export COPYDIR="~/quakewars"
IOQWREMOTE="https://github.com/ioid3-games/ioid3-qw.git"
IOQWLOCAL="/tmp/ioid3-qwcompile"
JOPTS="-j2" 
echo "This process requires you to have the following installed through your distribution:
 make
 git
 and all of the dependencies necessary for the Quake Wars server.
 If you do not have the necessary dependencies this script will bail out.
 Please edit this script. Inside you will find a COPYDIR variable you can alter to change where Quake Wars will be installed to."
while true; do
        read -p "Are you ready to compile Quake Wars in the $IOQWLOCAL directory, and have it installed into $COPYDIR? " yn
case $yn in
        [Yy]* )
if  [ -x "$(command -v git)" ] && [ -x "$(command -v make)" ] ; then
        git clone $IOQWREMOTE $IOQWLOCAL && cd $IOQWLOCAL && make $JOPTS && make copyfiles && cd $localPATH && rm -rf $IOQWLOCAL
fi
        exit;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
esac
done

#!/bin/bash

# set compile flags: 32-bit
#export CFLAGS="-march=i486 -mtune=i686 -Os -pipe"
#export CXXFLAGS="-march=i486 -mtune=i686 -Os -pipe"
#export LDFLAGS="-Wl,-O1"

# set compile flags: 64-bit
#export CFLAGS="-mtune=generic -Os -pipe"
#export CXXFLAGS="-mtune=generic -Os -pipe -fno-exceptions -fno-rtti"
#export LDFLAGS="-Wl,-O1"
#export CC="gcc -flto -fuse-linker-plugin -mtune=generic -Os -pipe"
#export CXX="g++ -flto -fuse-linker-plugin -mtune=generic -Os -pipe -fno-exceptions -fno-rtti"
export CXXFLAGS="$CXXFLAGS $(fltk-config --cxxflags)"
export CXXFLAGS="$CXXFLAGS -Wall -ffunction-sections -fdata-sections -Wno-strict-aliasing"
export LDFLAGS="$LDFLAGS $(fltk-config --ldstaticflags --use-images)"
export LDFLAGS="$LDFLAGS -Wl,-gc-sections"

# set additional compile flags
#export LIBS="-lwebkitfltk -lz -pthread -lxslt -lxml2 -ldl -lsqlite3 -lharfbuzz -lharfbuzz-icu -lfreetype -lfontconfig -lcairo -lpng -ljpeg -lrt -lcurl -lssl -lcrypto -lglib-2.0 -static-libgcc -static-libstdc++"

# export directory paths
#export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig

PREFIX=''
INSTALL=0




# check that the correct parameters have been sent to the script
if [[ "$1" =~ ^'PREFIX=' ]]; then
	PREFIX="$1"
fi
if [ "$1" == 'install' ] || [ "$2" == 'install' ]; then
	INSTALL=1
fi
#if (( INSTALL == 0 )); then
#	echo -e "\nERROR: You must include the 'install' parameter:\n"
#	echo -e "\t./make [PREFIX=/packaging/dir] install\n"
#	exit 1
#fi

# check that the account has appropriate permissions to install the software
if (( $INSTALL > 0 )) && (( $EUID > 0 )); then		# checks that this ACTION is being run as an elevated account (e.g. as root or using sudo) so we don't have any issues getting the directories all setup
	echo -e "\nThis install script was executed without sufficient priviledges. Try using a different account or 'sudo', exiting.\n"
	exit 1
fi

# check that the required packages are installed to compile the software
tce-load -wi webkitfltk-dev.tcz fltk-1.3-dev.tcz webkit-dev.tcz glib2-dev.tcz

# define the required packages for this software to run
DEP="fltk-1.3.tcz\nsqlite3.tcz\nlibEGL.tcz\nlibGLESv2"


# compile the software
echo -e "\nCompiling the software...\n"
#g++ -std=c++11 -o $1 *.C *.cpp $CXXFLAGS $LDFLAGS || exit 0
g++ -o web.ui -std=c++11 web.ui.cpp `pkg-config --cflags --libs \
webkitfltk` `fltk-config --cxxflags --ldflags --use-images` -lz \
-pthread -lxslt -lxml2 -ldl -lsqlite3 `icu-config --ldflags` -lharfbuzz \
-lharfbuzz-icu -lfreetype -lfontconfig -lcairo -lpng -ljpeg -lrt -lcurl \
-lssl -lcrypto -lglib-2.0

# strip the debug symbols from the just-compiled software
find . | xargs file | grep "executable" | grep ELF | grep "not stripped" | cut -f 1 -d : | xargs strip --strip-unneeded 2> /dev/null || find . | xargs file | grep "shared object" | grep ELF | grep "not stripped" | cut -f 1 -d : | xargs strip -g 2> /dev/null

# install the software
if (( $INSTALL == 0 )); then
	echo -e "\nThe software has compiled successfully!\n"
	exit 0
fi

echo -e "\nInstalling the software...\n"
if [ ! -d "${PREFIX}/usr/local/bin" ]; then
	mkdir -p "${PREFIX}/usr/local/bin"
fi
cp $1 "${PREFIX}/usr/local/bin"

if [ ! -d "${PREFIX}/usr/share/man/man1" ]; then
	mkdir -p "${PREFIX}/usr/share/man/man1"
fi
gzip < $1.1 > "${PREFIX}/usr/share/man/man1/$1.1.gz"

# last-call file structure manipulation
echo -e "\nYou are now ready to use the software!\n"
#exit 0						# prevents the packaging steps below (since most users won't need them)



# creating the .tcz files
cd /tmp 
mksquashfs "$1" "$1.tcz"
md5sum "$1.tcz" > "$1.tcz.md5.txt"		# create the md5 checksum file
echo -e $DEP > "$1.tcz.dep"			# create the dependency file
cd "/tmp/$1"
find usr -not -type d > "../$1.tcz.list"	# create the manifest

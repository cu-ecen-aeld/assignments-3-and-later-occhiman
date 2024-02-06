#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v6.7.3
BUSYBOX_VERSION=1_36_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-
WRITER_PATH=${PWD}
LIBS_PATH=$(which ${CROSS_COMPILE}gcc)

#Path to the Cross compiler libraries
LIBS_PATH=${LIBS_PATH%/*}
LIBS_PATH=${LIBS_PATH%/*}

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper

    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig

    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all

fi

echo "Adding the Image in outdir"
cp "$OUTDIR/linux-stable/arch/arm64/boot/Image" "$OUTDIR"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir "$OUTDIR/rootfs"
cd "$OUTDIR/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var conf
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
else
    cd busybox
fi

# TODO: Make and install busybox
export CROSS_COMPILE=aarch64-none-linux-gnu-
echo $CROSS_COMPILE
echo "building busybox"
make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

echo "Library dependencies"
${CROSS_COMPILE}readelf -a "${OUTDIR}/rootfs/bin/busybox" | grep "program interpreter"
${CROSS_COMPILE}readelf -a "${OUTDIR}/rootfs/bin/busybox" | grep "Shared library"


# TODO: Add library dependencies to rootfs
cd ${OUTDIR}/rootfs
echo "Copying libraries"
cp ${LIBS_PATH}/aarch64-none-linux-gnu/libc/lib/ld-linux-aarch64.so.1 ./lib/
cp ${LIBS_PATH}/aarch64-none-linux-gnu/libc/lib64/libm.so.6 ./lib64/
cp ${LIBS_PATH}/aarch64-none-linux-gnu/libc/lib64/libresolv.so.2 ./lib64/
cp ${LIBS_PATH}/aarch64-none-linux-gnu/libc/lib64/libc.so.6 ./lib64/


# TODO: Make device nodes
mknod -m 666 dev/null c 1 3
mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
echo "Building writer"
cd ${WRITER_PATH}
make clean
make CROSS_COMPILE=aarch64-none-linux-gnu-

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
echo "copying writer to rootfs"
cp ${WRITER_PATH}/writer ${OUTDIR}/rootfs/home
cp ${WRITER_PATH}/finder.sh ${OUTDIR}/rootfs/home
cp ${WRITER_PATH}/finder-test.sh ${OUTDIR}/rootfs/home
cp -a ${WRITER_PATH}/conf ${OUTDIR}/rootfs/home
cp -r ${WRITER_PATH}/../conf ${OUTDIR}/rootfs
cp ${WRITER_PATH}/autorun-qemu.sh ${OUTDIR}/rootfs/home

# TODO: Chown the root directory
echo "Chown the root directory"
cd ${OUTDIR}/rootfs
chown -R root:root *
# TODO: Create initramfs.cpio.gz
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio

echo "Initramfs compression"
cd ${OUTDIR}
gzip -f initramfs.cpio

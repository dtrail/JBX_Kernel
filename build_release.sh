#!/bin/bash
set -m

# Build script for JBX-Kernel RELEASE
make mrproper
make ARCH=arm distclean

# We build the kernel and its modules first
# Launch execute script in background
# First get tags in shell
cd ~/android/system
make ARCH=arm distclean
make mrproper
source build/envsetup.sh
lunch 9

# built kernel & modules
make -j8 TARGET_KERNEL_SOURCE=/home/mnl-manz/razr_kdev_kernel/JBX_Kernel/ TARGET_KERNEL_CONFIG=mapphone_OCE_defconfig $OUT/boot.img

# We don't use the kernel but the modules
cp out/target/product/umts_spyder/system/lib/modules/* ~/razr_kdev_kernel/JBX_Kernel/built/rls/system/lib/modules/

# Switch to kernel folder
cd ~/razr_kdev_kernel/JBX_Kernel

# Exporting the toolchain (You may change this to your local toolchain location)
export PATH=~/build/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH

# export the flags (leave this alone)
export ARCH=arm
export SUBARCH=arm
export CROSS_COMPILE=arm-eabi-

# define the defconfig (Do not change)
make ARCH=arm mapphone_OCE_defconfig
export LOCALVERSION="-JBX-0.6c-Hybrid-X"

# execute build command with "-j4 core flag" 
# (You may change this to the count of your CPU.
# Don't set it too high or it will result in a non-bootable kernel.
make -j4

# Copy and rename the zImage into nightly/nightly package folder
# Keep in mind that we assume that the modules were already built and are in place
# So we just copy and rename, then pack to zip including the date
cp arch/arm/boot/zImage built/rls/system/etc/kexec/kernel
cd built/rls
zip -r "JBX-Kernel-Hybrid-X_$(date +"%Y-%m-%d").zip" *
cp "JBX-Kernel-Hybrid-X_$(date +"%Y-%m-%d").zip" ~/razr_kdev_kernel/JBX_Kernel/built


# Cleaning out
rm ~/razr_kdev_kernel/JBX_Kernel/built/nightly/system/etc/kexec/*
rm ~/razr_kdev_kernel/JBX_Kernel/built/rls/system/etc/kexec/*
rm ~/razr_kdev_kernel/JBX_Kernel/built/nightly/system/lib/modules/*
rm ~/razr_kdev_kernel/JBX_Kernel/built/nightly/**
rm ~/razr_kdev_kernel/JBX_Kernel/built/rls/*

echo "done"

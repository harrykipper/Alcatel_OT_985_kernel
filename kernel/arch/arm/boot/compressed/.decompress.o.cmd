cmd_arch/arm/boot/compressed/decompress.o := /home/elvis/arm-eabi-4.4.3/bin/arm-eabi-gcc -Wp,-MD,arch/arm/boot/compressed/.decompress.o.d  -nostdinc -isystem /home/elvis/arm-eabi-4.4.3/bin/../lib/gcc/arm-eabi/4.4.3/include -I/home/elvis/OT_985/kernel/arch/arm/include -Iinclude  -include include/generated/autoconf.h -I../mediatek/custom//common -I../mediatek/platform/mt6573/kernel/core/include/ -I../mediatek/source/kernel/include/ -I../mediatek/custom/out/jrd73_gb/kernel/dct/ -I../mediatek/custom/out/jrd73_gb/kernel/ofn/ -I../mediatek/custom/out/jrd73_gb/kernel/leds/ -I../mediatek/custom/out/jrd73_gb/kernel/sound/ -I../mediatek/custom/out/jrd73_gb/kernel/accelerometer/ -I../mediatek/custom/out/jrd73_gb/kernel/rtc/ -I../mediatek/custom/out/jrd73_gb/kernel/touchpanel/ -I../mediatek/custom/out/jrd73_gb/kernel/headset/ -I../mediatek/custom/out/jrd73_gb/kernel/core/ -I../mediatek/custom/out/jrd73_gb/kernel/kpd/ -I../mediatek/custom/out/jrd73_gb/kernel/alsps/ -I../mediatek/custom/out/jrd73_gb/kernel/magnetometer/ -I../mediatek/custom/out/jrd73_gb/kernel/camera/ -I../mediatek/custom/out/jrd73_gb/kernel/battery/ -I../mediatek/custom/out/jrd73_gb/kernel/usb/ -I../mediatek/custom/out/jrd73_gb/kernel/ofn/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/leds/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/./ -I../mediatek/custom/out/jrd73_gb/kernel/sound/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/barometer/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/accelerometer/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/fm/ -I../mediatek/custom/out/jrd73_gb/kernel/eeprom/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/eeprom/ -I../mediatek/custom/out/jrd73_gb/kernel/lcm/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/lcm/ -I../mediatek/custom/out/jrd73_gb/kernel/gyroscope/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/lens/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/lens/ -I../mediatek/custom/out/jrd73_gb/kernel/jogball/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/alsps/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/magnetometer/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/imgsensor/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/imgsensor/ -I../mediatek/custom/out/jrd73_gb/kernel/flashlight/inc/ -I../mediatek/custom/out/jrd73_gb/kernel/flashlight/ -I../mediatek/custom/out/jrd73_gb/hal/audioflinger/ -I../mediatek/custom/out/jrd73_gb/hal/sensors/ -I../mediatek/custom/out/jrd73_gb/hal/camera/ -I../mediatek/custom/out/jrd73_gb/hal/inc/ -I../mediatek/custom/out/jrd73_gb/hal/matv/ -I../mediatek/custom/out/jrd73_gb/hal/eeprom/ -I../mediatek/custom/out/jrd73_gb/hal/combo/ -I../mediatek/custom/out/jrd73_gb/hal/ant/ -I../mediatek/custom/out/jrd73_gb/hal/lens/ -I../mediatek/custom/out/jrd73_gb/hal/imgsensor/ -I../mediatek/custom/out/jrd73_gb/hal/flashlight/ -I../mediatek/custom/out/jrd73_gb/hal/bluetooth/ -D__KERNEL__ -mlittle-endian -DMODEM_3G -I../mediatek/platform/mt6573/kernel/core/include -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -Os -marm -fno-omit-frame-pointer -mapcs -mno-sched-prolog -mabi=aapcs-linux -mno-thumb-interwork -D__LINUX_ARM_ARCH__=6 -march=armv6k -mtune=arm1136j-s -msoft-float -Uarm -DMTK_WAPI_SUPPORT -DMTK_SENSOR_SUPPORT -DCUSTOM_KERNEL_ALSPS -DCUSTOM_KERNEL_MAGNETOMETER -DCUSTOM_KERNEL_ACCELEROMETER -DMTK_BT_SUPPORT -DMTK_CAMERA_APP_3DHW_SUPPORT -DMTK_VT3G324M_SUPPORT -DMTK_M4U_SUPPORT -DMTK_MATV_ANALOG_SUPPORT -DMTK_WLAN_SUPPORT -DMTK_BT_21_SUPPORT -DMTK_BT_30_SUPPORT -DMTK_BT_40_SUPPORT -DMTK_BT_PROFILE_OPP -DMTK_BT_PROFILE_SIMAP -DMTK_BT_PROFILE_PRXM -DMTK_BT_PROFILE_PRXR -DMTK_BT_PROFILE_HIDH -DMTK_BT_PROFILE_FTP -DMTK_BT_PROFILE_PBAP -DMTK_BT_PROFILE_MANAGER -DMTK_BT_PROFILE_BPP -DMTK_BT_PROFILE_BIP -DMTK_BT_PROFILE_DUN -DMTK_BT_PROFILE_PAN -DMTK_BT_PROFILE_HFP -DMTK_BT_PROFILE_A2DP -DMTK_BT_PROFILE_AVRCP -DMTK_BT_PROFILE_AVRCP14 -DMTK_BT_PROFILE_MAPS -DMTK_BT_PROFILE_MAPC -DMTK_BT_PROFILE_SPP -DMTK_WB_SPEECH_SUPPORT -DHAVE_AWBENCODE_FEATURE -DHAVE_AACENCODE_FEATURE -DMTK_WIFI_HOTSPOT_SUPPORT -DMTK_FM_SUPPORT -DMTK_GPS_SUPPORT -DMTK_MAV_SUPPORT -DHAVE_AEE_FEATURE -DMTK_SIM_AUTHENTICATION_SUPPORT -DMTK_EAP_SIM_AKA -DMT6573 -DMT6620_FM -DMTK_MT6620 -DS5K5CAGX_YUV -DOV7690_YUV -DDUMMY -DDUMMY_LENS -DMT6620 -DFM_DIGITAL_INPUT -DMTK_GPS_MT6620 -DMT6620E3 -DDUMMY_FLASHLIGHT -DMODEM_3G -DHX8357B -DLCM_WIDTH=\"320\" -DLCM_HEIGHT=\"480\" -DMTK_LCM_PHYSICAL_ROTATION=\"0\" -DMTK_SHARE_MODEM_SUPPORT=\"2\" -DMTK_SHARE_MODEM_CURRENT=\"1\" -Wframe-larger-than=1024 -fno-stack-protector -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -fpic -fno-builtin   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(decompress)"  -D"KBUILD_MODNAME=KBUILD_STR(decompress)"  -c -o arch/arm/boot/compressed/decompress.o arch/arm/boot/compressed/decompress.c

deps_arch/arm/boot/compressed/decompress.o := \
  arch/arm/boot/compressed/decompress.c \
    $(wildcard include/config/kernel/gzip.h) \
    $(wildcard include/config/kernel/lzo.h) \
    $(wildcard include/config/kernel/lzma.h) \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbdaf.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/elvis/OT_985/kernel/arch/arm/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  /home/elvis/OT_985/kernel/arch/arm/include/asm/bitsperlong.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  /home/elvis/OT_985/kernel/arch/arm/include/asm/posix_types.h \
  include/linux/linkage.h \
  /home/elvis/OT_985/kernel/arch/arm/include/asm/linkage.h \
  /home/elvis/OT_985/kernel/arch/arm/include/asm/string.h \
  arch/arm/boot/compressed/../../../../lib/decompress_inflate.c \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/inftrees.c \
  include/linux/zutil.h \
  include/linux/zlib.h \
  include/linux/zconf.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/inftrees.h \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/inffast.c \
    $(wildcard include/config/have/efficient/unaligned/access.h) \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/inflate.h \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/inffast.h \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/inflate.c \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/infutil.h \
  arch/arm/boot/compressed/../../../../lib/zlib_inflate/inffixed.h \
  include/linux/decompress/mm.h \

arch/arm/boot/compressed/decompress.o: $(deps_arch/arm/boot/compressed/decompress.o)

$(deps_arch/arm/boot/compressed/decompress.o):

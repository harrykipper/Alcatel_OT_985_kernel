cmd_arch/arm/boot/zImage := /home/elvis/arm-eabi-4.4.3/bin/arm-eabi-objcopy -O binary -R .note -R .note.gnu.build-id -R .comment -S  arch/arm/boot/compressed/vmlinux arch/arm/boot/zImage

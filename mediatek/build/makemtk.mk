SHELL        := /bin/bash

include mediatek/build/Makefile
$(call codebase-path)
#mtk-config-files := $(strip $(call mtk.config.generate-rules,mtk-config-files))
#mtk-custom-files := $(strip $(call mtk.custom.generate-rules,mtk-custom-files))
$(shell $(foreach a,$(CMD_ARGU),$(if $(filter 2,$(words $(subst =, ,$(a)))),$(a))) make -f mediatek/build/custgen.mk > /dev/null)
PRJ_MF       := $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk
include $(MTK_ROOT_CONFIG_OUT)/ProjectConfig.mk
ifdef OPTR_SPEC_SEG_DEF
  ifneq (NONE,$(OPTR_SPEC_SEG_DEF))
     include $(MTK_ROOT_SOURCE_OPERATOR)/OperatorInfo.mk
  endif
endif

remake update-api javaoptgen drvgen emigen nandgen: custgen
#ifneq ($(PROJECT),)
# include $(PRJ_MF)
#endif
# *************************************************************************
# Set PHONY
# *************************************************************************
.PHONY : new newall remake remakeall clean cleanall \
         preloader uboot kernel android \
         challmod check-modem update-modem sign-image sign-modem check-dep \
         dump-memusage gen-relkey \
         codegen btcodegen javaoptgen emigen nandgen custgen drvgen ptgen \
         update-api dist perso

#MKTOPDIR      =  $(shell pwd)
LOGDIR        =  $(MKTOPDIR)/out/target/product
S_MODULE_LOG  =  out/target/product/$(PROJECT)_$(CUR_MODULE).log
S_CODEGEN_LOG =  out/target/product/$(PROJECT)_codegen.log
CODEGEN_LOG   =  $(LOGDIR)/$(PROJECT)_codegen.log
MODULE_LOG    =  $(LOGDIR)/$(PROJECT)_$(CUR_MODULE).log
S_LOG =  out/target/product/$(PROJECT)_
LOG   =  $(LOGDIR)/$(PROJECT)_
CUSTOM_MEMORY_HDR = mediatek/custom/$(PROJECT)/preloader/inc/custom_MemoryDevice.h
CUSTOM_NAND_HDR = mediatek/custom/$(PROJECT)/common/nand_device_list.h

#add by hpr_hfhuang CR:294553 begin
#MEMORY_DEVICE_XLS = mediatek/build/tools/emigen/$(MTK_PLATFORM)/MemoryDeviceList_$(MTK_PLATFORM).xls
ifeq ($(JRD_FOUR_PLUS_FOUR_SUPPORT),yes)
MEMORY_DEVICE_XLS = mediatek/build/tools/emigen/$(MTK_PLATFORM)/MemoryDeviceList_512M_$(MTK_PLATFORM).xls
else
MEMORY_DEVICE_XLS = mediatek/build/tools/emigen/$(MTK_PLATFORM)/MemoryDeviceList_$(MTK_PLATFORM).xls
endif
#add by hpr_hfhuang CR:294553 end

USERID        =  $(shell whoami)
PRELOADER_WD  =  mediatek/source/preloader
UBOOT_WD      =  bootable/bootloader/uboot
JRD_DEVICE    =  device/jrdcom/gin
KERNEL_WD     =  kernel
ANDROID_WD    =  .
ALL_MODULES   =  
MAKE_DEBUG    =  --no-print-directory
hide         :=  @
CMD_ARGU2    :=  $(filter-out -j%, $(CMD_ARGU))
REMAKECMD    :=  make -fmediatek/build/makemtk.mk CMD_ARGU=$(CMD_ARGU) $(CMD_ARGU2) $(MAKE_DEBUG)
MAKEPERSO    :=  make $(MAKEJOBS) perso $(CMD_ARGU) $(MAKE_DEBUG)
CPUCORES     :=  $(shell cat /proc/cpuinfo | grep processor | wc -l)
MAKEJOBS     :=  -j$(CPUCORES)

# Memory partition auto-gen related variable initilization
MEM_PARTITION_GENERATOR   := mediatek/build/tools/ptgen/ptgen.pl
MEM_PARTITION_TABLE       := mediatek/build/tools/ptgen/partition_table.xls
PARTITION_HEADER_LOCATION := mediatek/custom/$(PROJECT)/common
OTA_SCATTER_GENERATOR     := mediatek/build/tools/ptgen/ota_scatter.pl

#ifeq ($(ACTION),update-api)
#   MAKEJOBS := 
#endif
MAKECMD      :=  make $(MAKEJOBS) $(CMD_ARGU) $(MAKE_DEBUG)

SHOWTIMECMD   =  date "+%Y/%m/%d %H:%M:%S"
SHOWRSLT      =  /usr/bin/perl $(MKTOPDIR)/mediatek/build/tools/showRslt.pl

PRELOADER_IMAGES := $(PRELOADER_WD)/bin/preloader_$(PROJECT).bin
UBOOT_IMAGES     := $(UBOOT_WD)/uboot_$(PROJECT).bin \
                    $(UBOOT_WD)/logo.bin
KERNEL_IMAGES    := $(KERNEL_WD)/kernel_$(PROJECT).bin
ANDROID_IMAGES   := $(LOGDIR)/$(PROJECT)/system.img \
                    $(LOGDIR)/$(PROJECT)/boot.img \
                    $(LOGDIR)/$(PROJECT)/recovery.img \
                    $(LOGDIR)/$(PROJECT)/secro.img \
                    $(LOGDIR)/$(PROJECT)/userdata.img
ifeq (MT6573, $(MTK_PLATFORM))
  ifeq (android, $(CUR_MODULE))
    ANDROID_IMAGES += $(LOGDIR)/$(PROJECT)/DSP_BL
  endif
endif

ifeq ($(MTK_LCA_SUPPORT),yes)
  SCATTER_FILE := mediatek/source/misc/$(MTK_PLATFORM)_Android_scatter_LCA.txt
else
  SCATTER_FILE := mediatek/source/misc/$(MTK_PLATFORM)_Android_scatter.txt
endif
#wschen
OTA_SCATTER_FILE := mediatek/source/misc/ota_scatter.txt

export TARGET_PRODUCT=$(PROJECT)
export FLAVOR=$(FLAVOR)

ifneq ($(ACTION), )
  SHOWBUILD     =  $(ACTION)
else
  SHOWBUILD     =  build
endif
SHOWTIME      =  $(shell $(SHOWTIMECMD))
ifeq ($(ENABLE_TEE), TRUE)
  DEAL_STDOUT := 2>&1 | tee -a $(MODULE_LOG)
  DEAL_STDOUT_CODEGEN := 2>&1 | tee -a $(CODEGEN_LOG)
  DEAL_STDOUT_BTCODEGEN := 2>&1 | tee -a $(LOG)btcodegen.log
  DEAL_STDOUT_CUSTGEN := 2>&1 | tee -a $(LOG)custgen.log
  DEAL_STDOUT_EMIGEN := 2>&1 | tee -a $(LOG)emigen.log
  DEAL_STDOUT_NANDGEN := 2>&1 | tee -a $(LOG)nandgen.log
  DEAL_STDOUT_JAVAOPTGEN := 2>&1 | tee -a $(LOG)javaoptgen.log
  DEAL_STDOUT_IMEJAVAOPTGEN := 2>&1 | tee -a $(LOG)imejavaoptgen.log
  DEAL_STDOUT_SIGN_IMAGE := 2>&1 | tee -a $(LOG)sign-image.log
  DEAL_STDOUT_DRVGEN := 2>&1 | tee -a $(LOG)drvgen.log
  DEAL_STDOUT_SIGN_MODEM := 2>&1 | tee -a $(LOG)sign-modem.log
  DEAL_STDOUT_CHECK_MODEM := 2>&1 | tee -a $(LOG)check-modem.log
  DEAL_STDOUT_UPDATE_MD := 2>&1 | tee -a $(LOG)update-modem.log
  DEAL_STDOUT_DUMP_MEMUSAGE := 2>&1 | tee -a $(LOG)dump-memusage.log
  DEAL_STDOUT_PTGEN := 2>&1 | tee -a $(LOG)ptgen.log
  DEAL_STDOUT_MM := 2>&1 | tee -a $(LOG)mm.log
  DEAL_STDOUT_CUSTREL := 2>&1 | tee -a $(LOG)rel-cust.log
  DEAL_STDOUT_PERSO := 2>&1 | tee -a $(LOG)perso.log
else
  DEAL_STDOUT  := >> $(MODULE_LOG) 2>&1
  DEAL_STDOUT_CODEGEN  := > $(CODEGEN_LOG) 2>&1
  DEAL_STDOUT_BTCODEGEN  := > $(LOG)btcodegen.log 2>&1
  DEAL_STDOUT_CUSTGEN := > $(LOG)custgen.log 2>&1
  DEAL_STDOUT_EMIGEN := > $(LOG)emigen.log 2>&1
  DEAL_STDOUT_NANDGEN := > $(LOG)nandgen.log 2>&1
  DEAL_STDOUT_JAVAOPTGEN := > $(LOG)javaoptgen.log 2>&1
  DEAL_STDOUT_IMEJAVAOPTGEN := > $(LOG)imejavaoptgen.log 2>&1
  DEAL_STDOUT_SIGN_IMAGE := > $(LOG)sign-image.log 2>&1
  DEAL_STDOUT_SIGN_MODEM := > $(LOG)sign-modem.log 2>&1
  DEAL_STDOUT_CHECK_MODEM := > $(LOG)check-modem.log 2>&1
  DEAL_STDOUT_DRVGEN := > $(LOG)drvgen.log 2>&1
  DEAL_STDOUT_UPDATE_MD := > $(LOG)update-modem.log 2>&1
  DEAL_STDOUT_DUMP_MEMUSAGE := > $(LOG)dump-memusage.log 2>&1
  DEAL_STDOUT_PTGEN := > $(LOG)ptgen.log 2>&1
  DEAL_STDOUT_MM := > $(LOG)mm.log 2>&1
  DEAL_STDOUT_CUSTREL := > $(LOG)rel-cust.log 2>&1
  DEAL_STDOUT_PERSO := > $(LOG)perso.log 2>&1
endif

ifneq ($(PROJECT),generic)
  MAKECMD    +=  TARGET_PRODUCT=$(PROJECT) GEMINI=$(GEMINI) EVB=$(EVB) FLAVOR=$(FLAVOR)
else
  MAKECMD    +=  TARGET_PRODUCT=generic GEMINI=$(GEMINI) EVB=$(EVB) FLAVOR=$(FLAVOR)
endif

ifeq ($(BUILD_PRELOADER),yes)
  ALL_MODULES += preloader
endif

ifeq ($(BUILD_UBOOT),yes)
  ALL_MODULES += uboot
endif

ifeq ($(BUILD_KERNEL),yes)
  ALL_MODULES += kernel
  KERNEL_ARG = kernel_$(PROJECT).config
endif

ALL_MODULES += android

newall: challmod cleanall javaoptgen emigen nandgen ptgen custgen codegen update-api remakeall

#persoall : custgen remake 


check-dep: custgen
	$(eval include mediatek/build/addon/core/config.mak)
	$(if $(filter error,$(DEP_ERR_CNT)),\
                  $(error Dependency Check FAILED!!))
#	$(hide) echo " Dependency Check Successfully!!"
#	$(hide) echo "*******************************"

challmod:
	$(hide) chmod a+x mediatek/build/android/permfix.sh
	$(hide) mediatek/build/android/permfix.sh $(PROJECT)

new: clean codegen remake

remakeall:

cleanall remakeall:
	$(hide) for i in $(ALL_MODULES); do \
	  $(REMAKECMD) CUR_MODULE=$$i $(subst all,,$@); \
 	  done


ANDROID_NATIVE_TARGETS := \
         update-api \
         cts sdk otapackage banyan_addon \
         snod bootimage systemimage recoveryimage secroimage \
         factoryimage userdataimage dist
.PHONY: $(ANDROID_NATIVE_TARGETS)

systemimage: check-modem

$(ANDROID_NATIVE_TARGETS):
	$(hide) \
        $(if $(filter update-api,$@),\
          $(if $(filter true,$(strip $(BUILD_TINY_ANDROID))), \
            echo SKIP $@... \
            , \
            /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT) && \
            $(REMAKECMD) ACTION=$@ CUR_MODULE=$@ android \
           ) \
          , \
          /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT) && \
          $(if $(filter banyan_addon,$@), \
            $(REMAKECMD) ACTION=sdk_addon CUR_MODULE=sdk_addon android \
            , \
            $(REMAKECMD) ACTION=$@ CUR_MODULE=$@ android \
           ) \
         )

update-api: javaoptgen
banyan_addon: javaoptgen

ifeq ($(TARGET_PRODUCT),emulator)
   TARGET_PRODUCT := generic
endif

.PHONY: mm
mm:
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) (source build/envsetup.sh;cd $(MM_PATH);TARGET_PRODUCT=$(TARGET_PRODUCT) FLAVOR=$(FLAVOR) mm $(SNOD) $(DEAL_STDOUT_MM)) && \
          $(SHOWRSLT) $$? $(LOG)$@.log || \
          $(SHOWRSLT) $$? $(LOG)$@.log

.PHONY:rel-cust
ifeq ($(DUMP),true)
rel-cust: dump_option := -d
endif
rel-cust: 
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) python mediatek/build/tools/customRelease.py $(dump_option) ./ $(RELEASE_DEST) $(TARGET_PRODUCT) $(MTK_RELEASE_PACKAGE).xml $(DEAL_STDOUT_CUSTREL) && \
         $(SHOWRSLT) $$? $(LOG)$@.log || \
         $(SHOWRSLT) $$? $(LOG)$@.log

clean:
	$(hide) $(REMAKECMD) ACTION=$@ $(CUR_MODULE)

remake: 
	$(hide) /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT)
	$(hide) $(REMAKECMD) ACTION= $(CUR_MODULE)


update-modem: custgen check-modem
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) rm -rf mediatek/custom/out/
	$(hide) rm -rf mediatek/config/out/
	$(hide) ./makeMtk $(FULL_PROJECT) mm build/target/board/ snod $(DEAL_STDOUT_UPDATE_MD) && \
         $(SHOWRSLT) $$? $(LOG)$@.log || \
         $(SHOWRSLT) $$? $(LOG)$@.log
perso:  logo javaoptgen
	$(hide) /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT)
	$(hide) $(MAKEPERSO)

drvgen:
ifneq ($(PROJECT),generic)
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) mediatek/source/dct/DrvGen mediatek/custom/$(PROJECT)/kernel/dct/dct/codegen.dws $(DEAL_STDOUT_DRVGEN) && \
	 $(SHOWRSLT) $$? $(LOG)$@.log || \
	 $(SHOWRSLT) $$? $(LOG)$@.log
endif

codegen: drvgen btcodegen
ifneq ($(PROJECT),generic)
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_CODEGEN_LOG)
	$(hide) perl mediatek/build/tools/codegen.pl $(LOGDIR) $(DEAL_STDOUT_CODEGEN) && \
                $(SHOWRSLT) $$? $(CODEGEN_LOG) || \
                $(SHOWRSLT) $$? $(CODEGEN_LOG)
endif

ifneq ($(PROJECT),generic)
  ifeq ($(MTK_BT_SUPPORT), yes)
btcodegen: BT_DB_AUTO_GEN_SCRIPTS_ENTRY := mediatek/build/tools/BTCodegen.pl
btcodegen: BT_DB_AUTO_GEN_SCRIPTS_PATH := mediatek/source/external/bluetooth/blueangel/_bt_scripts
btcodegen: CGEN_EXECUTABLE := mediatek/source/cgen/Cgen
btcodegen: CGEN_HOST_CFG := mediatek/source/cgen/cgencfg/pc_cnf
btcodegen: CGEN_TARGET_CFG := mediatek/source/cgen/cgencfg/tgt_cnf
btcodegen:
    # Todo: use partial source flag to wrap here
    ifneq ($(wildcard mediatek/source/external/bluetooth/blueangel/_bt_scripts/BTCodegen.pl),)
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl $(BT_DB_AUTO_GEN_SCRIPTS_ENTRY) \
                     $(BT_DB_AUTO_GEN_SCRIPTS_PATH) \
                     $(CGEN_EXECUTABLE) \
                     $(CGEN_HOST_CFG) \
                     $(CGEN_TARGET_CFG) \
                     $(PROJECT) $(DEAL_STDOUT_BTCODEGEN) && \
                $(SHOWRSLT) $$? $(LOG)$@.log || \
                $(SHOWRSLT) $$? $(LOG)$@.log
    else # partial source building
btcodegen: ;
    endif
  else
btcodegen:
	$(hide) echo BT database auto-gen process disabled due to Bluetooth is turned off.

  endif
else
btcodegen: ;
endif

custgen:
	$(hide) echo $(SHOWTIME) $@ing...
	$(hide) make -f mediatek/build/custgen.mk $(DEAL_STDOUT_CUSTGEN) && \
                $(SHOWRSLT) $$? $(LOG)$@.log || $(SHOWRSLT) $$? $(LOG)$@.log
#	$(hide) echo $(SHOWTIME) $@ing ...
#	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
#	$(hide) perl mediatek/build/tools/mtkCustom.pl $(PRJ_MF) $(DEAL_STDOUT_CUSTGEN) && \
#	  $(SHOWRSLT) $$? $(LOG)$@.log || \
#	  $(SHOWRSLT) $$? $(LOG)$@.log

javaoptgen:
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl mediatek/build/tools/javaoptgen.pl $(PRJ_MF) $(OPTR_MF) $(DEAL_STDOUT_JAVAOPTGEN) && \
	perl mediatek/build/tools/gen_java_ime_definition.pl $(PRJ_MF) $(DEAL_STDOUT_IMEJAVAOPTGEN) && \
	  $(SHOWRSLT) $$? $(LOG)$@.log || \
          $(SHOWRSLT) $$? $(LOG)$@.log

sign-image:
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl mediatek/build/tools/SignTool/SignTool.pl $(PROJECT)  $(DEAL_STDOUT_SIGN_IMAGE) && \
	  $(SHOWRSLT) $$? $(LOG)$@.log || \
          $(SHOWRSLT) $$? $(LOG)$@.log

ifeq ($(filter generic banyan_addon,$(PROJECT)),)
sign-modem: custgen
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl mediatek/build/tools/sign_modem.pl \
                     $(FULL_PROJECT) \
                     $(MTK_SEC_MODEM_ENCODE) \
                     $(MTK_SEC_MODEM_AUTH) \
                     $(PROJECT) \
                     $(DEAL_STDOUT_SIGN_MODEM) && \
                $(SHOWRSLT) $$? $(LOG)$@.log || \
                $(SHOWRSLT) $$? $(LOG)$@.log
else # TARGET no modem
sign-modem: ;
endif

ifeq ($(filter generic banyan_addon,$(PROJECT)),)
check-modem: PRIVATE_CHK_MD_TOOL := mediatek/build/tools/checkMD.pl
check-modem: PRIVATE_DSP_BL := \
             $(if $(wildcard $(MTK_ROOT_CUSTOM)/$(PROJECT)/modem/$(strip $(CUSTOM_MODEM))/DSP_BL), \
               $(MTK_ROOT_CUSTOM)/$(PROJECT)/modem/$(strip $(CUSTOM_MODEM))/DSP_BL, \
               $(MTK_ROOT_CUSTOM)/common/modem/$(strip $(CUSTOM_MODEM))/DSP_BL \
              )
check-modem: PRIVATE_DSP_ROM := \
             $(if $(wildcard $(MTK_ROOT_CUSTOM)/$(PROJECT)/modem/$(strip $(CUSTOM_MODEM))/DSP_ROM), \
               $(MTK_ROOT_CUSTOM)/$(PROJECT)/modem/$(strip $(CUSTOM_MODEM))/DSP_ROM, \
               $(MTK_ROOT_CUSTOM)/common/modem/$(strip $(CUSTOM_MODEM))/DSP_ROM \
              )
check-modem:
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl $(PRIVATE_CHK_MD_TOOL) \
                     $(PRIVATE_DSP_BL) \
                     $(PRIVATE_DSP_ROM) \
                     $(strip $(MTK_MODEM_SUPPORT)) \
                     $(DEAL_STDOUT_CHECK_MODEM) && \
          $(SHOWRSLT) $$? $(LOG)$@.log || \
          $(SHOWRSLT) $$? $(LOG)$@.log
else # TARGET no modem
check-modem: ;
endif

emigen:
ifneq ($(PROJECT), generic)
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/emigen.pl $(CUSTOM_MEMORY_HDR) \
                     $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(PROJECT) $(DEAL_STDOUT_EMIGEN) && \
                $(SHOWRSLT) $$? $(LOG)$@.log || \
                $(SHOWRSLT) $$? $(LOG)$@.log
endif

nandgen:
ifneq ($(PROJECT), generic)
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/nandgen.pl \
                     $(CUSTOM_NAND_HDR) \
                     $(MEMORY_DEVICE_XLS) \
                     $(MTK_PLATFORM) \
                     $(PROJECT) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT_NANDGEN) && \
                $(SHOWRSLT) $$? $(LOG)$@.log || \
                $(SHOWRSLT) $$? $(LOG)$@.log
endif

dump-memusage: MEM_USAGE_LABEL := $(if $(LABEL),$(LABEL),$(shell date +%Y-%m-%d_%H:%M:%S))
dump-memusage: MEM_USAGE_GENERATOR := mediatek/build/tools/memmon/rommon.pl
dump-memusage: PRIVATE_PROJECT := $(if $(filter emulator, $(PROJECT)),generic,$(PROJECT))
dump-memusage: MEM_USAGE_DATA_LOCATION := mediatek/build/tools/memmon/data
dump-memusage: IMAGE_LOCATION := out/target/product/$(PRIVATE_PROJECT)
dump-memusage:
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl $(MEM_USAGE_GENERATOR) \
                     $(MEM_USAGE_LABEL) \
                     $(PRIVATE_PROJECT) \
                     $(FLAVOR) \
                     $(MEM_USAGE_DATA_LOCATION) \
                     $(IMAGE_LOCATION) \
                     $(DEAL_STDOUT_DUMP_MEMUSAGE) && \
                $(SHOWRSLT) $$? $(LOG)$@.log || \
                $(SHOWRSLT) $$? $(LOG)$@.log

ptgen:
ifneq ($(PROJECT), generic)
	$(hide) echo $(SHOWTIME) $@ing ...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_LOG)$@.log
	$(hide) perl $(MEM_PARTITION_GENERATOR) \
                     $(MTK_PLATFORM) \
                     $(MTK_LCA_SUPPORT) \
                     $(MEM_PARTITION_TABLE) \
                     $(PARTITION_HEADER_LOCATION) \
                     $(dir $(SCATTER_FILE)) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT_PTGEN) && \
                $(SHOWRSLT) $$? $(LOG)$@.log || \
                $(SHOWRSLT) $$? $(LOG)$@.log

	$(hide) perl $(OTA_SCATTER_GENERATOR) $(SCATTER_FILE) $(OTA_SCATTER_FILE)
endif

gen-relkey: PRIVATE_KEY_GENERATOR := development/tools/make_key
gen-relkey: PRIVATE_KEY_LOCATION := build/target/product/security/$(TARGET_PRODUCT)
gen-relkey: PRIVATE_KEY_LIST := releasekey media shared platform
gen-relkey: PRIVATE_SIGNATURE_SUBJECT := $(strip $(SIGNATURE_SUBJECT))
gen-relkey:
	$(hide) echo "Generating release key/certificate..."
	$(hide) if [ ! -d $(PRIVATE_KEY_LOCATION) ]; then \
                  mkdir $(PRIVATE_KEY_LOCATION); \
                fi
	$(hide) for key in $(PRIVATE_KEY_LIST); do \
                  $(PRIVATE_KEY_GENERATOR) $(strip $(PRIVATE_KEY_LOCATION))/$$key '$(PRIVATE_SIGNATURE_SUBJECT)' < /dev/null; \
                done

preloader:
ifeq ($(BUILD_PRELOADER),yes)
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
ifneq ($(ACTION), )
	$(hide) cd $(PRELOADER_WD) && \
	  (make clean $(DEAL_STDOUT) && \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION)) && cd $(MKTOPDIR)
else
ifeq ($(MTK_PLATFORM), MT6516)
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/emigen.pl $(CUSTOM_MEMORY_HDR) \
                $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(PROJECT) $(DEAL_STDOUT)
endif
ifeq ($(MTK_PLATFORM), MT6573)
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/emigen.pl $(CUSTOM_MEMORY_HDR) \
                $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(PROJECT) $(DEAL_STDOUT)
endif
ifeq ($(MTK_PLATFORM), MT6573)
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/nandgen.pl \
                     $(CUSTOM_NAND_HDR) \
                     $(MEMORY_DEVICE_XLS) \
                     $(MTK_PLATFORM) \
                     $(PROJECT) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT)
endif
	$(hide) perl $(MEM_PARTITION_GENERATOR) \
                     $(MTK_PLATFORM) \
                     $(MTK_LCA_SUPPORT) \
                     $(MEM_PARTITION_TABLE) \
                     $(PARTITION_HEADER_LOCATION) \
                     $(dir $(SCATTER_FILE)) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT)

	$(hide) perl $(OTA_SCATTER_GENERATOR) $(SCATTER_FILE) $(OTA_SCATTER_FILE)

	$(hide) cd $(PRELOADER_WD) && \
	  (./build.sh $(PROJECT) $(ACTION) $(DEAL_STDOUT) && \
	  cd $(MKTOPDIR) && \
          $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(PRELOADER_IMAGES),$(DEAL_STDOUT),&&) \
	  $(SHOWRSLT) $$? $(MODULE_LOG) || \
	  $(SHOWRSLT) $$? $(MODULE_LOG))
endif
else
	$(hide) echo Not support $@.
endif

logo:
	$(hide) cd $(JRD_DEVICE) 
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@..
	$(hide) chmod a+x $(JRD_DEVICE)/jrd_build_logo.sh
	$(hide) $(JRD_DEVICE)/jrd_build_logo.sh
	cd $(MKTOPDIR)

uboot:
ifeq ($(BUILD_UBOOT),yes)
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
ifneq ($(ACTION), )
	$(hide) cd $(UBOOT_WD) && \
	  (make distclean $(DEAL_STDOUT) && \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION)) && cd $(MKTOPDIR)
else
ifeq ($(MTK_PLATFORM), MT6516)
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/emigen.pl $(CUSTOM_MEMORY_HDR) \
                $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(PROJECT) $(DEAL_STDOUT)
endif
ifeq ($(MTK_PLATFORM), MT6573)
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/emigen.pl $(CUSTOM_MEMORY_HDR) \
                $(MEMORY_DEVICE_XLS) $(MTK_PLATFORM) $(PROJECT) $(DEAL_STDOUT)
endif
ifeq ($(MTK_PLATFORM), MT6573)
	$(hide) perl mediatek/build/tools/emigen/$(MTK_PLATFORM)/nandgen.pl \
                     $(CUSTOM_NAND_HDR) \
                     $(MEMORY_DEVICE_XLS) \
                     $(MTK_PLATFORM) \
                     $(PROJECT) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT)
endif
	$(hide) perl $(MEM_PARTITION_GENERATOR) \
                     $(MTK_PLATFORM) \
                     $(MTK_LCA_SUPPORT) \
                     $(MEM_PARTITION_TABLE) \
                     $(PARTITION_HEADER_LOCATION) \
                     $(dir $(SCATTER_FILE)) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT_PTGEN)

	$(hide) perl $(OTA_SCATTER_GENERATOR) $(SCATTER_FILE) $(OTA_SCATTER_FILE)

	$(hide) cd $(UBOOT_WD) && \
	  (MAKEJOBS=$(MAKEJOBS) ./build.sh $(ACTION) $(DEAL_STDOUT) && \
	  cd $(MKTOPDIR) && \
	   $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(UBOOT_IMAGES),$(DEAL_STDOUT) &&) \
	  $(SHOWRSLT) $$? $(MODULE_LOG) || \
	  $(SHOWRSLT) $$? $(MODULE_LOG)) 
endif
else
	$(hide) echo Not support $@.
endif

kernel:
ifeq ($(BUILD_KERNEL),yes)
  ifeq ($(ACTION),)
	$(hide) perl $(MEM_PARTITION_GENERATOR) \
                     $(MTK_PLATFORM) \
                     $(MTK_LCA_SUPPORT) \
                     $(MEM_PARTITION_TABLE) \
                     $(PARTITION_HEADER_LOCATION) \
                     $(dir $(SCATTER_FILE)) \
                     $(MTK_NAND_PAGE_SIZE) \
                     $(DEAL_STDOUT)

	$(hide) perl $(OTA_SCATTER_GENERATOR) $(SCATTER_FILE) $(OTA_SCATTER_FILE)
  endif

  ifneq ($(KMOD_PATH),)
	$(hide)	echo building kernel module KMOD_PATH=$(KMOD_PATH)
	$(hide) cd $(KERNEL_WD) && \
	(KMOD_PATH=$(KMOD_PATH) ./build.sh $(ACTION) $(KERNEL_ARG) ) && cd $(MKTOPDIR)
  else
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
	$(hide) cd $(KERNEL_WD) && \
	  (MAKEJOBS=$(MAKEJOBS) ./build.sh $(ACTION) $(PROJECT) $(DEAL_STDOUT) && \
	   cd $(MKTOPDIR) && \
	   $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(if $(strip $(ACTION)),,$(KERNEL_IMAGES)),$(DEAL_STDOUT),&&) \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION))
#	$(hide) $(SHOWTIMECMD) 
  endif
else
	$(hide) echo Not support $@.
endif

ifneq ($(ACTION),clean)
android: check-modem sign-modem
endif
android: CHECK_IMAGE := $(filter %/$(patsubst %image,%.img,$(ACTION)),$(ANDROID_IMAGES))
android:
ifeq ($(ACTION), )
	$(hide) /usr/bin/perl mediatek/build/tools/mtkBegin.pl $(PROJECT)
endif

ifneq ($(DR_MODULE),)
   ifneq ($(ACTION), clean)
	$(hide) echo building android module MODULE=$(DR_MODULE)
	$(hide) perl mediatek/build/tools/javaoptgen.pl $(PRJ_MF) $(OPTR_MF)
	$(MAKECMD) $(DR_MODULE)
   else
	$(hide) echo cleaning android module MODULE=$(DR_MODULE)
	$(hide) $(MAKECMD) clean-$(DR_MODULE) 
   endif
else 
	$(hide) echo $(SHOWTIME) $(SHOWBUILD)ing $@...
	$(hide) echo -e \\t\\t\\t\\b\\b\\b\\bLOG: $(S_MODULE_LOG)
ifeq ($(SHOWBUILD), build)
	$(hide) perl mediatek/build/tools/javaoptgen.pl $(PRJ_MF) $(OPTR_MF) $(DEAL_STDOUT)
endif
	$(hide) $(MAKECMD) $(ACTION) $(DEAL_STDOUT) && \
	  $(call chkImgSize,$(ACTION),$(PROJECT),$(SCATTER_FILE),$(if $(strip $(ACTION)),$(CHECK_IMAGE),$(ANDROID_IMAGES)),$(DEAL_STDOUT),&&) \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION) || \
	  $(SHOWRSLT) $$? $(MODULE_LOG) $(ACTION)

ifeq ($(ACTION), )
	$(hide) /usr/bin/perl mediatek/build/tools/mtkFinalize.pl $(PROJECT) $(MTK_PLATFORM)
endif
endif
st:
	
##############################################################
# function:  chkImgSize
# arguments: $(ACTION) $(PROJECT) $(SCATTER_FILE) $(IMAGES) $(DEAL_STDOUT) &&
#############################################################
define chkImgSize
$(if $(strip $(1)), \
     $(if $(strip $(4)), \
          $(if $(filter generic, $(2)),, \
               perl mediatek/build/tools/chkImgSize.pl $(3) $(2) $(4) $(5) $(6) \
           ) \
      ), \
     $(if $(filter generic, $(2)),, \
         perl mediatek/build/tools/chkImgSize.pl $(3) $(2) $(4) $(5) $(6) \
      ) \
 )
endef


#Tool chain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as

#Flags
Modules = app 											\
		  bsw/communication/canif 						\
		  bsw/communication/com 						\
		  bsw/communication/pdur 						\
		  bsw/ecua/iohwab/src 							\
		  bsw/mcal/adc									\
		  bsw/mcal/can									\
		  bsw/mcal/dio									\
		  bsw/mcal/port									\
		  bsw/mcal/pwm									\
		  bsw/services/bswm								\
		  bsw/services/ecum								\
		  bsw/services/os/src							\
		  bsw/services/os/arch/cortexm3_stm32f1 		\
		  cfg/mcal										\
		  platform/bsp/device							\
		  rte/core/src			 						\
		  swc/Swc_BrakeAcq								\
		  swc/Swc_CmdComposer							\
		  swc/Swc_DriveModeMgr							\
		  swc/Swc_GearSelector							\
		  swc/Swc_PedalAcq								\
		  swc/Swc_SafetyManager							

Includes =  -Ibsw/ecua/iohwab/inc 						\
			-Ibsw/services/os/inc 						\
			-Icfg/communication			 				\
			-Icfg/ecua					 				\
			-Iplatform/autosar_base		 				\
			-Iplatform/board 							\
			-Iplatform/bsp/cmsis 						\
			-Irte/core/inc 								\
			-Irte/swc_if 								
CFLAGS = -mcpu=cortex-m3 -mthumb -std=c11 -g -Wall -O0 -ffunction-sections -fdata-sections	\
		$(Includes)																			\
		$(addprefix -I, $(Modules))															\
		-DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
LDFLAGS = -Tplatform/bsp/ldscripts/stm32f103c8_flash.ld -Wl,--gc-sections -nostartfiles
LIBS = -lc -lm 

#Source Files
SRC = $(foreach module, $(Modules), $(wildcard $(module)/*.c))
STARTUP = platform/bsp/startup_stm32f10x_md.s bsw/services/os/arch/cortexm3_stm32f1/Os_Arch_Asm.s
OBJ = $(SRC:.c=.o) $(STARTUP:.s=.o)
OUT = BUILD/Project.elf

#RULES
build: $(OUT)
$(OUT): $(OBJ)
	@mkdir -p BUILD
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS)
clean:
	rm -rf BUILD platform/bsp/startup_stm32f10x_md.o
	rm -rf $(foreach module, $(Modules), $(wildcard $(module)/*.o)) 
flash:	$(OUT)
		openocd -f interface/stlink.cfg -f target/stm32f1x.cfg -c "program $(OUT) verify reset exit"

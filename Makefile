include libs/nanopb/extra/nanopb.mk

PROJ_NAME=udp_adc
ST_FLASH=st-flash
# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

###################################################



vpath %.c src


vpath %.c libs/CMSIS/DSP_Lib/Source/CommonTables
vpath %.c libs/CMSIS/DSP_Lib/Source/TransformFunctions
vpath %.S libs/CMSIS/DSP_Lib/Source/TransformFunctions
vpath %.c libs/CMSIS/DSP_Lib/Source/ComplexMathFunctions
vpath %.c libs/CMSIS/DSP_Lib/Source/StatisticsFunctions



vpath %.c libs/nanopb
vpath %.c libs/syscalls
vpath %.c boot
vpath %.s boot


vpath %.c libs/STM32F4xx_StdPeriph_Driver/src
vpath %.c libs/Ethernet/source
vpath %.c libs/STM32F4x7_ETH_Driver
vpath %.c libs/STM32F4x7_ETH_Driver/src
vpath %.c libs/STM32F4x7_ETH_Driver/src/netif
vpath %.c libs/STM32F4x7_ETH_Driver/src/netif/ppp
vpath %.c libs/STM32F4x7_ETH_Driver/src/core
vpath %.c libs/STM32F4x7_ETH_Driver/src/core/ipv4
vpath %.c libs/STM32F4x7_ETH_Driver/src/core/snmp
vpath %.c libs/STM32F4x7_ETH_Driver/src/api

ARM_GCC_LINK_DIR=linker
ARM_GCC_LD=arm-gcc-link.ld
OPT_LEVEL = 3
PBUF_NAME = message
OBJDIR = build/
LIBDIR = libs
ARM_MATH_LIB = arm_cortexM4lf_math
ROOT=$(shell pwd)

CFLAGS += -mcpu=cortex-m4 -mthumb -mthumb-interwork -mlittle-endian -Wall -w -ffunction-sections -g -O$(OPT_LEVEL) -c -DSTM32F407VG -DSTM32F4XX -DUSE_STDPERIPH_DRIVER -D__FPU_USED -DHSE_VALUE=8000000
CFLAGS += -I. -Iinc -Ilibs/cmsis/cmsis_boot -Ilibs/STM32F4x7_ETH_Driver -Ilibs/STM32F4x7_ETH_Driver/inc/lwip  -Ilibs/Ethernet/include -Ilibs/STM32F4x7_ETH_Driver/inc/lwip/arch
CFLAGS += -Ilibs/STM32F4x7_ETH_Driver/src/netif -Ilibs/STM32F4x7_ETH_Driver/inc -Ilibs/STM32F4xx_StdPeriph_Driver/inc -Ilibs/STM32F4x7_ETH_Driver/src/netif/ppp 
CFLAGS += -Ilibs/Ethernet -Ilibs/STM32F4x7_ETH_Driver/src -Ilibs/STM32F4x7_ETH_Driver/inc/netif -Ilibs/nanopb
CFLAGS += -Ilibs/CMSIS/Include -Ilibs/CMSIS/Device/ST/STM32F4xx/Include

#HARD FLOAT STUFF
FLOAT_ABI=hard
CFLAGS += -mfloat-abi=$(FLOAT_ABI) -mfpu=fpv4-sp-d16
CFLAGS += -fsingle-precision-constant -Wdouble-promotion
CFLAGS += -DARM_MATH_CM4 -D__FPU_PRESENT -D__USE_CMSIS

#SRCS += lib/startup_stm32f4xx.s # add startup file to build

ELFFLAGS = -mcpu=cortex-m4 -mthumb -mthumb-interwork -mlittle-endian -mfloat-abi=$(FLOAT_ABI) -mfpu=fpv4-sp-d16  -g -nostartfiles --specs=rdimon.specs -Wl,-Map=$(PROJ_NAME).map -O$(OPT_LEVEL) -Wl,--gc-sections #-Wl,--start-group -lgcc -lc -lm -lrdimon -L$(LIBDIR) -l$(ARM_MATH_LIB) -Wl,--end-group
#ELFFLAGS = -mcpu=cortex-m4 -mthumb -g -nostartfiles -Wl,-Map=$(PROJ_NAME).map,-O$(OPT_LEVEL),--gc-sections,
SRCS =  stm32f4xx_syscfg.c mem.c tcp.c err.c randm.c mib_structs.c tcp_in.c stm32f4xx_usart.c slipif.c memp.c autoip.c
SRCS += ip_frag.c msg_out.c netbuf.c tcpip.c stm32f4xx_dac.c asn1_dec.c lcp.c vj.c stm32f4x7_eth.c
SRCS += sys.c netconf.c mib2.c stm32f4xx_it.c netdb.c init.c stm32f4xx_adc.c stm32f4x7_eth_bsp.c  ethernetif.c chpms.c etharp.c 
SRCS += ip_addr.c  magic.c  pbuf.c stats.c stm32f4xx_rcc.c inet_chksum.c
SRCS += inet.c msg_in.c netif.c asn1_enc.c fsm.c api_msg.c chap.c stm32f4xx_gpio.c md5.c system_stm32f4xx.c syscalls.c sockets.c main.c
#SRCS += stm32f4_discovery.c
##Second cmd
SRCS += igmp.c pap.c udp.c ip.c auth.c stm32f4xx_exti.c ppp_oe.c icmp.c dns.c netifapi.c misc.c api_lib.c dhcp.c ppp.c  ipcp.c tcp_out.c loopif.c raw.c
SRCS += stm32f4xx_tim.c
SRCS += stm32f4xx_dma.c udp_echoserver.c
SRCS += pb_encode.c pb_decode.c pb_common.c udp_echoserver.c
SRCS += $(PBUF_NAME).pb.c

#SRCS += startup_stm32f40xx.s
SRCS += startup_stm32f4xx.c

SRCS += dsp.c 
MATH_SRCS = arm_common_tables.c arm_bitreversal.c arm_cfft_f32.c arm_cfft_radix8_f32.c arm_bitreversal2.S
MATH_SRCS += arm_cmplx_mag_f32.c arm_max_f32.c arm_rfft_fast_init_f32.c arm_rfft_fast_f32.c
SRCS += $(MATH_SRCS)
OBJS = $(patsubst %.c,$(OBJDIR)%.o,$(SRCS))


###################################################
.PHONY: proj

all: proj
$(OBJS):$(OBJDIR)%.o : %.c
#	$(CC) $(CFLAGS) -L$(LIBDIR) -l$(ARM_MATH_LIB) -o $@ $^ 
	$(CC) $(CFLAGS) -o $@ $^ 

udp_echoserver.o : $(PBUF_NAME).pb.c

$(PBUF_NAME).pb.c: $(PBUF_NAME).proto
	cd libs/nanopb/generator/proto/ && make -f Makefile
	$(PROTOC) $(PROTOC_OPTS) --nanopb_out=. --python_out=. $(PBUF_NAME).proto

flash: $(PROJ_NAME).elf
	$(ST_FLASH) write $(PROJ_NAME).bin 0x8000000
proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(OBJS)
#	$(CC) $(ELFFLAGS) -L$(ARM_GCC_LINK_DIR) -Wl,-T$(ARM_GCC_LINK_DIR)/$(ARM_GCC_LD) -g -o $@  $^ -Wl,--start-group -lgcc -lc -lm -lrdimon -L$(LIBDIR) -l$(ARM_MATH_LIB) -Wl,--end-group
	$(CC) $(ELFFLAGS) -L$(ARM_GCC_LINK_DIR) -Wl,-T$(ARM_GCC_LINK_DIR)/$(ARM_GCC_LD) -g -o $@  $^ -Wl,--start-group -lgcc -lc -lm -lrdimon -Wl,--end-group
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(PBUF_NAME).pb.c
	rm -f $(PBUF_NAME).pb.h
	rm -f $(PBUF_NAME)_pb2.py
	cd libs/nanopb/generator/proto/ && make -f Makefile clean
	rm -f $(OBJS)

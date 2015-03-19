# put your *.o targets here, make should handle the rest!

# all the files will be generated with this name (main.elf, main.bin, main.hex, etc)

PROJ_NAME=udp_adc
ST_FLASH=st-flash
# that's it, no need to change anything below this line!

###################################################

CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy

#CFLAGS  = -g -O0 -Wall -Tstm32_flash.ld 
#CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
#CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

###################################################
#vpath %.c STM32F4x7_ETH_Driver/src
#vpath %.c lib/STM32F4x7_ETH_Driver/src
#vpath %.c Utilities/STM32F4-Discovery

#vpath %.c src
#vpath %.a lib
#vpath %.a lwip

vpath %.c src
vpath %.c libs/stdio
vpath %.c libs/syscalls
vpath %.c libs/cmsis/cmsis_boot
vpath %.c libs/cmsis/cmsis_boot/startup
vpath %.c libs/cmsis/cmsis_lib/source
vpath %.c libs/Ethernet/source
vpath %.c libs/STM32F4x7_ETH_Driver
vpath %.c libs/STM32F4x7_ETH_Driver/src
vpath %.c libs/STM32F4x7_ETH_Driver/src/netif
vpath %.c libs/STM32F4x7_ETH_Driver/src/netif/ppp
vpath %.c libs/STM32F4x7_ETH_Driver/src/core
vpath %.c libs/STM32F4x7_ETH_Driver/src/core/ipv4
vpath %.c libs/STM32F4x7_ETH_Driver/src/core/snmp
vpath %.c libs/STM32F4x7_ETH_Driver/src/api

ARM_GCC_LINK_DIR = mystery_ld
ARM_GCC_LD = arm-gcc-link.ld
OBJDIR = build/
ROOT=$(shell pwd)
CFLAGS = -mcpu=cortex-m4 -mthumb -Wall -w -ffunction-sections -g -O0 -c -DSTM32F407VG -DSTM32F4XX -DUSE_STDPERIPH_DRIVER -D__FPU_USED -DHSE_VALUE=8000000

CFLAGS += -Iinc -Ilibs/cmsis/cmsis_boot -Ilibs/STM32F4x7_ETH_Driver -Ilibs/STM32F4x7_ETH_Driver/inc/lwip -Ilibs/cmsis/cmsis_lib -Ilibs/cmsis -Ilibs/Ethernet/include -Ilibs/STM32F4x7_ETH_Driver/inc/lwip/arch
CFLAGS += -Ilibs/STM32F4x7_ETH_Driver/src/netif -Ilibs/STM32F4x7_ETH_Driver/inc -Ilibs/cmsis/cmsis_lib/include -Ilibs/STM32F4x7_ETH_Driver/src/netif/ppp 
CFLAGS += -Ilibs/Ethernet -Ilibs/STM32F4x7_ETH_Driver/src -Ilibs/STM32F4x7_ETH_Driver/inc/netif
#SRCS += lib/startup_stm32f4xx.s # add startup file to build

ELFFLAGS = -mcpu=cortex-m4 -mthumb -g -nostartfiles -Wl,-Map=$(PROJ_NAME).map -O0 -Wl,--gc-sections
SRCS = startup_stm32f4xx.c stm32f4xx_syscfg.c mem.c tcp.c err.c randm.c mib_structs.c tcp_in.c stm32f4xx_usart.c slipif.c memp.c autoip.c
SRCS += ip_frag.c msg_out.c netbuf.c tcpip.c stm32f4xx_dac.c asn1_dec.c lcp.c vj.c fs.c  stm32f4x7_eth.c
SRCS += sys.c netconf.c mib2.c stm32f4xx_it.c netdb.c init.c stm32f4xx_adc.c stm32f4x7_eth_bsp.c  ethernetif.c chpms.c etharp.c 
SRCS += ip_addr.c httpd.c magic.c httpd_cgi_ssi.c pbuf.c stats.c stm32f4xx_rcc.c inet_chksum.c
SRCS += inet.c msg_in.c netif.c asn1_enc.c fsm.c api_msg.c chap.c stm32f4xx_gpio.c md5.c system_stm32f4xx.c syscalls.c sockets.c main.c
#SRCS += stm32f4_discovery.c
##Second cmd
SRCS += igmp.c pap.c udp.c ip.c auth.c stm32f4xx_exti.c ppp_oe.c icmp.c dns.c netifapi.c misc.c api_lib.c dhcp.c ppp.c  ipcp.c tcp_out.c loopif.c raw.c
SRCS += stm32f4xx_tim.c
SRCS += stm32f4xx_dma.c
OBJS = $(patsubst %.c,$(OBJDIR)%.o,$(SRCS))

###################################################
#--specs=rdimon.specs -lgcc -lc -lm -lrdimon
.PHONY: proj

all: proj
$(OBJS):$(OBJDIR)%.o : %.c
	$(CC) $(CFLAGS) -o $@ $^


flash: $(PROJ_NAME).elf
	$(ST_FLASH) write $(PROJ_NAME).bin 0x8000000
proj: 	$(PROJ_NAME).elf

$(PROJ_NAME).elf: $(OBJS)
	@echo $(SRCS)
	$(CC) $(ELFFLAGS) -L$(ARM_GCC_LINK_DIR) -Wl,-T$(ARM_GCC_LINK_DIR)/$(ARM_GCC_LD) -g -o $@  $^
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

clean:
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).bin
	rm -f $(PROJ_NAME).map
	rm -f $(OBJS)

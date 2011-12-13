##
## Made by fabien le mentec <texane@gmail.com>
## 
## Started on  Sun Sep 20 00:28:27 2009 texane
## Last update Tue Dec 13 08:31:26 2011 fabien le mentec
##


PATH	:=	$(PATH):$(HOME)/install/bin:$(HOME)/install/share/pk2cmd
SDCC_DIR:=	$(HOME)/install
GPUTILS_DIR:=	$(HOME)/install

RM	:=	-rm
MV	:=	mv

CC	:=	$(SDCC_DIR)/bin/sdcc
CFLAGS	:=	--disable-warning 196				\
		-mpic16						\
		-p18f4550 -Dpic18f4550				\
		-DSDCC_PIC18					\
		-I$(GPUTILS_DIR)/share/gputils/header 		\
		-I$(HOME)/repo/sdcc/device/non-free/include/pic16 \
		-I../src

AS	:=	gpasm
ASFLAGS	:=	-q -p18f4550 -Dpic18f4550 			\
		-I $(GPUTILS_DIR)/share/gputils/header 		\
		-DSDCC

LD	:=	$(CC)
LDFLAGS	:=	--use-crt=crt0.o				\
		-Wl-s../src/template-18f4550.lkr,-m		\
		-mpic16 -p18f4550				\
		-L$(HOME)/repo/sdcc/device/non-free/lib/build/pic16
LIBS	:=	-llibdev18f4550.lib -llibsdcc.lib -llibc18f.lib

LIB	:=	gplib -c

NAME	:=	slosyn.hex

CSRCS	:=	../src/main.c 			\
		../src/slosyn.c			\
		../src/config.c 		\
		../src/ep0.c			\
		../src/ep1.c			\
		../src/ep2.c			\
		../src/usb.c			\
		../src/usb_descriptors.c
ASRCS	:=

OBJS	:=	$(CSRCS:.c=.o)			\
		$(ASRCS:.asm=.o)

GARBAGE	:=	$(NAME:.hex=.lst)		\
		$(NAME:.hex=.cod)		\
		$(CSRCS:.c=.lst)		\
		$(CSRCS:.c=.asm)

# rules

all	:	$(NAME)

$(NAME)	:	$(OBJS)
		$(LD) $(OBJS) -o $(NAME) $(LDFLAGS) $(LIBS)

%.o	:	%.c
		$(CC) $(CFLAGS) -c -o $(@) $(<)

%.o	:	%.asm
		$(AS) $(ASFLAGS) -c -o $(@) $(<)

PHONY	:	clean fclean re write erase test

clean	:
		$(RM) $(OBJS)
		$(RM) $(GARBAGE)

fclean	:	clean
		$(RM) $(NAME)
		$(RM) slosyn.map

re	:	fclean all

write	:	$(NAME)
		pk2cmd -PPIC18f4550 -M -F$(NAME)

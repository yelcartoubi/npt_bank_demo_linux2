CROSS_COMPILE=arm-linux-gnueabi-

CC=$(CROSS_COMPILE)gcc
RM=rm
CP=cp
MKUPT=upt

# Application directory definition
BINDIR = bin
SRCDIR = src
INCDIR = inc
OBJDIR = obj_err
LIBAPPDIR = lib
LIBINCDIR = libapi

# Module directory definition
PRINTINCDIR = module/libprint/inc
PRINTSRCDIR = module/libprint/src
SECURITYIINCDIR = module/libsecurity/inc
SECURITYISRCDIR = module/libsecurity/src
CARDINCDIR = module/libcard/inc
CARDSRCDIR = module/libcard/src
UIINCDIR = module/libui/inc
UISRCDIR = module/libui/src
TOOLINCDIR = module/libtool/inc
TOOLSRCDIR = module/libtool/src
COMMINCDIR = module/libcomm/inc
COMMSRCDIR = module/libcomm/src
TMSINCDIR = module/libtms/inc
TMSSRCDIR = module/libtms/src

# SDK directory definition
SDKDIR = C:\\NPT_SDK
SDKAPIDIR = $(SDKDIR)\\Linux\\Core\\api\\legacy_ndk
SDKAPILIB = $(SDKAPIDIR)\\lib\\gcc_4.9.4

# NAPI
SDKNAPIDIR = $(SDKDIR)\\Linux\\Core\\api\\napi
SDKNAPIINC = $(SDKNAPIDIR)\\inc
SDKNAPILIB = $(SDKNAPIDIR)\\lib\\gcc_4.9.4

# EMV L2
LIBL2EMVDIR = $(SDKDIR)\\Linux\\Core\\EMV\\L2_SDK\\lib\\gcc_4.9.4

# Libc
SDKSYSLIB = $(SDKDIR)\\Linux\\Compiler\\4.9.4\\arm-linux-gnueabi\\libc\\usr\\lib

CFLAGS += -Wall -Werror -DNDEBUG $(CROSS_CFLAGS) -O2 -g $(INCPATH) -D USE_TMS -D DEMO
CFLAGS += -funwind-tables

# File search path
VPATH = src $(PRINTSRCDIR) $(SECURITYISRCDIR) $(CARDSRCDIR) $(UISRCDIR) $(TOOLSRCDIR) $(COMMSRCDIR) $(TMSSRCDIR) $(OBJDIR)

# Header file search path
INCLPATH = -I$(INCDIR) -I$(PRINTINCDIR) -I$(SECURITYIINCDIR) -I$(CARDINCDIR) -I$(UIINCDIR) -I$(TOOLINCDIR) -I$(COMMINCDIR) -I$(TMSINCDIR)  -I$(LIBINCDIR) -I$(SDKNAPIINC)

# Program link parameters
LDFLAGS += -L$(SDKAPILIB) -lndk

LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_app -lnapi_c_sysinfo 
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_datetime -lnapi_c_kb -lnapi_c_power -lnapi_c_beeper
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_eth -lnapi_c_modem -lnapi_c_net -lnapi_c_wifi -lnapi_c_wlm
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_ctlscard -lnapi_c_magcard -lnapi_c_smartcard
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_crypto -lnapi_c_printer -lnapi_c_led -lnapi_c_display -lnapi_c_serialport

LDFLAGS += -L$(SDKSYSLIB) -lssl -lcrypto -lcurl -lpthread -ldl -lz

# EMV L2 Lib
LDFLAGS += -L$(LIBL2EMVDIR) -lemvl2 -lentrypoint -lbase
# App Lib
LDFLAGS += -L$(LIBAPPDIR) -ltms 
LDFLAGS += -L$(LIBAPPDIR) -lemvl3

# model such as SP630
MODEL = SP630
# Application Name
PARAMINI_NAME = BANKDEMO
# Application Version
PARAMINI_VER = 2020102901

# Generated program name
NAME = main
APPNAME = app_$(MODEL)_$(PARAMINI_NAME)_$(PARAMINI_VER).NLD

# Module used in the program
SRCS = $(wildcard $(SRCDIR)/*.c $(PRINTSRCDIR)/*.c $(SECURITYISRCDIR)/*.c $(CARDSRCDIR)/*.c $(UISRCDIR)/*.c $(TOOLSRCDIR)/*.c $(COMMSRCDIR)/*.c $(TMSSRCDIR)/*.c )
SRSS = $(notdir $(SRCS))
OBJS = $(subst .c,.o,$(SRSS))
OBJSD = $(addprefix $(OBJDIR)/,$(SRSS))
OBJSY = $(subst .c,.o,$(OBJSD))

# Include dependency file
all: NLD

$(NAME):config $(OBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(NAME) $(OBJSY) $(LDFLAGS)
	iniupdate Name=$(PARAMINI_NAME) $(BINDIR)/param.ini
	iniupdate Version=$(PARAMINI_VER) $(BINDIR)/param.ini
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLPATH) $< -o $(OBJDIR)/$@
# Automatic generation of dependency files  
config: $(subst .o,.deps, $(OBJS))

%.deps: %.c
	$(CC) -MM $(INCLPATH) $< >$(OBJDIR)/$@ 

.PHONY:clean
clean:
	-$(RM) $(BINDIR)/$(NAME)
	-$(RM) $(OBJDIR)/*.o
	-$(RM) $(OBJDIR)/*.deps
	
NLD:$(NAME)
	$(MKUPT) -h $(BINDIR)/filepath.txt -p $(BINDIR)/param.ini -o $(BINDIR)/$(APPNAME)	

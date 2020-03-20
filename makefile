#Names of programs
MAIN	= nanorc

ABS	= .
BIN	= ~/bin/
BUILD	= $(ABS)/build
RM    = /bin/rm -f
MV		= /bin/mv -f

LFLAGS	= -Wl,-rpath,/usr/bin/g++
LIBDIRS	= $(LFLAGS) -L/usr/local/lib/ -L/usr/lib/boost/stage/lib/ -lstdc++fs
LIBS		= -lboost_program_options -lncurses

INC		= -I /usr/lib/boost/
CFLAGS	= -Wno-deprecated-declarations -fopenmp -std=c++17 -O3 -g
CC			= /usr/bin/g++ $(CFLAGS) $(INC) $(LIBS) 

#Output coloring
GREEN   = \033[1;32m
CYAN    = \033[36m
BLUE    = \033[1;34m
BRIGHT  = \033[1;37m
WHITE   = \033[0;m
MAGENTA = \033[35m
YELLOW  = \033[33m

#Source files
SRCS	= $(ABS)/rcio.c++
OBJS	= $(BUILD)/rcio.o \
		  $(BUILD)/lexer.o \
		  $(BUILD)/rcstreambuf.o \
		  $(BUILD)/nanorc.o \
		  $(BUILD)/lexcontext.o

#Builds
all:
	@printf "[      $(YELLOW)Building $(MAIN)$(WHITE)       ]\n"
	@printf "[$(BLUE)Building$(WHITE)] $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Program$(WHITE)\n"
	make -f makefile nanorc
	@printf "[$(GREEN) Built  $(WHITE)] $(BRIGHT)$(MAIN)$(WHITE) - $(MAGENTA)Program$(WHITE)\n"
	@printf "[        $(GREEN)Build Complete$(WHITE)        ]\n"

$(BUILD)/%.o: %.c++
	@printf "[$(CYAN)Building$(WHITE)]   $(BRIGHT)$<$(WHITE) - $(MAGENTA)Object$(WHITE)\n"
	cd $(ABS); $(CC) -c -o $@ $<
	@printf "[$(GREEN) Built  $(WHITE)]   $(BRIGHT)$<$(WHITE) - $(MAGENTA)Object$(WHITE)\n"

nanorc: $(OBJS)
	cd $(ABS); $(CC) $(OBJS) $(LIBDIRS) -o $(BIN)/$(MAIN) $(LIBS)
	
clean:
	$(RM) *.core $(BUILD)/*.o *.d *.stackdump

#Disable command echoing, reenabled with make verbose=1
ifndef verbose
.SILENT:
endif

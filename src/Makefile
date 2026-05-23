#This is the SmaugFUSS Makefile.
#This should be usable on pretty much any Linux system.
#Refer to your 'man gcc' for explanations of these options
#To compile in FreeBSD or OpenBSD, use the gmake compiler.

#Change this to reflect whichever compiler you want to use.
CC      = g++
#CC      = clang++

# Automatically detect if we are running in a Cygwin environment
ifeq ($(findstring CYGWIN, $(shell uname -s)), CYGWIN)
    IS_CYGWIN = 1
endif

# Check if libdl is available on the system
HAS_DL := $(shell ld -ldl --verbose >/dev/null 2>&1 && echo yes)

ifeq ($(HAS_DL),yes)
    NEED_DL = -ldl
else
    NEED_DL =
endif

#OS Detection. Used to check for MacOS below.
UNAME_S := $(shell uname -s)

# Set EXPORT_SYMBOLS only if not on Darwin (MacOS)
ifneq ($(UNAME_S), Darwin)
    EXPORT_SYMBOLS = -export-dynamic
endif

#GCC 4.8+ memory checking option. https://github.com/google/sanitizers Does not work in Cygwin.
#These should only be enabled for testing environments. Disable for a live environment.
ifndef IS_CYGWIN
   M_FLAGS = -fsanitize=address -fsanitize=leak -fno-omit-frame-pointer
endif

# These protect against stack smashing and buffer overflows in production.
# We add -D_FORTIFY_SOURCE=2 to perform runtime checks on string/memory functions.
HARDENING_FLAGS = -fstack-protector-strong -D_FORTIFY_SOURCE=2

W_FLAGS = -std=c++20 -Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls

C_FLAGS = -g2 $(W_FLAGS) $(M_FLAGS) $(HARDENING_FLAGS) $(EXPORT_SYMBOLS)
L_FLAGS = $(M_FLAGS) -lz $(NEED_DL)

#D_FLAGS : For the DNS Slave process. No need in linking all the extra libs for this.
D_FLAGS = -g2 -O $(M_FLAGS)

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c ban.c boards.c \
          build.c calendar.c chess.c clans.c color.c comm.c comments.c const.c db.c deity.c \
          dns.c fight.c handler.c hashstr.c hint.c hotboot.c house.c imm_host.c interp.c \
          liquids.c magic.c makeobjs.c mapout.c mapper.c mccp.c \
          misc.c mpxset.c mssp.c mud_comm.c mud_prog.c news.c planes.c player.c polymorph.c \
          renumber.c reset.c save.c services.c sha256.c shops.c skills.c special.c tables.c \
          track.c update.c variables.c weather.c

O_FILES := $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h)

all:
	$(info    Using C-Flags: $(C_FLAGS))
	$(info    Using L-Flags: $(L_FLAGS))
	$(MAKE) -s smaug
	$(MAKE) -s dns

# pull in dependency info for *existing* .o files
-include dependencies.d

ifdef IS_CYGWIN
smaug: $(O_FILES)
	rm -f smaug.exe
	dlltool --export-all --output-def smaug.def $(O_FILES)
	dlltool --dllname smaug.exe --output-exp smaug.exp --def smaug.def
	$(CC) -o smaug.exe $(O_FILES) smaug.exp $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w smaug.exe
	chmod a+x smaug.exe
	chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o smaug.exe dependencies.d resolver.exe resolver.o *~

else
smaug: $(O_FILES)
	rm -f smaug
	$(CC) -export-dynamic -o smaug $(O_FILES) $(L_FLAGS)
	@echo "Generating dependency file ...";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@echo "Done compiling mud.";
	chmod g+w smaug
	chmod a+x smaug
	chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o smaug dependencies.d resolver resolver.o *~
endif

dns: resolver.o
	rm -f resolver
	$(CC) $(D_FLAGS) -o resolver resolver.o
	@echo "Done compiling DNS resolver.";
	chmod g+w resolver
	chmod a+x resolver
	chmod g+w resolver.o

indent:
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(C_FILES)
	indent -ts3 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(H_FILES)

indentclean:
	rm *.c~ *.h~

o/%.o: %.c
	echo "  Compiling $@...";
	$(CC) -c $(C_FLAGS) $< -o $@

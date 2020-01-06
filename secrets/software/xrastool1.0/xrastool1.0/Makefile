# Makefile for xrastool -- DCR 93-04-19
#======================================

# Change the following values as required...

SHELL    = /bin/csh			# Shell to use
CC	 = cc				# Compiler
CFLAGS	 = -O4				# Optimize by default
LIB_PATH = /usr/openwin/lib		# Location of XView libraries
INC_PATH = /usr/openwin/include		# Location of XView header files
BIN_DEST = /usr/local/bin		# Optional install destination
MAN_DEST = /usr/local/man/mann		# Optional install.man destination
MAN_SECT = n				# Optional man page section to use

# No need to change anything below this line...

.KEEP_STATE:				# Recompile on new Makefile or .h files

# Options to use for debugging or gcc....

debug	:= CFLAGS = -g
gcc	:= CC = gcc
gcc	:= CFLAGS = -O4 -Wall

CPPFLAGS = -I$(INC_PATH)		# Flags for C preprocessor

# Omit "-target" designator for Gnu C compatability in following definitions...

COMPILE.c	= $(CC) $(CFLAGS) $(CPPFLAGS) -c
LINK.c		= $(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

# List of sources...

SOURCES = baseframe.c canvas.c cms.c loadimages.c mainpanel.c params.c\
		subpanel.c xrastool.c

# List of objects...

OBJECTS = $(SOURCES:.c=.o)

# Libraries to use...

LIBS = -L$(LIB_PATH) -lxview -lolgx -lX11

# Lint stuff...

LINTFILES = $(SOURCES:.c=.ln)

LINTFLAGS = -b

# Directives...

all opt debug gcc install: xrastool

xrastool: $(OBJECTS)
	$(LINK.c) -o $@ $(OBJECTS) $(LIBS)

opt:
	strip xrastool

install:
	cp xrastool $(BIN_DEST)

install.man:
	cp xrastool.man $(MAN_DEST)/xrastool.$(MAN_SECT)

lint: $(LINTFILES)
	$(LINT.c) $(LINTFILES)

cflow:
	cflow $(CPPFLAGS) $(SOURCES) | expand -3 >! cflow.out

clean:
	-rm -f {.,}*%
	-rm -f $(OBJECTS) $(LINTFILES) xrastool.lint cflow.out

backup:
	if (-e xrastool.tar.Z) mv -f xrastool.tar.Z{,.OLD}
	tar cvf xrastool.tar COPYRIGHT LICENSE Makefile README TODO\
		*.{c,h,man} bitmaps
	compress xrastool.tar

#/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
# *   mupen64plus-rsp-hle tests - Makefile                                  *
# *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
# *   Copyright (C) 2014 Bobby Smiles                                       *
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# *   This program is distributed in the hope that it will be useful,       *
# *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
# *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
# *   GNU General Public License for more details.                          *
# *                                                                         *
# *   You should have received a copy of the GNU General Public License     *
# *   along with this program; if not, write to the                         *
# *   Free Software Foundation, Inc.,                                       *
# *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
# * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

INCDIRS = -I../src/
LIBDIRS = -L./
LIBS = -lrsphle

RSPHLE_OBJDIR=_rsphle_obj
RSPHLE_SRCDIR=../src
RSPHLE_SOURCE= \
	$(RSPHLE_SRCDIR)/alist.c \
	$(RSPHLE_SRCDIR)/alist_audio.c \
	$(RSPHLE_SRCDIR)/alist_naudio.c \
	$(RSPHLE_SRCDIR)/alist_nead.c \
	$(RSPHLE_SRCDIR)/audio.c \
	$(RSPHLE_SRCDIR)/cicx105.c \
	$(RSPHLE_SRCDIR)/hle.c \
	$(RSPHLE_SRCDIR)/jpeg.c \
	$(RSPHLE_SRCDIR)/memory.c \
	$(RSPHLE_SRCDIR)/mp3.c \
	$(RSPHLE_SRCDIR)/musyx.c

RSPHLE_OBJECTS := \
	$(patsubst $(RSPHLE_SRCDIR)/%.c, $(RSPHLE_OBJDIR)/%.o, $(filter %.c, $(RSPHLE_SOURCE)))

AR     = ar
CC     = gcc
RM    ?= rm -f
MKDIR ?= mkdir -p

CFLAGS = -Wall -g $(INCDIRS)
OPTFLAGS = -O3 -flto -fwhole-program
ARFLAGS = rcs



all: regtest

clean:
	$(RM) -r regtest librsphle.a $(RSPHLE_OBJDIR) regtest.o

rebuild: clean all

librsphle.a: $(RSPHLE_OBJECTS)
	$(AR) $(ARFLAGS) $@ $^


regtest: librsphle.a
regtest: regtest.o
	$(CC) $(CFLAGS) $(OPTFAGS) $(LDFLAGS) $(LIBDIRS) $(LIBS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(RSPHLE_OBJDIR)/%.o: $(RSPHLE_SRCDIR)/%.c
	$(MKDIR) $(RSPHLE_OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: all clean rebuild

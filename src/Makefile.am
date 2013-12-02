
bin_PROGRAMS = sib-tcp

# Compiler flags

sib_tcp_CFLAGS  = -Wall -I$(top_srcdir) -I$(top_srcdir)/include @GNOME_CFLAGS@ @LIBSIB_CFLAGS@  $(sib_tcp_hipdex)

if HIPDEX
sib_tcp_hipdex  = -I../hip-dex/protocol/ -I../hip-dex/daemon/
endif

# Linker flags
sib_tcp_LDFLAGS = @GNOME_LIBS@ @LIBSIB_LIBS@ -lexpat -lm3_parse_n_gen -luuid -L/usr/local/ssl/lib/ $(sib_tcp_hipex_ldflags)

if HIPDEX
sib_tcp_hipex_ldflags = -L../hip-dex/protocol/ -L../hip-dex/daemon/ -Wl,--start-group -lhdx-daemon -lhdx-proto -lcrypto -ldl -lpthread -Wl,--end-group
endif
if AVAHI
	sib_tcp_LDFLAGS += @AVAHI_LIBS@
endif

AM_CFLAGS = $(test_sib_CFLAGS) -g
# Put these in alphabetical order so that they are easy to find.

sib_tcp_SOURCES = \
	main.c 


if HIPDEX
CCLD=g++
endif

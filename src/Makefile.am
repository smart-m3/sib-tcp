
bin_PROGRAMS = sib-tcp

# Compiler flags
sib_tcp_CFLAGS  = -Wall -I$(top_srcdir) -I$(top_srcdir)/include  
sib_tcp_CFLAGS += @GNOME_CFLAGS@ @LIBSIB_CFLAGS@


# Linker flags
sib_tcp_LDFLAGS = @GNOME_LIBS@ @LIBSIB_LIBS@ -lexpat -lm3_parse_n_gen -luuid

if AVAHI
	sib_tcp_LDFLAGS += @AVAHI_LIBS@
endif

AM_CFLAGS = $(test_sib_CFLAGS) -g
# Put these in alphabetical order so that they are easy to find.

sib_tcp_SOURCES = \
	main.c 



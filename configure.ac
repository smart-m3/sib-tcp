#############################################################################
#
#	
#
#	Copyright (c) 2007 Nokia Corporation
#
#############################################################################

AC_INIT()
AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE(sib-tcp, 0.4)
AM_CONFIG_HEADER(config.h)

#############################################################################
# Constants
#############################################################################

#############################################################################
# Checks for some common programs needed for configure/compile/install
#############################################################################

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_AWK
AC_PROG_LIBTOOL
AC_PROG_RANLIB


#############################################################################
# Check for some header files (TODO: Add more?)
#############################################################################
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdlib.h string.h unistd.h])

##############################################################################
# Check for iconv
##############################################################################
AC_CHECK_FUNCS([iconv])
AC_CHECK_HEADERS([iconv.h])
AC_CHECK_LIB([iconv],[main])

##############################################################################
# Check for pthread presence
##############################################################################
AC_CHECK_HEADERS([pthread.h],
		 [],
		 [AC_MSG_ERROR(WhiteBoard node needs pthread library)])
AC_CHECK_LIB([pthread],[main])

### debug prints or not...
AC_ARG_WITH(debug,
        AS_HELP_STRING([--with-debug],
                       [Print additional debug messages (default = no)]),
        [AC_DEFINE([WHITEBOARD_DEBUG],[1],[Print debug messages])],
        [with_debug=no]
)

AM_CONDITIONAL(SIB_DEBUG, test $with_debug = yes)


##############################################################################
# Check for CyberLink C & AV extension
##############################################################################
#[
#	clinkc >= 1.0-21
#	clinkcav >= 1.0-14
#])

#AC_SUBST(CLINKC_CFLAGS)
#AC_SUBST(CLINKC_LIBS)

##############################################################################
# Check for GNOME environment
##############################################################################

PKG_CHECK_MODULES(GNOME,
[
	glib-2.0 >= 2.8.6
	gobject-2.0 >= 2.8.6
	gthread-2.0 >= 2.8.6
	dbus-1 >= 0.61
])

AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)


PKG_CHECK_MODULES(LIBSIB,
[
	libsib            >= 2.0-beta1
	libwhiteboard-util >= 2.0-beta1
	libssap_parse_n_gen >= 2.0-beta1
])

AC_SUBST(LIBSIB_CFLAGS)
AC_SUBST(LIBSIB_LIBS)

AC_ARG_ENABLE(avahi,
	AS_HELP_STRING([--enable-avahi],
			[enable Avahi service discovery (default = no) NOTE: Not implemented yet!]),
	[
		if test $enableval = no; then
			USE_AVAHI=no
		else
			USE_AVAHI=yes
			AC_ERROR("Avahi support not yet implemented")
			AC_DEFINE([AVAHI],[1],[Use Avahi servcie discovery])
			PKG_CHECK_MODULES(AVAHI,
			[
				avahi-client >= 0.6.22
				avahi-glib   >= 0.6.22
				avahi-gobject >= 0.6.22
			])
			AC_SUBST(AVAHI_CFLAGS)
			AC_SUBST(AVAHI_LIBS)
		fi
	],[
		USE_AVAHI=no
	]
)
AM_CONDITIONAL(AVAHI, test $USE_AVAHI = yes)



AC_ARG_ENABLE(hipdex,
	AS_HELP_STRING([--enable-hipdex],
			[enable HIPDEX security (default = no)]),
	[
		if test $enableval = no; then
			USE_HIPDEX=no
		else
			USE_HIPDEX=yes
			AC_DEFINE([HIPDEX],[1],[Use hipdex security])
		fi
	],[
		USE_HIPDEX=no
	]
)
AM_CONDITIONAL(HIPDEX, test $USE_HIPDEX = yes)


##############################################################################
# Checks for typedefs, structures, and compiler characteristics.
##############################################################################

AC_C_CONST
AC_C_INLINE
AC_STRUCT_TM

##############################################################################
# Locale stuff
##############################################################################

AC_DEFINE_UNQUOTED([LOCALEDIR], 
		   "${runtime_localedir}", [Runtime locale catalog files path])

##############################################################################
# Check compile target to be able to disable local player in emulator
##############################################################################

case $host in
        i386-* | i686-*)
               CFLAGS="$CFLAGS -Di686"
	       USE_i686=yes
	       USE_ARM=no
               ;;
        arm-*)
               CFLAGS="$CFLAGS -DARM"
	       USE_ARM=yes
	       USE_i686=no
               ;;
esac

AM_CONDITIONAL(ARM, test $USE_ARM = yes)
AM_CONDITIONAL(i686, test $USE_i686 = yes)

##############################################################################
# Files to create
##############################################################################

AC_OUTPUT(
	Makefile
	include/Makefile
	src/Makefile
)

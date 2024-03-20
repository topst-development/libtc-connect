AUTOMAKE_OPTIONS = foreign subdir-objects

CC = @CC@
AM_CFLAGS = $(GLIB_CFLAGS) -I$(top_srcdir)/include
LIBS = @LIBS@ $(GLIB_LIBS)

lib_LTLIBRARIES = libTcConnect.la
libTcConnect_la_SOURCES = src/ConnectCallback.c src/HotPlugParser.c src/MountMonitor.c src/TCConnectInterface.c
libTcConnect_la_LDFLAGS = -version-info 1:0:0

include_HEADERS = include/ConnectCallback.h include/HotPlugParser.h include/MountMonitor.h include/TCConnectInterface.h


plugindir = $(libdir)/xchat-gnome/plugins
plugin_LTLIBRARIES = indicator.la

indicator_la_SOURCES = indicator.c

indicator_la_CPPFLAGS = 	\
	-I$(srcdir)		\
	$(AM_CPPFLAGS)

indicator_la_CFLAGS =		\
	$(DEPENDENCIES_CFLAGS)	\
	$(UNITY_CFLAGS)	\
	$(AM_CFLAGS)

indicator_la_LDFLAGS = 		\
	-avoid-version 		\
	-module			\
	$(AM_LDFLAGS)

indicator_la_LIBADD = 		\
	$(DEPENDENCIES_LIBS)	\
	$(UNITY_LIBS)

EXTRA_DIST =	xchat-plugin.h	\
		COPYING		\
		NEWS


install-xchat:
	mkdir -p $(libdir)/xchat/plugins
	cp .libs/indicator.so $(libdir)/xchat/plugins

noinst_LTLIBRARIES = libfluxsessionfe.la
 
libfluxsessionfe_la_SOURCES = $(top_srcdir)/../flux/fluxmgr.c $(top_srcdir)/../flux/spindlefemgr.c 
libfluxsessionfe_la_CPPFLAGS = -I$(top_srcdir)/../include -I$(top_srcdir)/../flux $(FLUX_CORE_CFLAGS) $(FLUX_HOSTLIST_CFLAGS)
libfluxsessionfe_la_LDFLAGS = $(fluxplugin_ldflags) -Wl,--allow-multiple-definition
libfluxsessionfe_la_LIBADD = $(FLUX_CORE_LIBS) $(FLUX_HOSTLIST_LIBS)


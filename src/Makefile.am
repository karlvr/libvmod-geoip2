AM_CPPFLAGS = @VMOD_INCLUDES@
AM_CFLAGS = -Wall -Wextra -Werror

vmoddir = @VMOD_DIR@
vmod_LTLIBRARIES = libvmod_geoip2.la

libvmod_geoip2_la_LDFLAGS = -module -export-dynamic -avoid-version -shared \
	-lmaxminddb

libvmod_geoip2_la_SOURCES = \
	vcc_if.c \
	vcc_if.h \
	vmod_geoip2.c

vcc_if.c vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_geoip2.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_geoip2.vcc

VMOD_TESTS = $(top_srcdir)/src/tests/*.vtc
.PHONY: $(VMOD_TESTS)

$(top_srcdir)/src/tests/*.vtc:
	@VARNISHTEST@ -Dvarnishd=@VARNISHD@ -Dvmod_topbuild=$(abs_top_builddir) $@

check: $(VMOD_TESTS)

EXTRA_DIST = vmod_geoip2.vcc $(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c $(builddir)/vcc_if.h

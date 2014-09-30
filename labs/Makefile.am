lib_LTLIBRARIES = libczmq_labs.la

include_HEADERS = \
    ../include/czmq.h \
    ../include/czmq_prelude.h \
    ../include/zactor.h \
    ../include/zauth.h \
    ../include/zbeacon.h \
    ../include/zcert.h \
    ../include/zcertstore.h \
    ../include/zchunk.h \
    ../include/zclock.h \
    ../include/zconfig.h \
    ../include/zdigest.h \
    ../include/zdir.h \
    ../include/zdir_patch.h \
    ../include/zfile.h \
    ../include/zframe.h \
    ../include/zhash.h \
    ../include/zgossip.h \
    ../include/ziflist.h \
    ../include/zlist.h \
    ../include/zloop.h \
    ../include/zmonitor.h \
    ../include/zmsg.h \
    ../include/zpoller.h \
    ../include/zproxy.h \
    ../include/zrex.h \
    ../include/zring.h \
    ../include/zsock.h \
    ../include/zsock_option.h \
    ../include/zstr.h \
    ../include/zsys.h \
    ../include/zuuid.h \
    ../src/zgossip_msg.h \
    ../include/zauth_v2.h \
    ../include/zbeacon_v2.h \
    ../include/zctx.h \
    ../include/zmonitor_v2.h \
    ../include/zmutex.h \
    ../include/zproxy_v2.h \
    ../include/zsocket.h \
    ../include/zsockopt.h \
    ../include/zthread.h

libczmq_labs_la_SOURCES = \
    zdlist.c \
    ztimeout.c \
    zspeedtest.c \
    zpubsub_filter.c \
    zpubsub_option.c \
    zpubsub.c

AM_CFLAGS = -g
AM_CPPFLAGS = -I$(top_srcdir)/include
bin_PROGRAMS = czmq_labs_selftest
czmq_labs_selftest_LDADD = libczmq_labs.la ../src/libczmq.la
czmq_labs_selftest_SOURCES = czmq_labs_selftest.c

if ON_MINGW
libczmq_labs_la_LDFLAGS = -no-undefined -avoid-version -version-info @LTVER@
else
libczmq_labs_la_LDFLAGS = -version-info @LTVER@
endif

TESTS = czmq_labs_selftest

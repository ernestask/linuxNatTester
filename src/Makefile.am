AM_CXXFLAGS = ${LIBXML_CFLAGS}

SUBDIRS =  	tools \
	udp \
	tester \
	upnp \
	stun 
	

bin_PROGRAMS = NatTester

NatTester_SOURCES = main.cxx framework.cfg
NatTester_LDFLAGS = 
NatTester_LDADD = tester/libtester.a \
	stun/libstun.a \
	udp/libudp.a \
	upnp/libupnp.a \
	tools/libtools.a 	\
	upnp/libupnp.a ${LIBXML_LIBS}

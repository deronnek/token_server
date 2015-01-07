include ../Makefile.in

CURBUILDDIR = $(SRVBUILDDIR)

$(shell mkdir -p $(CURBUILDDIR))
$(shell mkdir -p $(CURBUILDDIR)/../common)

DBDUMPCHECKOBJS = $(patsubst %.c, ../dict_dump/%$(OBJEXT), $(wildcard ../dict_dump/*.c))\
                  $(patsubst %.c, ../dict_dump/%$(OBJEXT), $(wildcard ../common/*.c))\

DBLOADOBJS      = $(patsubst %.c, ../dict_load/%$(OBJEXT), $(wildcard ../dict_load/*.c))\
                  $(patsubst %.c, ../dict_load/%$(OBJEXT), $(wildcard ../common/*.c))\
              
DBDUMPHEADERS   = $(wildcard ../dict_dump/*.h) $(wildcard ../common/*.h) $(wildcard $(GKLIBINCDIR)/*.h)

DBCHECKOBJS = $(patsubst %.c, ../mytchtest/%$(OBJEXT), $(wildcard ../mytchtest/*.c))\
              $(patsubst %.c, ../mytchtest/%$(OBJEXT), $(wildcard ../common/*.c))
              
DBHEADERS   = $(wildcard ../mytchtest/*.h) $(wildcard ../common/*.h) $(wildcard $(GKLIBINCDIR)/*.h)
            
ALLOBJS     = $(patsubst %.c, $(CURBUILDDIR)/%$(OBJEXT), $(wildcard *.c))\
              $(patsubst %.c, $(CURBUILDDIR)/%$(OBJEXT), $(wildcard ../common/*.c))

HEADERS     = $(wildcard *.h) $(wildcard ../common/*.h) $(wildcard $(GKLIBINCDIR)/*.h)


TARGETS = $(BUILDDIR)/toksrv $(BUILDDIR)/isdbcorrupt $(BUILDDIR)/dict_dump $(BUILDDIR)/dict_load\

default: $(TARGETS)

$(BUILDDIR)/isdbcorrupt: $(DBCHECKOBJS) $(LIBRARIES)
	$(LD) $(LDOPTIONS) $(EXEOUTPUTFILE) $(DBCHECKOBJS) $(LIBSDIR) $(LIBS)
	chmod 744 $@
	@if [ "$(BINDIR)" ]; then cp $@* $(BINDIR); fi

$(BUILDDIR)/dict_dump: $(DBDUMPCHECKOBJS) $(LIBRARIES)
	$(LD) $(LDOPTIONS) $(EXEOUTPUTFILE) $(DBDUMPCHECKOBJS) $(LIBSDIR) $(LIBS)
	chmod 744 $@
	@if [ "$(BINDIR)" ]; then cp $@* $(BINDIR); fi

$(BUILDDIR)/dict_load: $(DBLOADOBJS) $(LIBRARIES)
	$(LD) $(LDOPTIONS) $(EXEOUTPUTFILE) $(DBLOADOBJS) $(LIBSDIR) $(LIBS)
	chmod 744 $@
	@if [ "$(BINDIR)" ]; then cp $@* $(BINDIR); fi

$(BUILDDIR)/toksrv: $(ALLOBJS) $(LIBRARIES)
	$(LD) $(LDOPTIONS) $(EXEOUTPUTFILE) $(ALLOBJS) $(LIBSDIR) $(LIBS)
	chmod 744 $@
	@if [ "$(BINDIR)" ]; then cp $@* $(BINDIR); fi

clean:
	rm -f $(ALLOBJS) 
	rm -f ../common/*.o

doc:
	doxygen ../doc/Doxyfile

realclean:
	rm -f $(ALLOBJS) $(TARGETS)

prod:
	echo "I hope you're on hm7 (dev) or hm10 (prod)"
	sudo cp $(BUILDDIR)/toksrv /opt/bin
	sudo cp $(BUILDDIR)/isdbcorrupt /opt/bin
	sudo cp $(BUILDDIR)/dict_dump /opt/bin
	sudo cp $(BUILDDIR)/dict_load /opt/bin
	sudo chmod 755 /opt/bin/toksrv
	sudo chmod 755 /opt/bin/isdbcorrupt
	sudo chmod 755 /opt/bin/dict_dump
	sudo chmod 755 /opt/bin/dict_load


$(ALLOBJS) : $(HEADERS) ../Makefile.in Makefile

$(DBCHECKOBJS) : $(HEADERS) $(DBHEADERS) ../Makefile.in Makefile

$(DBLOADOBJS) : $(HEADERS) ../Makefile.in Makefile

$(DBDUMPCHECKOBJS) : $(HEADERS) ../Makefile.in Makefile

../mytchtest/%$(OBJEXT) : %.c
	$(CC) $(CFLAGS) $(SOURCEFILE) $(OUTPUTFILE)

$(CURBUILDDIR)/%$(OBJEXT) : %.c
	$(CC) $(CFLAGS) $(SOURCEFILE) $(OUTPUTFILE)

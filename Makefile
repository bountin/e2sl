
prefix = /usr/local
sbindir = $(prefix)/sbin

INSTALL = install

VERSION  = 0.1
FULLNAME = e2sl-$(VERSION)
DIST  = $(FULLNAME)
	
CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_LARGEFILE_SOURCE=1 -D_LARGEFILE64_SOURCE=1 -D_FILE_OFFSET_BITS=64

CFLAGS = -O

#CFLAGS = -O -Wall -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes

LDFLAGS = -s

objs = ext2-superblock.o

all: ext2-superblock

dist: clean
	pwd=`pwd` && \
	cd .. && \
	rm -rf $(DIST) && \
	mkdir -p $(DIST) && \
	cp -rp $$pwd/* $(DIST) && \
	rm -f $(DIST).tar* && \
	tar cf $(DIST).tar $(DIST) && \
	gzip -9 $(DIST).tar && \
	rm -rf $(DIST)

ext2-superblock: $(objs)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(objs)

install: all
	$(INSTALL) -m 0755 ext2-superblock $(sbindir)
	
clean:
	rm -f *.o ext2-superblock


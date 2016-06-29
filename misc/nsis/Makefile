ifndef VERSION
VERSION=1.0
endif
ifndef RELEASE
RELEASE=0
endif
ifndef ARCH
ARCH=x86
endif
ifndef INSTALLDIR
INSTALLDIR=.
endif
ifndef USE_RENDERER_DLOPEN
USE_RENDERER_DLOPEN=1
endif
ifndef USE_OPENAL_DLOPEN
USE_OPENAL_DLOPEN=1
endif
ifndef USE_CURL_DLOPEN
USE_CURL_DLOPEN=0
endif
ifndef USE_INTERNAL_SPEEX
USE_INTERNAL_SPEEX=1
endif
ifndef USE_INTERNAL_ZLIB
USE_INTERNAL_ZLIB=1
endif
ifndef USE_INTERNAL_JPEG
USE_INTERNAL_JPEG=1
endif
ifndef SDLDLL
	ifeq ($(ARCH),x86_64)
		SDLDLL=SDL264.dll
	else
		SDLDLL=SDL2.dll
	endif
endif

DEFINES=
ifeq ($(USE_RENDERER_DLOPEN),1)
	DEFINES+= -DUSE_RENDERER_DLOPEN
endif
ifeq ($(USE_OPENAL_DLOPEN),1)
	DEFINES+= -DUSE_OPENAL_DLOPEN
endif
ifeq ($(USE_CURL_DLOPEN),1)
	DEFINES+= -DUSE_CURL_DLOPEN
endif
ifeq ($(USE_INTERNAL_SPEEX),1)
	DEFINES+= -DUSE_INTERNAL_SPEEX
endif
ifeq ($(USE_INTERNAL_ZLIB),1)
	DEFINES+= -DUSE_INTERNAL_ZLIB
endif
ifeq ($(USE_INTERNAL_JPEG),1)
	DEFINES+= -DUSE_INTERNAL_JPEG
endif


all: quakewars-$(VERSION)-$(RELEASE).$(ARCH).exe

quakewars.$(ARCH).nsi: quakewars.nsi.in
	sed 's/XXXVERSIONXXX/$(VERSION)/;s/XXXRELEASEXXX/$(RELEASE)/;s/x86/$(ARCH)/g;s/SDL2.dll/$(SDLDLL)/' < $< > $@

quakewars-$(VERSION)-$(RELEASE).$(ARCH).exe: quakewars.$(ARCH).nsi
	makensis $(DEFINES) quakewars.$(ARCH).nsi

clean:
	rm -rf *.exe quakewars.$(ARCH).nsi

install:
	mkdir -p $(INSTALLDIR)
	mv quakewars-$(VERSION)-$(RELEASE).$(ARCH).exe $(INSTALLDIR)

.PHONY: all clean


SUBDIRS = g_common g_re1 g_re2 g_re3 r_common r_opengl r_soft

bin_PROGRAMS = reevengi

AM_CFLAGS = $(SDL_CFLAGS) $(AVCODEC_CFLAGS) $(AVFORMAT_CFLAGS) \
	$(SWSCALE_CFLAGS) $(PHYSFS_CFLAGS)
AM_CXXFLAGS = $(SDL_CFLAGS) $(AVCODEC_CFLAGS) $(AVFORMAT_CFLAGS) \
	$(SWSCALE_CFLAGS) $(PHYSFS_CFLAGS)

LIBS = $(SDL_LIBS) $(SDL_IMAGE_LIBS) $(PHYSFS_LIBS) $(ZLIB_LIBS) \
	$(AVCODEC_LIBS) $(AVFORMAT_LIBS) $(SWSCALE_LIBS) $(AVUTIL_LIBS) \
	$(MATH_LIBS)

reevengi_SOURCES = background_bss.c background_tim.c clock.c \
	depack_mdec.c depack_vlc.c \
	filesystem.c idctfst.c log.c main.c \
	parameters.c physfsrwops.c \
	video.c video_opengl.c \
	view_background.c view_movie.c view_movie_sdl2.c

reevengi_headers = background_bss.h background_tim.h clock.h \
	depack_mdec.h depack_vlc.h \
	filesystem.h idctfst.h log.h \
	parameters.h physfsrwops.h \
	video.h \
	view_background.h view_movie.h

reevengi_LDADD = g_re1/libg_re1.a g_re2/libg_re2.a g_re3/libg_re3.a \
	g_common/libg_common.a \
	r_opengl/libr_opengl.a r_soft/libr_soft.a \
	r_common/libr_common.a

EXTRA_DIST = $(reevengi_headers) reevengi.vcproj

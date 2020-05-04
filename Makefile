PROGRAM=wima.exe
SRCS=\
	winmain.c \
	wima.c
HDRS=\
	wima.h
OBJS=$(SRCS:.c=.obj)

CC=cl /nologo
CFLAGS=/O2tx /Qpar /GL /GA /utf-8 /MD /W3
LD=cl /nologo
LFLAGS=$(CFLAGS)
EXTLDFLAGS=/link /LTCG /MACHINE:X64 /SUBSYSTEM:WINDOWS
LIBS=user32.lib shell32.lib shlwapi.lib
RM=del
RMFLAGS=/Q


all: $(PROGRAM)

clean:
	$(RM) $(RMFLAGS) $(OBJS)

distclean: clean
	$(RM) $(RMFLAGS) $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LD) $(LDFLAGS) /Fe$@ $** $(LIBS) $(EXTLDFLAGS)

$(OBJS): $(HDRS)

.c.obj:
	$(CC) $(CFLAGS) /c /Fo$@ $<

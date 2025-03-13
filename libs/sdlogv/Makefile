TARGET=sdlogv

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
CFLAGS = -ansi -pedantic -Wall -Wextra -Werror
LIBS = -lSDL2_mixer -lSDL2 -ltheoradec -ltheora -lvorbis -logg

all: $(OBJS) $(TARGET)

%: %.c theora.h
	$(CC) $(CFLAGS) $< -c $@

$(TARGET):
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

clean:
	rm $(TARGET) *.o || true

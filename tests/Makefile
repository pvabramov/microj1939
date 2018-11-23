TARGET        = unittest
### SOURCES


UNITY_SRCS      := $(wildcard unity/*.c)
HELPERS_SRCS    := $(wildcard helpers/*.c)

SRCS            :=  $(wildcard ./*.c) \
                    $(HELPERS_SRCS)

INCDIRS         := . ..

DEFINES         := _GNU_SOURCE=1

### BUILD CONFIGURATION


UNITY_OBJS      := $(UNITY_SRCS:%.c=%.o)
OBJS            := $(SRCS:%.c=%.o)


CFLAGS_INCS     := $(INCDIRS:%=-I%)
CFLAGS_DEFS     := $(DEFINES:%=-D%)
CFLAGS          := -O2 -std=gnu11 -Wall -Wextra $(CFLAGS_DEFS) $(CFLAGS_INCS)

LDFLAGS         := -L.. -lj1939 -lgcov


### BUILD RULES


all: $(TARGET)


$(TARGET): unity $(OBJS)
	$(CC) -o $@ $(UNITY_OBJS) $(OBJS) $(LDFLAGS)


unity: $(UNITY_OBJS)


run:
	@./$(TARGET) -v


clean:
	$(RM) $(UNITY_OBJS)
	$(RM) $(OBJS)
	$(RM) $(TARGET)


%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<


.PHONY: all clean


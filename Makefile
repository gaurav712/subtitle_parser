CC="gcc"
CFLAGS="-Wall"

all:
	$(CC) -o subtitle_parser $(CFLAGS) subtitle_parser.c

clean:
	rm -f subtitle_parser

install: all
	cp subtitle_parser /usr/local/bin/subtitle_parser

uninstall:
	rm -f /usr/local/bin/subtitle_parser

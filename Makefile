all:
	clang -lbsd -g -o cache_test *.c
	dd if=/dev/zero of=my.img bs=1M count=2

sanitize:
	clang -lbsd -fsanitize=address -O0 -g -o cache_test *.c
	dd if=/dev/zero of=my.img bs=1M count=2


clean:
	rm cache_test my.img

open:
	gedit *.h *.c

.PHONY: clean open

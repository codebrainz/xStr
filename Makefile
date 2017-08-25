cflags = $(CPPFLAGS) $(CFLAGS) -g -Wall -Werror -Wextra -std=c99 -pedantic
ldflags = $(LDFLAGS)

lib_sources = xstr.c
lib_objects = $(lib_sources:.c=.o)
lib_depends = $(lib_sources:.c=.d)

test_sources = test.c
test_objects = $(test_sources:.c=.o)
test_depends = $(test_sources:.c=.d)

checks = check check-asan check-msan check-static check-valgrind
tests = test test-msan test-asan

all: libxstr.a

check: test
	./test

check-all: $(checks)

check-asan: test-asan
	./test-asan

check-msan: test-msan
	./test-msan

check-static: test.c xstr.c xstr.h
	cppcheck --std=c99 --inline-suppr test.c xstr.c xstr.h

check-valgrind: test
	valgrind --leak-check=full --show-reachable=yes ./test

clean:
	$(RM) *.[ado] $(tests)

libxstr.a: $(lib_objects)
	$(AR) rcs $@ $(lib_objects)

.c.o:
	$(CC) $(cflags) -c -MMD -o $@ $<

test: test.c xstr.c
	$(CC) $(cflags) -o $@ test.c xstr.c $(ldflags)

test-asan: test.c xstr.c
	clang $(cflags) -O1 -fsanitize=address -fno-omit-frame-pointer -o $@ test.c xstr.c $(ldflags)

test-msan: test.c xstr.c
	clang $(cflags) -O1 -fsanitize=memory -fno-omit-frame-pointer -o $@ test.c xstr.c $(ldflags)

-include $(lib_depends) $(test_depends)

.PHONY: all $(checks) check-all clean

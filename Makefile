CC 				?=	clang
LIBS 			:=	-lcap
DEFS 			:=
INCLUDES		:=	-I/usr/include
BIN 			:=	tinyc
LIB 			:=	./src/libtinyc.a
SOURCE_BIN 		:=	./src/main.c
BUILD 			:=	debug
CFLAGS 			:=	-std=gnu99 -Wall -O2

SRCS 			:=	$(shell find ./src/ -name '*.c')
LIB_OBJS 		:=	$(patsubst %.c, %.o, $(filter-out $(SOURCE_BIN), $(SRCS)))
TESTS 			:=	$(shell find ./test/ -name '*.c')
TESTS_BINS		:=	$(patsubst %.c, %.out, $(filter-out $(SOURCE_BIN), $(TESTS)))


all: $(BIN) depend

$(BIN): $(LIB) $(SOURCE_BIN)
	$(CC) $(CFLAGS) $(SOURCE_BIN) $(DEFS) $(INCLUDES) -o $@ $< $(LIBS)

install: $(BIN)
	sudo cp ./tinyc /usr/local/bin/tinyc

test: clean all $(TESTS_BINS)
	@echo "-------"
	@echo "TESTS:"
	@echo "-------"
	@find ./test/ -name "*.out" -exec /bin/sh -c '{ echo {} ; ./{} ; }'  \;

test-leak: clean all $(TESTS_BINS)
	@echo "-------"
	@echo "TEST LEAK:"
	@echo "-------"
	@find ./test/ -name "*.out" -exec /bin/sh -c '{ echo {} ; valgrind --leak-check=yes --error-exitcode=1 {} ; }'  \;


$(LIB): $(LIB_OBJS)
	$(AR) rvs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<


depend: .depend

.depend: $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -MM $^ -MF ./.depend

include .depend

%.out: %.c
	$(CC) $(CFLAGS) $< $(DEFS) $(INCLUDES) $(LIBS) -o $@ $(LIB)

print-%:
	@echo '$*=$($*)'

clean:
	find . \( -name "*.o" -o -name "*.a" -o -name "*.out" \) -type f -delete &
	find . \( -name "callgrind.*" -o -name $(BIN) \) -type f -delete

fmt:
	find . -name "*.c" -o -name "*.h" | xargs clang-format -style=file -i


.PHONY: clean depend fmt


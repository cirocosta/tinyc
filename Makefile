CC 				?= clang
LIBS 			:= -lpthread
DEFS 			:= -D_GNU_SOURCE
INCLUDES		:= -I/usr/include
BIN 			:= tinyc
LIB 			:= src/libtinyc.a
SOURCE_BIN 		:= src/main.c
TESTS_DIR 		:= tests/
BUILD 			:= debug
CFLAGS 			:= -std=gnu99 -Wall -O2

SRCS 			:= $(shell find src/ -name '*.c')
LIB_OBJS 		:= $(patsubst %.c, %.o, $(filter-out $(SOURCE), $(SRCS)))
TESTS			:= $(patsubst %.c, %.out, $(shell find $(TESTS_DIR) -name '*.c'))


all: $(BIN) test depend

$(BIN): $(LIB) $(SOURCE_BIN)
	$(CC) $(CFLAGS) $(SOURCE_BIN) $(DEFS) $(INCLUDES) $(LIBS) -o $@ $<

$(LIB): $(LIB_OBJS)
	$(AR) rvs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) $(INCLUDES) -c -o $@ $<


depend: .depend

.depend: $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) -MM $^ -MF ./.depend

include .depend

test: $(LIB) $(TESTS)
	@$(foreach test_exec,$(TESTS),./$(test_exec);)

%.out: %.c
	$(CC) $(CFLAGS) $< $(DEFS) $(INCLUDES) $(LIBS) -o $@ $(LIB)

print-%:
	@echo '$*=$($*)'

clean:
	find . \( -name "*.o" -o -name "*.a" -o -name "*.out" \) -type f -delete &
	find . \( -name "callgrind.*" -o -name $(BIN) \) -type f -delete

.PHONY: clean test depend


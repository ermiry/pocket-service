TYPE		:= development

TARGET      := pocket-service

all: directories $(TARGET)

directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

PTHREAD 	:= -l pthread
MATH 		:= -lm

OPENSSL		:= -l ssl -l crypto

CURL		:= -l curl

# MONGOC 		:= `pkg-config --libs --cflags libmongoc-1.0`
MONGOC 		:= -l mongoc-1.0 -l bson-1.0
MONGOC_INC	:= -I /usr/local/include/libbson-1.0 -I /usr/local/include/libmongoc-1.0

CMONGO		:= -l cmongo
CMONGO_INC	:= -I /usr/local/include/cmongo

HIREDIS		:= -l hiredis

CREDIS		:= -l credis
CREDIS_INC	:= -I /usr/local/include/credis

CERVER		:= -l cerver
CERVER_INC	:= -I /usr/local/include/cerver

DEVELOPMENT	:= -D POCKET_DEBUG

DEFINES		:= -D _GNU_SOURCE

CC          := gcc

GCCVGTEQ8 	:= $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 8)

SRCDIR      := src
INCDIR      := include
BUILDDIR    := objs
TARGETDIR   := bin

SRCEXT      := c
DEPEXT      := d
OBJEXT      := o

# common flags
# -Wconversion -march=native
COMMON		:=  -Wall -Wno-unknown-pragmas \
				-Wfloat-equal -Wdouble-promotion -Wint-to-pointer-cast -Wwrite-strings \
				-Wtype-limits -Wsign-compare -Wmissing-field-initializers \
				-Wuninitialized -Wmaybe-uninitialized -Wempty-body \
				-Wunused-but-set-parameter -Wunused-result \
				-Wformat -Wformat-nonliteral -Wformat-security -Wformat-overflow -Wformat-signedness -Wformat-truncation

# main
CFLAGS      := $(DEFINES)

ifeq ($(TYPE), development)
	CFLAGS += -g -fasynchronous-unwind-tables -D_FORTIFY_SOURCE=2 -fstack-protector $(DEVELOPMENT)
else ifeq ($(TYPE), test)
	CFLAGS += -g -fasynchronous-unwind-tables -D_FORTIFY_SOURCE=2 -fstack-protector -O2 $(DEVELOPMENT)
else
	CFLAGS += -D_FORTIFY_SOURCE=2 -O2
endif

CFLAGS += -std=c11 -Wpedantic -pedantic-errors
# check for compiler version
ifeq "$(GCCVGTEQ8)" "1"
	CFLAGS += -Wcast-function-type
else
	CFLAGS += -Wbad-function-cast
endif

CFLAGS += $(COMMON)

LIB         := -L /usr/local/lib $(PTHREAD) $(MATH) $(OPENSSL) $(MONGOC) $(CERVER) $(CMONGO) $(HIREDIS) $(CREDIS)
INC         := -I $(INCDIR) -I /usr/local/include $(MONGOC_INC) $(CERVER_INC) $(CMONGO_INC) $(CREDIS_INC)
INCDEP      := -I $(INCDIR)

SOURCES     := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

# link
$(TARGET): $(OBJECTS)
	$(CC) $^ $(LIB) -o $(TARGETDIR)/$(TARGET)

# compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) $(LIB) -c -o $@ $<
	@$(CC) $(CFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

# tests
TESTDIR		:= test
TESTBUILD	:= $(TESTDIR)/objs
TESTTARGET	:= $(TESTDIR)/bin

TESTFLAGS	:= -g $(DEFINES) -Wall -Wno-unknown-pragmas -Wno-format

TESTLIBS	:= -L /usr/local/lib $(PTHREAD) $(CURL) $(CERVER)

TESTINC		:= -I $(INCDIR) -I ./$(TESTDIR) $(CERVER_INC)

TESTS		:= $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))
TESTOBJS	:= $(patsubst $(TESTDIR)/%,$(TESTBUILD)/%,$(TESTS:.$(SRCEXT)=.$(OBJEXT)))

integration: testout $(TESTOBJS)
	$(CC) $(TESTINC) ./$(TESTBUILD)/categories.o ./$(TESTBUILD)/curl.o -o ./$(TESTTARGET)/categories $(TESTLIBS)
	$(CC) $(TESTINC) ./$(TESTBUILD)/places.o ./$(TESTBUILD)/curl.o -o ./$(TESTTARGET)/places $(TESTLIBS)
	$(CC) $(TESTINC) ./$(TESTBUILD)/transactions.o ./$(TESTBUILD)/curl.o -o ./$(TESTTARGET)/transactions $(TESTLIBS)
	$(CC) $(TESTINC) ./$(TESTBUILD)/users.o ./$(TESTBUILD)/curl.o -o ./$(TESTTARGET)/users $(TESTLIBS)

testout:
	@mkdir -p ./$(TESTTARGET)

test: testout
	$(MAKE) $(TESTOBJS)
	$(MAKE) integration

# compile tests
$(TESTBUILD)/%.$(OBJEXT): $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CC) $(TESTFLAGS) $(TESTINC) $(TESTLIBS) -c -o $@ $<
	@$(CC) $(TESTFLAGS) $(INCDEP) -MM $(TESTDIR)/$*.$(SRCEXT) > $(TESTBUILD)/$*.$(DEPEXT)
	@cp -f $(TESTBUILD)/$*.$(DEPEXT) $(TESTBUILD)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(TESTBUILD)/$*.$(OBJEXT):|' < $(TESTBUILD)/$*.$(DEPEXT).tmp > $(TESTBUILD)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(TESTBUILD)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(TESTBUILD)/$*.$(DEPEXT)
	@rm -f $(TESTBUILD)/$*.$(DEPEXT).tmp

clean:
	@$(RM) -rf $(BUILDDIR) 
	@$(RM) -rf $(TARGETDIR)
	@$(RM) -rf $(TESTBUILD)
	@$(RM) -rf $(TESTTARGET)

.PHONY: all clean

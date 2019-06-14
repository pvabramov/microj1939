# output binary
BIN := libj1939.a

# source files
SRCS := $(wildcard $(CURDIR)/src/*.c)
INCS := $(CURDIR)

# files included in the tarball generated by 'make dist' (e.g. add LICENSE file)
DISTFILES := $(BIN)

# filename of the tar archive generated by 'make dist'
DISTOUTPUT := $(BIN).tar.gz

# intermediate directory for generated object files
OBJDIR := .o
# intermediate directory for generated dependency files
DEPDIR := .d

# object files, auto generated from source files
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

# compilers (at least gcc and clang) don't create the subdirectories automatically
$(shell mkdir -p $(dir $(OBJS)) >/dev/null)
$(shell mkdir -p $(dir $(DEPS)) >/dev/null)

# tar
TAR := tar

COV_INFO = libj1939.info
COV_REPORT_DIR = coverage_report

# C flags
ifndef NOTEST
COVFLAGS := -fprofile-arcs -ftest-coverage
endif
CFLAGS := -std=c11 $(INCS:%=-I%) -g -Wall -Wextra -pedantic $(COVFLAGS)
#
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

# compile C source files
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@
# link object files to binary
LINK.o = $(CC) $(LDFLAGS) $(LDLIBS) -o $@
STLIB  = $(AR) rcs ${@}
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all: $(BIN)

dist: $(DISTFILES)
	$(TAR) -cvzf $(DISTOUTPUT) $^

.PHONY: clean
clean:
	@make -C tests $@
	$(RM) -r $(OBJDIR) $(DEPDIR)
	$(RM) $(BIN)
	$(RM) $(COV_INFO)
	$(RM) -r $(COV_REPORT_DIR)

.PHONY: distclean
distclean: clean
	$(RM) $(BIN) $(DISTOUTPUT)

.PHONY: install
install:
	@echo no install tasks configured

.PHONY: uninstall
uninstall:
	@echo no uninstall tasks configured

.PHONY: tests
tests: $(BIN)
	@make -C tests

.PHONY: check
check:
	@make -C tests run

.PHONY: coverage
coverage:
	@lcov --directory .o --capture --output-file $(COV_INFO) -t unittest
	@mkdir -p $(COV_REPORT_DIR)
	@genhtml -o $(COV_REPORT_DIR) --function-coverage -s -t "unittest coverage report" --legend $(COV_INFO)

.PHONY: help
help:
	@echo available targets: all dist clean distclean install uninstall check tests

$(BIN): $(OBJS)
	$(STLIB) $^

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc
$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cxx
$(OBJDIR)/%.o: %.cxx $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS = $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(DEPS)


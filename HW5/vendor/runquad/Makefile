RM       = rm -rf
MAKEFLAGS = --no-print-directory

.PHONY: run clean run-one libsysy64.ll

BUILD_DIR = $(CURDIR)

clean: 
	$(RM) ../../test/*.llvm; \
	$(RM) $(CURDIR)/libsysy64.ll; \

HOST_ARCH = $(shell uname -m)

# Detect architecture and select appropriate libsysy
ARCH = $(shell uname -m)
ifeq ($(ARCH),x86_64)
	EXE=$(CURDIR)/runquad-ubuntu-amd64
else ifeq ($(ARCH),aarch64)
	EXE=$(CURDIR)/runquad-ubuntu-aarch64
else ifeq ($(ARCH),arm64)
	EXE=$(CURDIR)/runquad-macos-silicon
else 
	$(error "Unsupported architecture: $(ARCH)")
endif

LLVMLINK := $(shell if command -v llvm-link >/dev/null 2>&1; then echo llvm-link; elif command -v llvm-link-14 >/dev/null 2>&1; then echo llvm-link-14; fi)
ifeq ($(LLVMLINK),)
	$(error "Neither llvm-link nor llvm-link-14 found in PATH")
endif

ifeq ($(LLVMLINK),llvm-link-14)
	LLI = lli-14
else ifeq ($(LLVMLINK),llvm-link)
	LLI = lli
else
	$(error "Unsupported architecture for lli: $(ARCH)")
endif

$(CURDIR)/libsysy64.ll: $(CURDIR)/libsysy64.c
	@echo "Compiling libsysy64.c to LLVM IR..."
	@clang -emit-llvm -c -fno-builtin $(CURDIR)/libsysy64.c -o $(CURDIR)/libsysy64.ll

run: $(EXE) $(CURDIR)/libsysy64.ll
	cd $(CURDIR)/../../test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "4-xml.quad" ]; then \
			echo "Reading $${file}"; \
			$(EXE) "$${file%%.*}"; \
			echo echo "Running $${file%%.*}.4.llvm"; \
			$(LLVMLINK) "$${file%%.*}.4.llvm" $(CURDIR)/libsysy64.ll -o "${FILE}.4.llvm.linked" && $(LLI) "${FILE}.4.llvm.linked" || true; \
			$(RM) "${FILE}.4.llvm.linked"; \
		fi; \
	done; \
	cd $(CURDIR) > /dev/null 2>&1 

FILE=test1

run-one: $(EXE) $(CURDIR)/libsysy64.ll
	cd $(CURDIR)/../../test && \
	echo "Reading ${FILE}.4-xml.quad"; \
	$(EXE) "${FILE}"; \
	echo echo "Running ${FILE}.4.llvm"; \
	$(LLVMLINK) "${FILE}.4.llvm" $(CURDIR)/libsysy64.ll -o "${FILE}.4.llvm.linked" && $(LLI) "${FILE}.4.llvm.linked" || true; \
	$(RM) "${FILE}.4.llvm.linked"; \
	cd $(CURDIR) > /dev/null 2>&1 

RM       = rm -rf
MAKEFLAGS = --no-print-directory

.PHONY: build clean rebuild handin run-one run-one-assem run run-assem

BUILD_DIR = $(CURDIR)/build

build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release; \
	cd $(BUILD_DIR) && ninja
# cd $(BUILD_DIR) && ninja -v

clean: 
	@$(RM) build ; \
	$(RM) test/k?/*

rebuild: clean build

handin:
	@if [ ! -f docs/report.pdf ]; then \
		echo "请先在docs文件夹下攥写报告, 并转换为'report.pdf'"; \
		exit 1; \
	fi; \
	echo "请输入'学号-姓名' (例如: 12345678910-某个人)"; \
	read filename; \
	zip -q -r "docs/$$filename-hw12.zip" \
	  docs/report.pdf include lib

MAIN = $(BUILD_DIR)/tools/main/main
LIBSYS32 = $(CURDIR)/vendor/libsysy/libsysy32.s

$(LIBSYS32):
	@echo "Making libsysy32.s..."; \
	cd $(CURDIR)/vendor/libsysy && \
	arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -S libsysy32.c -o libsysy32.s

run: $(MAIN)
	cd $(CURDIR)/test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "5-xml.asm" ]; then \
			echo "Reading $${file}"; \
			$(MAIN) "$${file%%.*}"; \
		fi; \
	done; \
	cd .. > /dev/null 2>&1 

run-assem: $(LIBSYS32)
	cd $(CURDIR)/test/k2 && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "colored.s" ]; then \
		    echo "Running $${file} ........." ; \
			arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra  -Wextra -Wl,-z,noexecstack --static -o "$${file%%.*}" "$${file%%.*}.colored.s" $(LIBSYS32) -lm; \
            echo "Running the final assembly program with k=2........." ; \
            qemu-arm "$${file%%.*}"; \
			arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra  -Wextra -Wl,-z,noexecstack  --static -o "../k5/$${file%%.*}" "../k5/$${file%%.*}.colored.s" $(LIBSYS32) -lm; \
            echo "Running the final assembly program with k=5........." ; \
            qemu-arm "../k5/$${file%%.*}"; \
			arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra  -Wextra -Wl,-z,noexecstack  --static -o "../k9/$${file%%.*}" "../k9/$${file%%.*}.colored.s" $(LIBSYS32) -lm; \
            echo "Running the final assembly program with k=9........." ; \
            qemu-arm "../k9/$${file%%.*}"; \
        fi; \
    done; \
    cd .. > /dev/null 2>&1 

FILE=test1

run-one: $(MAIN) 
	cd $(CURDIR)/test && \
	echo "Reading ${FILE}.5-xml.asm"; \
	$(MAIN) "${FILE}"; \
	cd .. > /dev/null 2>&1 

run-one-assem-2: $(LIBSYS32)
	cd $(CURDIR)/test && \
	arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static -o "k2/${FILE}" "k2/${FILE}.colored.s" $(LIBSYS32) ; \
	echo "Running the final assembly program........." ; \
	qemu-arm "k2/${FILE}"; \
	cd .. > /dev/null 2>&1 

run-one-assem-5: $(LIBSYS32)
	cd $(CURDIR)/test && \
	arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static -o "k5/${FILE}" "k5/${FILE}.colored.s" $(LIBSYS32) ; \
	echo "Running the final assembly program........." ; \
	qemu-arm "k5/${FILE}"; \
	cd .. > /dev/null 2>&1 

run-one-assem-9: $(LIBSYS32)
	cd $(CURDIR)/test && \
	arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -Wextra -Wl,-z,noexecstack --static -o "k9/${FILE}" "k9/${FILE}.colored.s" $(LIBSYS32) ; \
	echo "Running the final assembly program........." ; \
	qemu-arm "k9/${FILE}"; \
	cd .. > /dev/null 2>&1 

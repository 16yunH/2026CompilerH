RM       = rm -rf
MAKEFLAGS = --no-print-directory

.PHONY: build clean handin run run-one rebuild deepclean

BUILD_DIR = $(CURDIR)/build

build:
	@cmake -G Ninja -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release; \
	cd $(BUILD_DIR) && ninja

clean: 
	@$(RM) build ; \
	$(RM) test/*ssa*.quad; \

rebuild: clean build

deepclean: clean 

handin:
	@if [ ! -f docs/report.pdf ]; then \
		echo "请先在docs文件夹下攥写报告, 并转换为'report.pdf'"; \
		exit 1; \
	fi; \
	echo "请输入'学号-姓名' (例如: 12345678910-某个人)"; \
	read filename; \
	zip -q -r "docs/$$filename-hw7.zip" \
	  docs/report.pdf include lib

MAIN = $(BUILD_DIR)/tools/main/main

run: $(MAIN)
	cd $(CURDIR)/test && \
	for file in $$(ls .); do \
		if [ "$${file#*.}" = "4-quadwithflow-xml.quad" ]; then \
			echo "Reading $${file}"; \
			$(MAIN) "$${file%%.*}"; \
		fi; \
	done; \
	cd .. > /dev/null 2>&1 

FILE=test1

run-one: $(MAIN)
	cd $(CURDIR)/test && \
	echo "Reading ${FILE}.4-quadwithflow-xml.quad"; \
	$(MAIN) "${FILE}"; \
	cd .. > /dev/null 2>&1 
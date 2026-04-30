CC = cc
CFLAGS = -Wall -Wextra -pedantic -D__LCOM_OPTIMIZED__ -I.
BUILD_DIR = ../build/lib
AR = ar
ARFLAGS = rcs

MODULES = utils keyboard mouse rtc timer vbe

all: $(MODULES)

# Generic rule for each module
$(MODULES):
	@echo "Building library: lib$@.a"
	@mkdir -p $(BUILD_DIR)/$@
	@for file in $@/*.c; do \
		if [ -f "$$file" ]; then \
			$(CC) $(CFLAGS) -c "$$file" -o $(BUILD_DIR)/$@/$$(basename "$$file" .c).o; \
		fi \
	done
	@# Only create the archive if there are .o files compiled
	@if ls $(BUILD_DIR)/$@/*.o >/dev/null 2>&1; then \
		$(AR) $(ARFLAGS) $(BUILD_DIR)/lib$@.a $(BUILD_DIR)/$@/*.o; \
	fi

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean $(MODULES)

include Makefile.inc

# Activar advertencias de compilación
GCCFLAGS += -Wall

KERNEL=kernel.bin

SOURCES_MEMORY=memory/memoryManager.c

OBJDIR := obj

src_to_obj = $(OBJDIR)/$(subst /,_,$(basename $(1))).o

C_SOURCES := $(wildcard *.c)
C_SOURCES += $(wildcard drivers/*.c)
C_SOURCES += $(wildcard idt/*.c)
C_SOURCES += $(wildcard processes/*.c)
C_SOURCES += $(wildcard utils/*.c)
C_SOURCES += $(SOURCES_MEMORY)
C_SOURCES := $(sort $(C_SOURCES))

ASM_SOURCES := $(wildcard asm/*.asm) $(wildcard idt/*.asm)
ASM_SOURCES := $(sort $(ASM_SOURCES))

LOADERSRC=loader.asm
LOADEROBJECT=$(call src_to_obj,$(LOADERSRC))
STATICLIBS=

C_OBJECTS=$(foreach src,$(C_SOURCES),$(call src_to_obj,$(src)))
ASM_OBJECTS=$(foreach src,$(ASM_SOURCES) $(LOADERSRC),$(call src_to_obj,$(src)))
ASM_OBJECTS_NO_LOADER=$(filter-out $(LOADEROBJECT),$(ASM_OBJECTS))

all: $(KERNEL)

ALL_OBJECTS=$(sort $(C_OBJECTS) $(ASM_OBJECTS_NO_LOADER))

$(KERNEL): $(LOADEROBJECT) $(ALL_OBJECTS) $(STATICLIBS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(ALL_OBJECTS) $(STATICLIBS)
		@echo "========================================"
		@echo "Kernel compiled successfully!"
		@echo "========================================"

define make-c-rule
$$(call src_to_obj,$(1)): $(1)
	@mkdir -p $(OBJDIR)
	$(GCC) $(GCCFLAGS) -Iinclude -c $$< -o $$@
endef

define make-asm-rule
$$(call src_to_obj,$(1)): $(1)
	@mkdir -p $(OBJDIR)
	$(ASM) $(ASMFLAGS) $$< -o $$@
endef

$(foreach src,$(C_SOURCES),$(eval $(call make-c-rule,$(src))))
$(foreach src,$(ASM_SOURCES) $(LOADERSRC),$(eval $(call make-asm-rule,$(src))))

clean:
	rm -rf $(OBJDIR) *.bin

.PHONY: all clean

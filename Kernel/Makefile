include Makefile.inc

# Activar advertencias de compilación
GCCFLAGS += -Wall

KERNEL=kernel.bin

# ============================================
#  SELECCIÓN CONDICIONAL DEL MEMORY MANAGER
# ============================================
ifeq ($(MM),USE_BUDDY)
    # Compilar SOLO buddy.c
    SOURCES_MEMORY=memory/buddy.c
    GCCFLAGS+=-DUSE_BUDDY
    $(info ========================================)
    $(info    Compiling with BUDDY SYSTEM)
    $(info ========================================)
else
    # Compilar SOLO memoryManager.c
    SOURCES_MEMORY=memory/memoryManager.c
    $(info ========================================)
    $(info    Compiling with STANDARD MM)
    $(info ========================================)
endif

OBJDIR := obj

src_to_obj = $(OBJDIR)/$(subst /,_,$(basename $(1))).o

C_SOURCES=$(wildcard *.c) \
          $(wildcard drivers/*.c) \
          $(wildcard idt/*.c) \
          $(wildcard processes/*.c) \
          $(wildcard utils/*.c)

C_SOURCES+=$(SOURCES_MEMORY)

ASM_SOURCES=$(wildcard asm/*.asm) $(wildcard idt/*.asm)

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
ifeq ($(MM),USE_BUDDY)
	@echo "Memory Manager: BUDDY SYSTEM"
else
	@echo "Memory Manager: STANDARD"
endif
	@echo "========================================"

define make-c-rule
$$(call src_to_obj,$(1)): $(1)
	@mkdir -p $(OBJDIR)
	$(GCC) $(GCCFLAGS) -I./include -c $$< -o $$@
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

include Makefile.inc

# Activar advertencias de compilación
GCCFLAGS += -Wall

KERNEL=kernel.bin
SOURCES=$(wildcard *.c)
SOURCES_IDT=$(wildcard idt/*.c)
SOURCES_DRIVERS=$(wildcard drivers/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
SOURCES_ASM_IDT=$(wildcard idt/*.asm)
SOURCES_PROCESSES=$(wildcard processes/*.c)
SOURCES_UTILS=$(wildcard utils/*.c)

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

OBJECTS=$(SOURCES:%.c=$(OBJDIR)/%.o) \
        $(SOURCES_IDT:%.c=$(OBJDIR)/%.o) \
        $(SOURCES_DRIVERS:%.c=$(OBJDIR)/%.o) \
        $(SOURCES_MEMORY:%.c=$(OBJDIR)/%.o) \
        $(SOURCES_PROCESSES:%.c=$(OBJDIR)/%.o) \
        $(SOURCES_UTILS:%.c=$(OBJDIR)/%.o)
OBJECTS_ASM=$(SOURCES_ASM:%.asm=$(OBJDIR)/%.o) $(SOURCES_ASM_IDT:%.asm=$(OBJDIR)/%.o)

LOADERSRC=loader.asm
LOADEROBJECT=$(OBJDIR)/loader.o
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(OBJECTS_ASM) $(STATICLIBS)
	@echo "========================================"
	@echo "Kernel compiled successfully!"
ifeq ($(MM),USE_BUDDY)
	@echo "Memory Manager: BUDDY SYSTEM"
else
	@echo "Memory Manager: STANDARD"
endif
	@echo "========================================"

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

$(OBJDIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT): $(LOADERSRC)
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	rm -rf $(OBJDIR) *.bin

.PHONY: all clean

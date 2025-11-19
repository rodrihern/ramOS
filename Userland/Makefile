include Makefile.inc

# Activar advertencias de compilación
GCCFLAGS += -Wall

MODULE=0000-sampleCodeModule.bin
SAMPLE_DATA=0001-sampleDataModule.bin

OBJDIR := obj

# ============================================
# AGREGAR tests/ aquí
# ============================================
SOURCES=$(wildcard *.c)
SOURCES_SHELL=$(wildcard shell/*.c)
SOURCES_USRLIB=$(wildcard usrlib/*.c)
# Recursive wildcard helper for nested program sources (returns files + dirs)
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# Recursively include all C sources under programs/ (filter to keep only *.c, avoid dirs getting into OBJECTS and being rm -rf'ed)
SOURCES_PROGRAMS := $(filter %.c,$(call rwildcard, programs/, *.c))
SOURCES_TESTS=$(wildcard tests/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)

OBJECTS=$(SOURCES:%.c=$(OBJDIR)/%.o)
OBJECTS_SHELL=$(SOURCES_SHELL:%.c=$(OBJDIR)/%.o)
OBJECTS_USRLIB=$(SOURCES_USRLIB:%.c=$(OBJDIR)/%.o)
OBJECTS_PROGRAMS=$(SOURCES_PROGRAMS:%.c=$(OBJDIR)/%.o)
OBJECTS_TESTS=$(SOURCES_TESTS:%.c=$(OBJDIR)/%.o)
OBJECTS_ASM=$(SOURCES_ASM:%.asm=$(OBJDIR)/%.o)

# Unified deduplicated object list (avoids multiple inclusion of the same .o in link)
ALL_OBJECTS := $(sort $(OBJECTS) $(OBJECTS_SHELL) $(OBJECTS_USRLIB) $(OBJECTS_PROGRAMS) $(OBJECTS_TESTS) $(OBJECTS_ASM))

# Dynamically find any leftover objects inside programs/ (covers removed sources)
PROGRAMS_ORPHAN_OBJECTS := $(shell find programs -name '*.o' 2>/dev/null)

# ============================================
# AGREGAR OBJECTS_TESTS aquí
# ============================================
all: $(MODULE) sampleDataModule
rebuild: clean all

$(MODULE): $(ALL_OBJECTS)
	$(LD) $(LDFLAGS) -T sampleCodeModule.ld -o $(MODULE) $(ALL_OBJECTS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(GCC) $(GCCFLAGS) -I. -Iinclude -c $< -o $@

$(OBJDIR)/%.o: %.asm
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

# ============================================
# AGREGAR tests/*.o aquí
# ============================================
clean:
	find programs -name '*.o' -delete 2>/dev/null || true
	rm -rf $(OBJDIR) $(MODULE) $(SAMPLE_DATA)

sampleDataModule:
	printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

.PHONY: all clean sampleDataModule rebuild
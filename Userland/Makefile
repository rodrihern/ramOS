include Makefile.inc

# Activar advertencias de compilación
GCCFLAGS += -Wall

MODULE=0000-sampleCodeModule.bin
SAMPLE_DATA=0001-sampleDataModule.bin

OBJDIR := obj

# Recursive wildcard helper for nested program sources (returns files + dirs)
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

src_to_obj = $(OBJDIR)/$(subst /,_,$(basename $(1))).o

C_SOURCES := $(wildcard *.c)
C_SOURCES += $(wildcard shell/*.c)
C_SOURCES += $(wildcard usrlib/*.c)
C_SOURCES += $(wildcard tests/*.c)
C_SOURCES += $(filter %.c,$(call rwildcard, programs/, *.c))
C_SOURCES := $(sort $(C_SOURCES))

ASM_SOURCES := $(wildcard asm/*.asm)
ASM_SOURCES := $(sort $(ASM_SOURCES))

C_OBJECTS=$(foreach src,$(C_SOURCES),$(call src_to_obj,$(src)))
ASM_OBJECTS=$(foreach src,$(ASM_SOURCES),$(call src_to_obj,$(src)))
ALL_OBJECTS=$(sort $(C_OBJECTS) $(ASM_OBJECTS))

# ============================================
# AGREGAR OBJECTS_TESTS aquí
# ============================================
all: $(MODULE) sampleDataModule
rebuild: clean all

$(MODULE): $(ALL_OBJECTS)
	$(LD) $(LDFLAGS) -T sampleCodeModule.ld -o $(MODULE) $(ALL_OBJECTS)

define make-c-rule
$$(call src_to_obj,$(1)): $(1)
	@mkdir -p $(OBJDIR)
	$(GCC) $(GCCFLAGS) -I. -Iinclude -c $$< -o $$@
endef

define make-asm-rule
$$(call src_to_obj,$(1)): $(1)
	@mkdir -p $(OBJDIR)
	$(ASM) $(ASMFLAGS) $$< -o $$@
endef

$(foreach src,$(C_SOURCES),$(eval $(call make-c-rule,$(src))))
$(foreach src,$(ASM_SOURCES),$(eval $(call make-asm-rule,$(src))))

# ============================================
# AGREGAR tests/*.o aquí
# ============================================
clean:
	rm -rf $(OBJDIR) $(MODULE) $(SAMPLE_DATA)

sampleDataModule:
	printf "This is sample data." >> $(SAMPLE_DATA) && dd if=/dev/zero bs=1 count=1 >> $(SAMPLE_DATA)

.PHONY: all clean sampleDataModule rebuild

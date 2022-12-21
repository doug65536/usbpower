
# 1 Name prefix (like AVRC)
# 2 Toolchain prefix (like AVRC)
# 3 obj directory name fragment (like $(AVR_MCU))
# 4 no all
define extract_names=
$(2)_OBJDIR = obj-$(3)

$(1)_SOURCE_NAMES_CC = $$(filter %.cc,$$($(1)_SRCS))
$(1)_SOURCE_NAMES_S = $$(filter %.S,$$($(1)_SRCS))

$(1)_OBJECTS_CC = $$(patsubst %.cc,$$($(2)_OBJDIR)/%.o,$$($(1)_SOURCE_NAMES_CC))
$(1)_OBJECTS_S = $$(patsubst %.S,$$($(2)_OBJDIR)/%.o,$$($(1)_SOURCE_NAMES_S))

$(1)_OBJECTS_ALL = $$($(1)_OBJECTS_CC) $$($(1)_OBJECTS_S)

COMBINED_OBJECTS_ALL += $$($(1)_OBJECTS_ALL)

$(eval $(if $(subst 1,,$(4)), OBJECTS_ALL += $$($(1)_OBJECTS_ALL), ))
endef
#$(info new OBJECTS_ALL ($(1) $(2) $(3) $(4)): $(OBJECTS_ALL))

#$(eval $(call extract_names,BUILD,BUILD,host,0))

#COMBINED_OBJECTS_ALL = $$(OBJECTS_ALL) $$(BUILD_OBJECTS_ALL)
define combine_objects=
COMPILE_ONLY_ALL = $$(patsubst %.o,%.S,$$(OBJECTS_ALL))
PREPROCESS_ONLY_ALL = $$(patsubst %.o,%.i,$$(OBJECTS_ALL))

BUILD_COMPILE_ONLY_ALL = $$(patsubst %.o,%.S,$$(OBJECTS_ALL))
BUILD_PREPROCESS_ONLY_ALL = $$(patsubst %.o,%.i,$$(OBJECTS_ALL))


# Generated dependencies
DEPFILES = $$(patsubst %.o,%.d,$$(patsubst $$(SRC_DIR)/%,%,$$(COMBINED_OBJECTS_ALL)))
$$(DEPFILES):
endef

# $(1): filename
# $(2): extension
# $$($(3)): compiler
# $$($(4)): flags
# $$($(5)): OBJDIR
# $$($(6)): SRCDIR

# $(info Adding rule \
# 	for $$($(5))/$(patsubst %.$(2),%.o,$(1)) \
# 	from $$($(6))/$(1) \
# 	with $$($(3)))
define compile_extension=


# Preprocess, compile, and assemble to object file normally
$$($(5))/$(patsubst %.$(2),%.o,$(1)): $$($(6))/$(1)
	$(MKDIR) -p $$(@D)
	$$($(3)) -o $$@ -MMD $$($(4)) -c $$<

# Preprocess, compile, but do not assemble, and do not create object file
$$($(5))/$(patsubst %.$(2),%.S,$(1)): $$($(6))/$(1)
	$(MKDIR) -p $$(@D)
	$$($(3)) -o $$@ -MMD $$($(4)) -S $$<
	$(LESS) $$@

# Preprocess only, do not compile, do not assemble, do not create object file
$$($(5))/$(patsubst %.$(2),%.i,$(1)): $$($(6))/$(1)
	$(MKDIR) -p $$(@D)
	$$($(3)) -o $$@ -MMD $$($(4)) -E $$<
	$(LESS) $$@

$$($(5))/$(patsubst %.$(2),%.d,$(1)): $(patsubst %.$(2),%.o,$(1))

endef

# Host objects

define compile_targets=
$$(foreach file,$$($(1)_SOURCE_NAMES_CC), \
	$$(eval $$(call compile_extension,$$(file),cc,$(2)_CXX,$(2)_CXXFLAGS,$(2)_OBJDIR,SRCDIR)))

$$(foreach file,$$($(1)_SOURCE_NAMES_S), \
	$$(eval $$(call compile_extension,$$(file),S,$(2)_CXX,$(2)_CXXFLAGS,$(2)_OBJDIR,SRCDIR)))
endef

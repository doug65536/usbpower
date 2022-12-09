
define extract_names=
$(1)_SOURCE_NAMES_CC = $$(filter %.cc,$$($(1)_SRCS))
$(1)_SOURCE_NAMES_S = $$(filter %.S,$$($(1)_SRCS))

$(1)_OBJECTS_CC = $$(patsubst %.cc,$(OBJDIR)/%.o,$$($(1)_SOURCE_NAMES_CC))
$(1)_OBJECTS_S = $$(patsubst %.S,$(OBJDIR)/%.o,$$($(1)_SOURCE_NAMES_S))

$(1)_OBJECTS_ALL = $$($(1)_OBJECTS_CC) $$($(1)_OBJECTS_S)
endef

OBJDIR = obj-$(MCU)

SOURCE_NAMES_CC = $(filter %.cc,$(SRCS))
SOURCE_NAMES_S = $(filter %.S,$(SRCS))

OBJECTS_CC = $(patsubst %.cc,$(OBJDIR)/%.o,$(SOURCE_NAMES_CC))
OBJECTS_S = $(patsubst %.S,$(OBJDIR)/%.o,$(SOURCE_NAMES_S))
OBJECTS_ALL = $(OBJECTS_CC) $(OBJECTS_S)

$(eval $(call extract_names,BUILD))

COMPILE_ONLY_ALL = $(patsubst %.o,%.S,$(OBJECTS_ALL))
PREPROCESS_ONLY_ALL = $(patsubst %.o,%.i,$(OBJECTS_ALL))

BUILD_COMPILE_ONLY_ALL = $(patsubst %.o,%.S,$(OBJECTS_ALL))
BUILD_PREPROCESS_ONLY_ALL = $(patsubst %.o,%.i,$(OBJECTS_ALL))

COMBINED_OBJECTS_ALL = $(OBJECTS_ALL) $(BUILD_OBJECTS_ALL)

# Generated dependencies
DEPFILES = $(patsubst %.o,%.d,$(patsubst $(SRC_DIR)/%,%,$(COMBINED_OBJECTS_ALL)))
$(DEPFILES):

# $(1): filename
# $(2): extension
# $$($(3)): compiler
# $$($(4)): flags
define compile_extension=

# Preprocess, compile, and assemble to object file normally
$(OBJDIR)/$(patsubst %.$(2),%.o,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$$($(3)) -o $$@ -MMD $$($(4)) -c $$<

# Preprocess, compile, but do not assemble, and do not create object file
$(OBJDIR)/$(patsubst %.$(2),%.S,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$$($(3)) -o $$@ -MMD $$($(4)) -S $$<
	$(LESS) $$@

# Preprocess only, do not compile, do not assemble, do not create object file
$(OBJDIR)/$(patsubst %.$(2),%.i,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$$($(3)) -o $$@ -MMD $$($(4)) -E $$<
	$(LESS) $$@

$(OBJDIR)/$(patsubst %.$(2),%.d,$(1)): $(patsubst %.$(2),%.o,$(1))

endef

# Target objects

$(foreach file,$(SOURCE_NAMES_CC), \
	$(eval $(call compile_extension,$(file),cc,CXX,CXXFLAGS)))

$(foreach file,$(SOURCE_NAMES_S), \
	$(eval $(call compile_extension,$(file),S,CXX,CXXFLAGS)))

# Host objects

$(foreach file,$(BUILD_SOURCE_NAMES_CC), \
	$(eval $(call compile_extension,$(file),cc,BUILD_CXX,BUILD_CXXFLAGS)))

$(foreach file,$(BUILD_SOURCE_NAMES_S), \
	$(eval $(call compile_extension,$(file),S,BUILD_CXX,BUILD_CXXFLAGS)))

.SUFFIXES:

OBJDIR = obj-$(MCU)

SOURCE_NAMES_CC = $(filter %.cc,$(SRCS))
SOURCE_NAMES_S = $(filter %.S,$(SRCS))

OBJECTS_CC = $(patsubst %.cc,$(OBJDIR)/%.o,$(SOURCE_NAMES_CC))
OBJECTS_S = $(patsubst %.S,$(OBJDIR)/%.o,$(SOURCE_NAMES_S))

OBJECTS_ALL = $(OBJECTS_CC) $(OBJECTS_S)

COMPILE_ONLY_ALL = $(patsubst %.o,%.S,$(OBJECTS_ALL))
PREPROCESS_ONLY_ALL = $(patsubst %.o,%.i,$(OBJECTS_ALL))

# Generated dependencies
DEPFILES = $(patsubst %.o,%.d,$(patsubst $(SRC_DIR)/%,%,$(OBJECTS_ALL)))
$(DEPFILES):

define compile_extension=

# Preprocess, compile, and assemble to object file normally
$(OBJDIR)/$(patsubst %.$(2),%.o,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -MMD $(COMPILEFLAGS) $(CXXFLAGS) -c $$<

# Preprocess, compile, but do not assemble, and do not create object file
$(OBJDIR)/$(patsubst %.$(2),%.S,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -MMD $(COMPILEFLAGS) $(CXXFLAGS) -S $$<
	$(LESS) $$@

# Preprocess only, do not compile, do not assemble, do not create object file
$(OBJDIR)/$(patsubst %.$(2),%.i,$(1)): $(SRC_DIR)/$(1)
	mkdir -p $$(@D)
	$(CXX) -o $$@ -MMD $(COMPILEFLAGS) $(CXXFLAGS) -E $$<
	$(LESS) $$@

$(OBJDIR)/$(patsubst %.$(2),%.d,$(1)): $(patsubst %.$(2),%.o,$(1))

endef

$(foreach file,$(SOURCE_NAMES_CC), \
	$(eval $(call compile_extension,$(file),cc)))

$(foreach file,$(SOURCE_NAMES_S), \
	$(eval $(call compile_extension,$(file),S)))

# =======
# Основные пути и настройки
# =======

SRC_DIR ?= src
INC_DIR ?= include
RES_DIR ?= resources
LIB_DIR ?= libs
BIN_DIR ?= bin
OBJ_DIR ?= build

TARGET ?= debug
OUT ?= app

CC ?= gcc
CFLAGS += -Wall -Wextra -pedantic -std=c17 -g -fdiagnostics-color -Werror
LDFLAGS +=

PKG_CONFIG ?= pkg-config
PKGS += libsodium libcjson

EXTRA_CFLAGS +=
EXTRA_LDFLAGS +=

# =======
# Целевые конфигурации
# =======

ifneq ($(TARGET),release)
	CFLAGS += -g3 -O0 -DDEBUG
	LDFLAGS += -fsanitize=undefined,address,leak
else
	TARGET := release
	CFLAGS += -O2 -DNDEBUG -s -ffast-math
	LDFLAGS += -flto
endif

OBJ_DIR_NO_TARGET := $(OBJ_DIR)
BIN_DIR_NO_TARGET := $(BIN_DIR)

OBJ_DIR := $(OBJ_DIR)/$(TARGET)
BIN_DIR := $(BIN_DIR)/$(TARGET)

# =======
# Поиск файлов
# =======

LIB_SRC_DIRS := $(shell find $(LIB_DIR) -type d -name "$(SRC_DIR)" 2>/dev/null)
LIB_INCLUDE_DIRS := $(shell find $(LIB_DIR) -type d -name "$(INC_DIR)" 2>/dev/null)
LIB_RES_DIRS := $(shell find $(LIB_DIR) -type d -name "$(RES_DIR)" 2>/dev/null)

CFLAGS += $(foreach dir,$(LIB_INCLUDE_DIRS),-I$(dir))
CFLAGS += -I$(INC_DIR)

SRC_FILES := $(shell find $(SRC_DIR) $(LIB_SRC_DIRS) -name "*.c" -type f)
OBJ_FILES := $(foreach src,$(SRC_FILES),$(OBJ_DIR)/$(src:.c=.o))
RES_FILES := $(shell find $(RES_DIR) $(LIB_RES_DIR) -type f)

RES_TARGETS := $(patsubst $(RES_DIR)/%,$(BIN_DIR)/%,$(RES_FILES))

# =======
# Pkg-config зависимости
# =======

ifeq ($(PKGS),)
    PKG_CFLAGS :=
    PKG_LDFLAGS :=
else
    PKG_CFLAGS := $(shell $(PKG_CONFIG) --cflags $(PKGS))
    PKG_LDFLAGS := $(shell $(PKG_CONFIG) --libs $(PKGS))
endif

CFLAGS += $(PKG_CFLAGS) $(EXTRA_CFLAGS)
LDFLAGS += $(PKG_LDFLAGS) $(EXTRA_LDFLAGS)

# =======
# Правила сборки
# =======

all: $(BIN_DIR)/$(OUT) resources

debug: TARGET := debug
debug: all

release: TARGET := release
release: all

$(BIN_DIR)/$(OUT): $(OBJ_FILES) | $(BIN_DIR) $(OBJ_DIR)
	$(CC) $(OBJ_FILES) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $@

$(BIN_DIR):
	@mkdir -p $@

# =======
# Копирование ресурсов
# =======

resources: $(RES_TARGETS)

$(BIN_DIR)/%: $(RES_DIR)/% | $(BIN_DIR)
	cp $< $@

# =======
# Очистка
# =======

clean:
	rm -rf $(OBJ_DIR_NO_TARGET) $(BIN_DIR_NO_TARGET)

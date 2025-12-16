# Makefile
CC = gcc
CFLAGS = -Wall -pthread
# macOS n'a pas besoin de -lrt pour les sémaphores, mais Linux oui. 
# On laisse juste -pthread qui suffit généralement sur les deux.
LDFLAGS = -pthread

# Dossiers
SRC_DIR = src
OBJ_DIR = obj

# Fichiers
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
EXEC = ElevatorSimulation

# Règle par défaut
all: $(EXEC)

# Création de l'exécutable
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Compilation des fichiers objets
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -rf $(OBJ_DIR) $(EXEC)

re: clean all

.PHONY: all clean re
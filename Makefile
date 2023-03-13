NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
SANITIZE = -fsanitize=address -g
RM = rm -f

SRC = ServerConfig.cpp ServerConfigParser.cpp ServerLocationDirectiveConfig.cpp \
	  ServerNormalDirectiveConfig.cpp Socket.cpp BindingSocket.cpp ListeningSocket.cpp \
	  Server.cpp Request.cpp Response.cpp Log.cpp\

SERVER = Server.cpp
CONFIG = ServerConfig.cpp ServerConfigParser.cpp ServerLocationDirectiveConfig.cpp ServerNormalDirectiveConfig.cpp
LOG = Log.cpp
SOCKET = BindingSocket.cpp ListeningSocket.cpp Socket.cpp
REQUEST = Request.cpp
RESPONSE = Response.cpp

DIR_CONFIG = src/config
DIR_LOG = src/log
DIR_SERVER = src/server
DIR_REQUEST = $(DIR_SERVER)/request
DIR_RESPONSE = $(DIR_SERVER)/response
DIR_SOCKET = src/socket

SERVER_FILES = $(addprefix $(DIR_SERVER)/, $(SERVER))
CONFIG_FILES = $(addprefix $(DIR_CONFIG)/, $(CONFIG))
LOG_FILES = $(addprefix $(DIR_LOG)/, $(LOG))
REQUEST_FILES = $(addprefix $(DIR_REQUEST)/, $(REQUEST))
RESPONSE_FILES = $(addprefix $(DIR_RESPONSE)/, $(RESPONSE))
SOCKET_FILES = $(addprefix $(DIR_SOCKET)/, $(SOCKET))

MAIN = main.cpp

ALL_FILES := $(MAIN) $(SERVER_FILES) $(CONFIG_FILES) $(LOG_FILES) $(REQUEST_FILES) $(RESPONSE_FILES) $(SOCKET_FILES)
OBJECTS = $(ALL_FILES:.cpp=.o)
OBJ_DIR = obj/

$(NAME): $(OBJECTS)
	$(CC) -o $@ $^

all: $(NAME)

clean:
	$(RM) $(OBJECTS)

fclean: clean
	$(RM) $(NAME)

re: clean all

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

leaks: re
	$(CC) $(CFLAGS) $(MAIN) $(SRC) -o $(NAME) -D PRINT_MSG -D TEST_LEAKS
	./$(NAME)

test: re
	$(re) ./$(NAME)

old: clean
	$(CC) $(CFLAGS) $(SANITIZE) ./oldmain.cpp $(SRC) -o $(NAME)
	./$(NAME)

san: fclean 
	$(fclean)
	$(CC) $(CFLAGS) $(SANITIZE) $(MAIN) $(SRC) -o $(NAME)
	./$(NAME)

print: re
	$(CC) $(CFLAGS) $(SANITIZE) $(MAIN) $(SRC) -o $(NAME) -D PRINT_MSG
	./$(NAME)

.PHONY:	all clean fclean re test

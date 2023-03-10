NAME = webserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
SANITIZE = -fsanitize=address -g
RM = rm -f

SRC = ServerConfig.cpp ServerConfigParser.cpp ServerLocationDirectiveConfig.cpp \
	  ServerNormalDirectiveConfig.cpp Socket.cpp BindingSocket.cpp ListeningSocket.cpp \
	  Server.cpp Request.cpp Response.cpp Log.cpp\

MAIN = main.cpp

$(NAME):
	$(CC) $(CFLAGS) $(SANITIZE) $(MAIN) $(SRC) -o $(NAME)

all: $(NAME)

clean:
	$(RM) $(NAME)

fclean:
	$(RM) $(NAME)

re: clean all

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

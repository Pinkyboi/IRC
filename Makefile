NAME = irc

SRC_PATH = ./src
OBJ_PATH = ./obj

SRC_FILES = Server.c Client.c

OBJ_FILES = $(SRC_FILES:.c=.o)

SRC = $(addprefix $(SRC_PATH)/,$(SRC_FILES))
OBJ = $(addprefix $(OBJ_PATH)/,$(OBJ_FILES))

DEPS = 

INC = -Iinc

CC = g++ -std=c++98

CFLAGS = #-Wall -Werror -Wextra

all : $(NAME)

$(OBJ_PATH)/%.o : $(SRC_PATH)/%.c $(DEPS)
	@mkdir -p $(OBJ_PATH)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(NAME) : $(OBJ)
	$(CC) $(OBJ) -o $@

clean :
	make -C libft clean
	/bin/rm -rf $(OBJ_PATH) 2> /dev/null

fclean : clean
	make -C libft fclean
	/bin/rm -f $(NAME)

re : fclean all

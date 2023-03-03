NAME = irc

SRC_PATH = ./src
OBJ_PATH = ./obj

SRC_FILES = Server.cpp Client.cpp Channel.cpp

OBJ_FILES = $(SRC_FILES:.cpp=.o)

SRC = $(addprefix $(SRC_PATH)/,$(SRC_FILES))
OBJ = $(addprefix $(OBJ_PATH)/,$(OBJ_FILES))

DEPS = 

INC = -Iinc

CC = g++ -std=c++98

CFLAGS = #-Wall -Werror -Wextra

all : $(NAME)

$(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp $(DEPS)
	@mkdir -p $(OBJ_PATH)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(NAME) : $(OBJ)
	$(CC) $(OBJ) -o $@

clean :
	/bin/rm -rf $(OBJ_PATH) 2> /dev/null

fclean : clean
	/bin/rm -f $(NAME)

re : fclean all

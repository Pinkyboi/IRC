NAME = irc
CC = clang++ -std=c++98
CFLAGS = #-Wall -Werror -Wextra


SRC_FOLDER = src
INC_FOLDER = inc

OBJ_FOLDER = ./obj

CLASS_FOLDERS = Server\
				Client\
				Channel

SRC_FOLDERS = $(addprefix  $(SRC_FOLDER)/, $(CLASS_FOLDERS))

VPATH = $(SRC_FOLDERS)

SRC_FILES = Server.cpp\
			Client.cpp\
			Channel.cpp\
			CircularBuffer.cpp\
			main.cpp

OBJ_FILES = $(SRC_FILES:.cpp=.o)

CLASS_INC = $(addsuffix  /$(INC_FOLDER), $(SRC_FOLDERS))
INC = $(addprefix -I, $(CLASS_INC))
OBJ = $(addprefix $(OBJ_FOLDER)/,$(OBJ_FILES))

all : $(NAME)

$(OBJ_FOLDER)/%.o : %.cpp $(CLASS_INC)
	@mkdir -p $(OBJ_FOLDER)
	@echo "Compiling $< ..."
	@$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(NAME) : $(OBJ)
	@echo "Linking $@ ..."
	@$(CC) $(OBJ) -o $@
	@echo "Done."

clean :
	@rm -rf $(OBJ_FOLDER)

fclean : clean
	@rm -f $(NAME)

re : fclean all

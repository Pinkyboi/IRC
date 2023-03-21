NAME = ircserv
CC = clang++ -std=c++98
CFLAGS = #-Wall -Werror -Wextra

SRC_FOLDER = src
INC_FOLDER = inc

OBJ_FOLDER = ./obj

CLASS_FOLDERS = Server\
				Client\
				Channel\
				Parser\
				CircularBuffer

SRC_FOLDERS = $(addprefix  $(SRC_FOLDER)/, $(CLASS_FOLDERS))

HDR_FOLDERS = $(addsuffix  /$(INC_FOLDER), $(SRC_FOLDERS))

VPATH = %.cpp $(SRC_FOLDERS) : %.hpp $(HDR_FOLDERS)

SRC_FILES =	Parser.cpp\
			Server.cpp\
			Client.cpp\
			Channel.cpp\
			CircularBuffer.cpp\
			main.cpp

HDR_FILES = Parser.hpp\
			Server.hpp\
			Client.hpp\
			Channel.hpp\
			CircularBuffer.hpp

OBJ_FILES = $(SRC_FILES:.cpp=.o)

# HDR_FOLDERS = $(addsuffix  /$(INC_FOLDER), $(SRC_FOLDERS))

INC = $(addprefix -I, $(HDR_FOLDERS))
OBJ = $(addprefix $(OBJ_FOLDER)/,$(OBJ_FILES))

all : $(NAME)

$(OBJ_FOLDER)/%.o : %.cpp $(HDR_FILES)
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

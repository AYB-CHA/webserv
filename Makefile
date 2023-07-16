NAME = webserv
HEADERS	= $(wildcard *.hpp) $(wildcard config/*.hpp) $(wildcard server/*.hpp) $(wildcard selector/*.hpp)
CLIENTFILES = Client.cpp
SERVERFILES = ABase.cpp ConfigChecker.cpp Location.cpp Server.cpp utils.cpp
CONFIGFILES	=	Config.cpp ConfigLexer.cpp ConfigParser.cpp Directive.cpp BlockDirective.cpp Token.cpp
SELECTORFILES	=	Selector.cpp
COREFILES = Core.cpp
CPPFILES = main.cpp $(addprefix config/, $(CONFIGFILES)) $(addprefix server/, $(SERVERFILES)) $(addprefix client/, $(CLIENTFILES)) $(addprefix core/, $(COREFILES)) $(addprefix selector/, $(SELECTORFILES))
OBJFILES = $(CPPFILES:%.cpp=%.o)
CFLAGS = -Wall -Wextra -Werror -Wno-implicit-fallthrough -std=c++98  # -g
CC = c++

all: $(NAME)

$(NAME): $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $(NAME)

%.o : %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJFILES)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

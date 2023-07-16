NAME = webserv
SERVERFILES = ABase.cpp ConfigChecker.cpp Location.cpp Server.cpp utils.cpp
CONFIGFILES	=	Config.cpp ConfigLexer.cpp ConfigParser.cpp Directive.cpp BlockDirective.cpp Token.cpp
CPPFILES = main.cpp $(addprefix config/, $(CONFIGFILES)) $(addprefix server/, $(SERVERFILES))
OBJFILES = $(CPPFILES:%.cpp=%.o)
CFLAGS = -Wall -Wextra -Werror -Wno-implicit-fallthrough -std=c++98  # -g
CC = c++

all: $(NAME)

$(NAME): $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $(NAME)

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJFILES)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

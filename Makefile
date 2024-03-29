NAME = webserv
HEADERS	= $(wildcard *.hpp) $(wildcard cgi/*.hpp) $(wildcard client/*.hpp) $(wildcard config/*.hpp) $(wildcard core/*.hpp) $(wildcard request/*.hpp) $(wildcard response/*.hpp) $(wildcard selector/*.hpp) $(wildcard server/*.hpp) $(wildcard utils/*.hpp)
CLIENTFILES = Client.cpp SendFileWrapper.cpp FormData.cpp
REQUESTFILES = HttpRequest.cpp HttpRequestParser.cpp RequestHandler.cpp
RESPONSEFILES = HttpResponseBuilder.cpp HttpResponseException.cpp Mime.cpp
UTILSFILES = string.cpp
SERVERFILES = AContext.cpp ConfigChecker.cpp Location.cpp Server.cpp utils.cpp
CONFIGFILES	=	Config.cpp ConfigLexer.cpp ConfigParser.cpp Directive.cpp BlockDirective.cpp Token.cpp
SELECTORFILES	=	Selector.cpp Mediator.cpp Multiplexer.cpp
CGIFILES = CgiResolver.cpp
COREFILES = Core.cpp
CPPFILES = main.cpp $(addprefix config/, $(CONFIGFILES)) $(addprefix server/, $(SERVERFILES)) $(addprefix client/, $(CLIENTFILES)) \
			$(addprefix core/, $(COREFILES)) $(addprefix selector/, $(SELECTORFILES)) $(addprefix request/, $(REQUESTFILES)) $(addprefix response/, $(RESPONSEFILES)) \
			$(addprefix utils/, $(UTILSFILES)) $(addprefix cgi/, $(CGIFILES))

OBJFILES = $(CPPFILES:%.cpp=%.o)
CFLAGS = -Wall -Wextra -Werror -std=c++98
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

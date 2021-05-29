SRCS	=	Config/BaseConfig.cpp Config/Config.cpp Config/Reader.cpp  Config/ServerConfig.cpp Config/ServerRoutes.cpp \
			Exceptions/CGIException.cpp Exceptions/ParsingException.cpp Exceptions/ParsingRequestException.cpp \
			RequestHandler/CGI.cpp RequestHandler/Envp.cpp RequestHandler/RequestHandler.cpp RequestHandler/RequestParser.cpp \
			Server/Cluster.cpp Server/Connection.cpp Server/Server.cpp Utils/Base64.cpp Utils/StringUtils.cpp \
			main.cpp

NAME	=	webserv

OBJS	=	$(SRCS:.cpp=.o)

CXX		=	clang++

CPPFLAGS	=	-Wall -Wextra -Werror  -I. -Iincludes -std=c++98

all:		$(NAME)

$(NAME):	$(OBJS)
			$(CXX) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean:
			rm -f $(OBJS)

fclean:		clean
			rm -f $(NAME)

re:			fclean all

.PHONY: 	all clean fclean re
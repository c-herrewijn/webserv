NAME = webserv
SOURCES = demo.cpp \
		  ./CGI.cpp \
		  ./server/HTTPparser.cpp \
		  ./server/WebServer.cpp \
		  ./parser/Config.cpp \
		  ./parser/Listen.cpp \
		  ./parser/Location.cpp \
		  ./parser/Parameters.cpp \
		  ./parser/Server.cpp \
		  ./parser/parser_utils.cpp

CONFIG = \"${PWD}/cgi_server.conf\"

$(NAME): $(SOURCES)
	c++ -DDEF_CONF=$(CONFIG) -I server -I parser $(SOURCES) -o $(NAME)

test: $(NAME)
	@./$(NAME) &

	@sleep 1
	@echo "start test..."
	@curl --http0.9 -d "myRequestFromCurl" localhost:8080

run: $(NAME)
	@./$(NAME) cgi_server.conf

.PHONY: test

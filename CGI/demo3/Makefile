NAME = server
SOURCES = demo.cpp HTTPparser.cpp WebServer.cpp

$(NAME): $(SOURCES)
	c++ $(SOURCES) -o $(NAME)

test: $(NAME)
	@./$(NAME) &

	@sleep 1
	@echo "start test..."
	@curl --http0.9 -d "myRequestFromCurl" localhost:8080

.PHONY: test

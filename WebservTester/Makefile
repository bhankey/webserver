	GOCMD=go
	GOBUILD=$(GOCMD) build
	GOCLEAN=$(GOCMD) clean
	BINARY_NAME=webserv_test

all: build run

build:
	$(GOBUILD) -o $(BINARY_NAME) -v

clean:
	$(GOCLEAN)
	rm -rf $(BINARY_NAME)
run:
	$(GOBUILD) -o $(BINARY_NAME)
	./$(BINARY_NAME)

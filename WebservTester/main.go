package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"
)

const (
	defaultServerPort = ":8080"
	defaultServerHost = "127.0.0.1"
)

const (
	None  = "\033[0m"
	Red   = "\033[0;31m"
	Green = "\033[0;32m"
)

type test struct {
	name               string
	method             string
	url                string
	sentBody           *strings.Reader
	wantBody           string
	wantFileBody       string
	expectedStatusCode int
	acceptLanguage     string
	contentType        string
	allow              []string
	specialCase        func(client *http.Client, r *http.Request, t test)
}

func FileReader(fileName string) string {
	file, err := ioutil.ReadFile(fileName)
	if err != nil {
		log.Fatalln(Red + "Failed to read file: " + fileName + None)
	}
	return string(file)
}

func unquoteCodePoint(s string) (string, error) {
	r, err := strconv.ParseInt(strings.TrimPrefix(s, "\\U"), 16, 32)
	return string(r), err
}

func main() {
	client := &http.Client{}
	argsWithoutProg := os.Args[1:]
	var defaultURL string
	if len(argsWithoutProg) == 0 {
		defaultURL = "http://" + defaultServerHost + defaultServerPort
	} else if len(argsWithoutProg) == 1 {
		defaultURL = "http://" + argsWithoutProg[0]
	} else {
		log.Fatalln("Not valid count of arguments")
	}
	fmt.Println(Green + "Get method Tests" + None)
	GetTests(client, defaultURL)
	fmt.Println(Green + "Head method Tests" + None)
	HeadTests(client, defaultURL)
	fmt.Println(Green + "Delete method Tests" + None)
	DeleteTests(client, defaultURL)
	fmt.Println(Green + "Post method Tests" + None)
	PostTests(client, defaultURL)
	fmt.Println(Green + "Put method Tests" + None)
	PutTests(client, defaultURL)
	fmt.Println(Green + "Options method Tests" + None)
	OptionsTests(client, defaultURL)
	fmt.Println(Green + "Special Tests" + None)
	SpecialTests(client, defaultURL)
}

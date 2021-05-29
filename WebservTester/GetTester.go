package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"regexp"
)

var GetCases = []test{
	{
		name:               "custom 404 error page",
		method:             http.MethodGet,
		url:                "/",
		sentBody:           nil,
		wantBody:           FileReader("errorPages/404.html"),
		expectedStatusCode: http.StatusNotFound,
		contentType:        "text/html",
	},
	{
		name:               "test php-cgi",
		method:             http.MethodGet,
		url:                "/cgi-bin",
		sentBody:           nil,
		wantBody:           "<html>\n<head>\n<title>Index.php</title>\n</head>\n<body>\n<header>\n    <nav>\n        <a href=\"index.php?page=main\">Main page</a> |\n        <a href=\"index.php?page=contacts\">Contacts</a>\n    </nav>\n</header>\n\n<div>\n    Just main page.\n</div>\n<footer>\n    footer of the site\n</footer>\n</body>\n</html>",
		expectedStatusCode: http.StatusOK,
		contentType:        "text/html",
	},
	{
		name:               "get franche version of index.html",
		method:             http.MethodGet,
		url:                "/html",
		sentBody:           nil,
		wantBody:           "<html>\n<head>\n  <meta charset=\"UTF-8\">\n  <title>French index html page</title>\n</head>\n<body>\n<div class=\"header\">\n  Welcome to French html page\n</div>\n<div class=\"content\">\n  <h1>baguette!</h1>\n</div>\n</body>\n</html>",
		expectedStatusCode: http.StatusOK,
		acceptLanguage:     "fr, ge",
		contentType:        "text/html",
	},
	{
		name:               "get russian version of index.html",
		method:             http.MethodGet,
		url:                "/html",
		sentBody:           nil,
		wantBody:           "<!DOCTYPE html>\n<html>\n  <head>\n   <title>Russian index html page</title>\n  </head>\n<body>\n  <div class=\"header\">\n   Welcome to Russian html page\n  </div>\n  <div class=\"content\">\n   <h1>Здравствуйте!</h1>\n  </div>\n</body>\n</html>",
		expectedStatusCode: http.StatusOK,
		acceptLanguage:     "ru, fr",
		contentType:        "text/html",
	},
	{
		name:               "get default version of index.html",
		method:             http.MethodGet,
		url:                "/html",
		sentBody:           nil,
		wantBody:           "<!DOCTYPE html>\n<html>\n<head>\n    <title>Default index html page</title>\n</head>\n<body>\n<div class=\"header\">\n    Welcome to default html page\n</div>\n<div class=\"content\">\n    <h1>Welcome!</h1>\n</div>\n</body>\n</html>",
		expectedStatusCode: http.StatusOK,
		acceptLanguage:     "en",
		contentType:        "text/html",
	},
	{
		name:               "forbidden get method",
		method:             http.MethodGet,
		url:                "/PostAndPut/sometext.txt",
		sentBody:           nil,
		wantBody:           "<h1>Error 405</h1><hr><p>Method Not Allowed</p>",
		expectedStatusCode: http.StatusMethodNotAllowed,
		contentType:        "text/html",
	},
	{
		name:               "get file without specifying the extension",
		method:             http.MethodGet,
		url:                "/html/index",
		sentBody:           nil,
		wantBody:           "<!DOCTYPE html>\n<html>\n<head>\n    <title>Default index html page</title>\n</head>\n<body>\n<div class=\"header\">\n    Welcome to default html page\n</div>\n<div class=\"content\">\n    <h1>Welcome!</h1>\n</div>\n</body>\n</html>",
		expectedStatusCode: http.StatusOK,
		contentType:        "text/html",
	},
}

func GetTests(client *http.Client, URL string) {
	for i, test := range GetCases {
		fmt.Printf("Test №%d: %s\n", i+1, test.name)

		request, err := http.NewRequest(test.method, URL+test.url, nil)
		if err != nil {
			log.Fatalln(Red + err.Error() + None)
		}
		if test.acceptLanguage != "" {
			request.Header.Set("Accept-Language", test.acceptLanguage)
		}
		response, err := client.Do(request)
		if err != nil {
			log.Fatalln(Red + err.Error() + None)
		}
		content, _ := ioutil.ReadAll(response.Body)
		body := string(content)
		isContentType, _ := regexp.MatchString(test.contentType+"*", response.Header.Get("Content-Type"))
		if body != test.wantBody || int64(len(test.wantBody)) != response.ContentLength {
			log.Fatal(Red + "Wrong body of respond" + None)
		} else if response.StatusCode != test.expectedStatusCode {
			log.Fatal(Red + "Wrong status code" + None)
		} else if !isContentType {
			log.Fatal(Red + "Wrong content type" + None)
		}
		s, err := unquoteCodePoint("\\U0001f197")
		if err == nil {
			fmt.Printf("%s\n", s)
		} else {
			fmt.Println("OK")
		}
	}
}

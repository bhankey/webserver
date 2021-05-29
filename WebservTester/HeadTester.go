package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"regexp"
)

var HeadCases = []test{
	{
		name:               "custom 404 error page",
		method:             http.MethodHead,
		url:                "/",
		sentBody:           nil,
		wantBody:           "",
		expectedStatusCode: http.StatusNotFound,
		acceptLanguage:     "",
		contentType:        "text/html",
	},
	{
		name:               "test php-cgi",
		method:             http.MethodHead,
		url:                "/cgi-bin",
		sentBody:           nil,
		expectedStatusCode: http.StatusOK,
		acceptLanguage:     "",
		contentType:        "text/html",
	},
	{
		name:               "get franche version of index.html",
		method:             http.MethodHead,
		url:                "/html",
		sentBody:           nil,
		wantBody:           "",
		expectedStatusCode: http.StatusOK,
		acceptLanguage:     "fr, ge",
		contentType:        "text/html",
	},
	{
		name:               "get russian version of index.html",
		method:             http.MethodHead,
		url:                "/html",
		sentBody:           nil,
		wantBody:           "",
		expectedStatusCode: http.StatusOK,
		acceptLanguage:     "ru, fr",
		contentType:        "text/html",
	},
	{
		name:               "get default version of index.html",
		method:             http.MethodHead,
		url:                "/html",
		sentBody:           nil,
		wantBody:           "",
		expectedStatusCode: http.StatusOK,
		acceptLanguage:     "en",
		contentType:        "text/html",
	},
	{
		name:               "forbidden head method",
		method:             http.MethodHead,
		url:                "/PostAndPut/sometext.txt",
		sentBody:           nil,
		wantBody:           "",
		expectedStatusCode: http.StatusMethodNotAllowed,
		contentType:        "text/html",
	},
	{
		name:               "get file without specifying the extension",
		method:             http.MethodHead,
		url:                "/html/index",
		sentBody:           nil,
		wantBody:           "",
		expectedStatusCode: http.StatusOK,
		contentType:        "text/html",
	},
}

func HeadTests(client *http.Client, URL string) {

	for i, test := range HeadCases {
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
		if body != "" {
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

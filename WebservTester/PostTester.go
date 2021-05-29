package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strings"
)

var PostCases = []test{
	{
		name:               "create new resource",
		method:             http.MethodPost,
		url:                "/PostAndPut/post.txt",
		sentBody:           strings.NewReader("aaaaaaaaaaaaaaaaaaaa"),
		wantFileBody:       "aaaaaaaaaaaaaaaaaaaa",
		expectedStatusCode: http.StatusCreated,
	},
	{
		name:               "append to existent resources",
		method:             http.MethodPost,
		url:                "/PostAndPut/post.txt",
		sentBody:           strings.NewReader("aaaaaaaaaaaaaaaaaaaa"),
		wantFileBody:       "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
		expectedStatusCode: http.StatusOK,
	},
	{
		name:               "request entity too large",
		method:             http.MethodPost,
		url:                "/PostAndPut/rand.txt",
		sentBody:           strings.NewReader(strings.Repeat("f", 501)),
		wantFileBody:       "",
		expectedStatusCode: http.StatusRequestEntityTooLarge,
	},
	{
		name:               "forbidden post method",
		method:             http.MethodPost,
		url:                "/html/post.html",
		sentBody:           strings.NewReader(strings.Repeat("f", 100)),
		wantBody:           "<h1>Error 405</h1><hr><p>Method Not Allowed</p>",
		expectedStatusCode: http.StatusMethodNotAllowed,
	},
	{
		name:               "create file in non exist directory",
		method:             http.MethodPost,
		url:                "/PostAndPut/new/text",
		sentBody:           strings.NewReader(strings.Repeat("f", 100)),
		wantFileBody:       strings.Repeat("f", 100),
		wantBody:           "",
		expectedStatusCode: http.StatusCreated,
	},
	{
		name:               "create file based on Content-Type",
		method:             http.MethodPost,
		url:                "/PostAndPut",
		sentBody:           strings.NewReader(strings.Repeat("n", 100)),
		wantFileBody:       strings.Repeat("n", 100),
		wantBody:           "",
		expectedStatusCode: http.StatusCreated,
		specialCase:        CreateFileBasedOnContentType,
	},
}

func CreateFileBasedOnContentType(client *http.Client, r *http.Request, t test) {

	r.Header.Add("Content-Type", "text/html")
	response, err := client.Do(r)
	if err != nil {
		log.Fatalln(Red + err.Error() + None)
	}
	file, _ := ioutil.ReadFile("./PostAndPut/PostAndPut.html")
	if response.StatusCode != t.expectedStatusCode {
		log.Fatal(Red + "Wrong status code" + None)
	}
	if string(file) != t.wantFileBody {
		log.Fatal(Red + "Wrong resource" + None)
	}

}

func PostTests(client *http.Client, URL string) {
	_ = os.Remove("./PostAndPut/post.txt")
	_ = os.Remove("./PostAndPut/new/text")
	_ = os.Remove("./PostAndPut/PostAndPut.html")
	for i, test := range PostCases {
		fmt.Printf("Test №%d: %s\n", i+1, test.name)
		request, err := http.NewRequest(test.method, URL+test.url, test.sentBody)
		if err != nil {
			log.Fatalln(Red + err.Error() + None)
		}
		if test.specialCase == nil {
			response, err := client.Do(request)
			if err != nil {
				log.Fatalln(Red + err.Error() + None)
			}
			file, _ := ioutil.ReadFile("./" + test.url)
			content, _ := ioutil.ReadAll(response.Body)
			body := string(content)
			if test.wantBody == "" && string(file) != test.wantFileBody || test.wantBody != "" && body != test.wantBody {
				log.Fatal(Red + "Wrong resource" + None)
			} else if response.StatusCode != test.expectedStatusCode {
				log.Fatal(Red + "Wrong status code" + None)
			}
		} else {
			test.specialCase(client, request, test)
		}
		s, err := unquoteCodePoint("\\U0001f197")
		if err == nil {
			fmt.Printf("%s\n", s)
		} else {
			fmt.Println("OK")
		}
	}
}

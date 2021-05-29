package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"strings"
)

var PutCases = []test{
	{
		name:               "create new resource",
		method:             http.MethodPut,
		url:                "/PostAndPut/put.txt",
		sentBody:           strings.NewReader(strings.Repeat("a", 500)),
		wantFileBody:       strings.Repeat("a", 500),
		expectedStatusCode: http.StatusCreated,
	},
	{
		name:               "rewrite created resource",
		method:             http.MethodPut,
		url:                "/PostAndPut/put.txt",
		sentBody:           strings.NewReader(strings.Repeat("c", 500)),
		wantFileBody:       strings.Repeat("c", 500),
		expectedStatusCode: http.StatusNoContent,
	},
	{
		name:               "request entity too large",
		method:             http.MethodPut,
		url:                "/PostAndPut/rand.txt",
		sentBody:           strings.NewReader(strings.Repeat("f", 501)),
		wantFileBody:       "",
		expectedStatusCode: http.StatusRequestEntityTooLarge,
	},
	{
		name:               "forbidden put method",
		method:             http.MethodPut,
		url:                "/html/put.html",
		sentBody:           strings.NewReader(strings.Repeat("f", 100)),
		wantBody:           "<h1>Error 405</h1><hr><p>Method Not Allowed</p>",
		expectedStatusCode: http.StatusMethodNotAllowed,
	},
	{
		name:               "create file in non exist directory",
		method:             http.MethodPut,
		url:                "/PostAndPut/put/text",
		sentBody:           strings.NewReader(strings.Repeat("f", 100)),
		wantFileBody:       strings.Repeat("f", 100),
		wantBody:           "",
		expectedStatusCode: http.StatusCreated,
	},
	{
		name:               "put on many representation of resource",
		method:             http.MethodPut,
		url:                "/PostAndPut/representations/index.html",
		sentBody:           strings.NewReader(strings.Repeat("f", 100)),
		wantFileBody:       strings.Repeat("f", 100),
		expectedStatusCode: http.StatusNoContent,
		specialCase:        PutToManyRepresentations,
	},
}

func PutToManyRepresentations(client *http.Client, r *http.Request, t test) {
	_, _ = os.Create("./PostAndPut/representations/index.html")
	_, _ = os.Create("./PostAndPut/representations/index.html.fr")
	_, _ = os.Create("./PostAndPut/representations/index.html.ru")
	response, err := client.Do(r)
	if err != nil {
		log.Fatalln(Red + err.Error() + None)
	}
	if response.StatusCode != t.expectedStatusCode {
		log.Fatalln(Red + "Wrong status code" + None)
	}
	if _, err = os.Stat("./PostAndPut/representations/index.html"); os.IsNotExist(err) {
		log.Fatalln(Red + "Main representation of file wasn't create" + None)
	}
	if _, err = os.Stat("./PostAndPut/representations/index.html.fr"); os.IsExist(err) {
		log.Fatalln(Red + "old representation of file wasn't delete" + None)
	}
	if _, err = os.Stat("./PostAndPut/representations/index.html.ru"); os.IsExist(err) {
		log.Fatalln(Red + "old representation of file wasn't delete" + None)
	}
}

func PutTests(client *http.Client, URL string) {
	_ = os.Remove("./PostAndPut/put.txt")
	_ = os.Remove("./PostAndPut/put/text")
	for i, test := range PutCases {
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

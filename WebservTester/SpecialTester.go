package main

import (
	"fmt"
	"log"
	"net/http"
)

var SpecialCases = []test{
	{
		name:               "not implemented method",
		method:             "LOL",
		url:                "/",
		sentBody:           nil,
		wantBody:           "",
		expectedStatusCode: http.StatusNotImplemented,
		specialCase:        NotExistentMethod,
	},
}

func NotExistentMethod(client *http.Client, r *http.Request, t test) {
	response, err := client.Do(r)
	if err != nil {
		log.Fatalln(Red + err.Error() + None)
	}
	if response.StatusCode != t.expectedStatusCode {
		log.Fatalln(Red + "Wrong status code" + None)
	}
}

func SpecialTests(client *http.Client, URL string) {

	for i, test := range SpecialCases {
		fmt.Printf("Test №%d: %s\n", i+1, test.name)
		request, err := http.NewRequest(test.method, URL+test.url, nil)
		if err != nil {
			log.Fatalln(Red + err.Error() + None)
		}
		test.specialCase(client, request, test)
		s, err := unquoteCodePoint("\\U0001f197")
		if err == nil {
			fmt.Printf("%s\n", s)
		} else {
			fmt.Println("OK")
		}
	}
}

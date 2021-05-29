package main

import (
	"fmt"
	"log"
	"net/http"
	"reflect"
	"sort"
	"strings"
)

var OptionsCases = []test{
	{
		name:               "Info about /",
		method:             http.MethodOptions,
		url:                "/",
		expectedStatusCode: http.StatusOK,
		allow:              []string{"OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE", "CONNECT"},
	},
	{
		name:               "Info about /html",
		method:             http.MethodOptions,
		url:                "/html",
		expectedStatusCode: http.StatusOK,
		allow:              []string{"GET", "OPTIONS", "HEAD"},
	},
	{
		name:               "Info about /Configurations",
		method:             http.MethodOptions,
		url:                "/Configurations",
		expectedStatusCode: http.StatusMethodNotAllowed,
		allow:              []string{"GET"},
	},
}

func OptionsTests(client *http.Client, URL string) {

	for i, test := range OptionsCases {
		sort.Strings(test.allow)
		fmt.Printf("Test №%d: %s\n", i+1, test.name)
		request, err := http.NewRequest(test.method, URL+test.url, nil)
		if err != nil {
			log.Fatalln(Red + err.Error() + None)
		}
		response, err := client.Do(request)
		if err != nil {
			log.Fatalln(Red + err.Error() + None)
		}
		responseAllow := response.Header.Get("Allow")
		responseAllowSlice := strings.Split(strings.ReplaceAll(responseAllow, " ", ""), ",")
		sort.Strings(responseAllowSlice)

		if reflect.DeepEqual(test.allow, responseAllowSlice) == false {
			log.Fatal(Red + "Wrong allow header of response" + None)
		} else if response.StatusCode != test.expectedStatusCode {
			log.Fatal(Red + "Wrong status code" + None)
		}
		s, err := unquoteCodePoint("\\U0001f197")
		if err == nil {
			fmt.Printf("%s\n", s)
		} else {
			fmt.Println("OK")
		}
	}
}

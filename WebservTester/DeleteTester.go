package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"regexp"
)

var DeleteCases = []test{
	{
		name:               "Delete available resource",
		method:             http.MethodDelete,
		url:                "/Delete/tempFile",
		sentBody:           nil,
		expectedStatusCode: http.StatusNoContent,
	},
	{
		name:               "Delete non-existing resource",
		method:             http.MethodDelete,
		url:                "/Delete/NotExists",
		sentBody:           nil,
		expectedStatusCode: http.StatusNoContent,
	},
	{
		name:               "Delete empty directory",
		method:             http.MethodDelete,
		url:                "/Delete/empty_dir",
		specialCase:        DeleteEmptyDirTest,
		expectedStatusCode: http.StatusNoContent,
	},
	{
		name:               "Delete not empty directory",
		method:             http.MethodDelete,
		url:                "/Delete",
		specialCase:        DeleteNotEmptyDir,
		expectedStatusCode: http.StatusConflict,
	},
}

func DeleteEmptyDirTest(client *http.Client, r *http.Request, t test) {
	dirPath := "." + t.url
	if _, err := os.Stat(dirPath); os.IsNotExist(err) {
		_ = os.Mkdir(dirPath, 0777)
	}
	response, err := client.Do(r)
	if err != nil {
		log.Fatalln(Red + err.Error() + None)
	}
	if response.StatusCode != t.expectedStatusCode {
		log.Fatalln(Red + "Wrong status code" + None)
	} else if _, err = os.Stat(dirPath); os.IsExist(err) {
		log.Fatalln(Red + "Directory hasn't been removed" + None)
	}
}

func DeleteNotEmptyDir(client *http.Client, r *http.Request, t test) {
	dirPath := "./" + t.url
	response, err := client.Do(r)
	if err != nil {
		log.Fatalln(Red + err.Error() + None)
	}
	if response.StatusCode != t.expectedStatusCode {
		log.Fatalln(Red + "Wrong status code" + None)
	} else if _, err = os.Stat(dirPath); os.IsNotExist(err) {
		log.Fatalln(Red + "Not empty directory has been removed" + None)
	}
}

func DeleteTests(client *http.Client, URL string) {
	for i, test := range DeleteCases {
		fmt.Printf("Test №%d: %s\n", i+1, test.name)
		request, err := http.NewRequest(test.method, URL+test.url, nil)
		if err != nil {
			log.Fatalln(Red + err.Error() + None)
		}
		if test.specialCase == nil {
			if isMatch, _ := regexp.MatchString("\\*NotExists\\*", test.url); isMatch == false {
				pathToFile := test.url[1:]
				pathToFile = "./" + pathToFile
				if _, err := os.Stat(pathToFile); err != nil {
					_, err := os.Create(pathToFile)
					if err != nil {
						log.Fatalln(Red + "Failed to create file for test" + None)
					}
				}
			}
			response, err := client.Do(request)
			if err != nil {
				log.Fatalln(Red + err.Error() + None)
			}
			if response.StatusCode != test.expectedStatusCode {
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

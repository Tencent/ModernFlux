package main

import (
	"Demo/src/flux"
	"fmt"
	"log"
	"strings"
	"time"
)

var rlAPI flux.RateLimitAPI

type fluxStat struct {
	pass      int64
	refuse    int64
	setRefuse int64
}

func main() {

	var namespace string
	var service string
	var rateLimitKeyStr string
	var err error

	namespace = "Production"
	service = "Test"
	rateLimitKeyStr = "namespace1_service1"


	var (
		callerNamespace []string
		callerService   []string
		numKeys         int
	)

	rateLimitKeyList := strings.Split(rateLimitKeyStr, ",")
	numKeys = len(rateLimitKeyList)
	for _, keyStr := range rateLimitKeyList {
		keyList := strings.Split(keyStr, "_")
		if len(keyList) != 2 {
			log.Fatalf("invalid rate limit key format, need: namesapce_service , get: %s", keyStr)
		}
		callerNamespace = append(callerNamespace, keyList[0])
		callerService = append(callerService, keyList[1])
	}

	rlAPI, err = flux.NewRateLimitAPI(namespace, service)
	fmt.Println(rlAPI, err)
	if err != nil {
		log.Fatal(err.Error())
	}

	stat := make([]fluxStat, numKeys)
	rstStr := []string{"pass", "pass", "key refuse", "set refuse"}
	timeout := time.After(time.Second * 1200)
	finish := make(chan bool)
	count := 1
	go func() {
		for {
			select {
			case <-timeout:
				fmt.Println("timeout")
				finish <- true
				return
			default:
				index := count % numKeys
				rst := rlAPI.CheckQuota(callerNamespace[index], callerService[index])
				if rst < 2 {
					stat[index].pass += 1
				} else if rst == 2 {
					stat[index].refuse += 1
				} else if rst == 3 {
					stat[index].setRefuse += 1
				}
				log.Printf("round: %d, namespace: %s, key: %s, result: %s\n",
					count, callerNamespace[index], callerService[index], rstStr[rst])
				count++
			}
			time.Sleep(10 * time.Millisecond)
		}
	}()
	<-finish
	fmt.Println("Finish")
	log.Printf("total stat: %+v\n", stat)
}

package l5

import (
	"testing"
)

func TestWeightedRoundRobin(t *testing.T) {
	api, err := NewDefaultApi()
	if err != nil {
		t.Error(err)
		return
	}
	count := make(map[string]int);
	for i := 0; i < 100000; i++ {
		srv, err := api.GetServerBySid(64255937, 4915200)
		if err != nil {
			t.Error(err)
			return
		}
		if _, exists := count[srv.Ip.String()]; !exists {
			count[srv.Ip.String()] = 0
		}
		count[srv.Ip.String()]++
	}
	for k, v := range count {
		t.Logf("%s\t%d\n", k, v);
	}
}
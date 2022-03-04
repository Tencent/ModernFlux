package flux

import (
	"log"
	"os"
	"testing"
)

var (
	rlAPI RateLimitAPI
	_     = TestMain
	_     = BenchmarkCheckQuota
)

var BzNameSpace = []string{"1", "2", "3", "4"}
var Key = []string{"service1", "service2", "service3", "service4"}

//CheckQuota性能测试
func BenchmarkCheckQuota(b *testing.B) {
	b.RunParallel(func(pb *testing.PB) {
		for i := 0; pb.Next(); i++ {
			i = i % len(BzNameSpace)
			index := i % len(BzNameSpace)
			rlAPI.CheckQuota(BzNameSpace[index], Key[index])
		}
	})
	//for i := 0; i < b.N; i++ {
	//
	//}
}

func TestMain(m *testing.M) {

	_, err := NewRateLimitAPI("Production", "Test")
	if err != nil {
		log.Fatal(err.Error())
	}

	os.Exit(m.Run())
}

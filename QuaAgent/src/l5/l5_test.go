package l5

import "testing"

func TestApi_GetServerByName(t *testing.T) {
	api, err := NewDefaultApi()
	if err != nil {
		t.Errorf("NewDefaultApi Errof:%s", err.Error())
		return
	} else {
		t.Logf("NewDefaultApi Success")
	}
	name := "prot_proxy"
	//name := "swoole_cgi_x6m5_logic_test" //@todo not work
	domain, err := api.Query(name)
	if err != nil {
		t.Errorf("Api.Query(%s) Errof:%s", name, err.Error())
		return
	} else {
		t.Logf("Api.Query(%s) Mod:%d Cmd:%d", name, domain.Mod, domain.Cmd)
	}
}

func TestApi_GetServerBySid(t *testing.T) {
	api, err := NewDefaultApi()
	if err != nil {
		t.Errorf("NewDefaultApi Errof:%s", err.Error())
		return
	} else {
		t.Logf("NewDefaultApi Success")
	}
	mod := 64255937
	cmd := 5046272
	srv, err := api.GetServerBySid(int32(mod), int32(cmd))
	if err != nil {
		t.Errorf("Api.GetServerBySid Errof:%s", err.Error())
		return
	} else {
		t.Logf("Api.GetServerBySid(Mod:%d, Cmd:%d) Ip:%s Port:%d", mod, cmd, srv.Ip.String(), srv.Port)
	}

}

/*
func TestApiGetRouteBySid_And_ApiRouteResultUpdate(t *testing.T) {
	name := "prot_proxy"
	srv, err := ApiGetRouteBySid(name)
	if err != nil {
		t.Error("ApiGetRouteBySid Errof:%s", err.Error())
		return
	}
	err = ApiRouteResultUpdate(srv, 0, 0)
	if err != nil {
		t.Error("ApiRouteResultUpdate Errof:%s", err.Error())
		return
	}
}
*/

func BenchmarkApi_GetServerBySid(b *testing.B) {
	//b.StopTimer()
	api, err := NewDefaultApi()
	if err != nil {
		b.Error(err)
		return
	}
	//b.StartTimer()
	for i := 0; i < b.N; i++ {
		_, err = api.GetServerBySid(64255937, 5046272)
		if err != nil {
			b.Error(err)
			return
		}
	}
}

func BenchmarkApi_GetServerByName(b *testing.B) {
	//b.StopTimer()
	api, err := NewDefaultApi()
	if err != nil {
		b.Error(err)
		return
	}
	//b.StartTimer()
	for i := 0; i < b.N; i++ {
		_, err = api.GetServerByName("prot_proxy")
		if err != nil {
			b.Error(err)
			return
		}
	}
}

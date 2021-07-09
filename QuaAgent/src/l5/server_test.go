package l5

import "testing"

func TestServer_StatUpdate(t *testing.T) {
	api, err := NewDefaultApi()
	if err != nil {
		t.Errorf("NewDefaultApi Errof:%s", err.Error())
		return
	} else {
		t.Logf("NewDefaultApi Success")
	}
	name := "prot_proxy"
	srv, err := api.GetServerByName(name)
	if err != nil {
		t.Errorf("Api.GetServerByName(%s) Errof:%s", name, err.Error())
		return
	} else {
		t.Logf("Api.GetServerByName(%s) Ip:%s Port:%d", name, srv.Ip.String(), srv.Port)
	}
	err = srv.StatUpdate(0, 1)
	if err != nil {
		t.Errorf("Server.StatUpdate() Errof:%s", err.Error())
		return
	} else {
		t.Logf("Server.StatUpdate() Success")
	}
}

func BenchmarkServer_StatUpdate(b *testing.B) {
	b.StopTimer()
	api, err := NewDefaultApi()
	if err != nil {
		b.Errorf("NewDefaultApi Errof:%s", err.Error())
		return
	}
	name := "prot_proxy"
	srv, err := api.GetServerByName(name)
	if err != nil {
		b.Errorf("Api.GetServerByName(%s) Errof:%s", name, err.Error())
		return
	}
	b.StartTimer()
	for i := 0; i < b.N; i++ {
		err = srv.StatUpdate(0, 1)
		if err != nil {
			b.Errorf("Server.StatUpdate() Errof:%s", err.Error())
			return
		}
	}
}

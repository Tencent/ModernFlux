#/bin/bash
go clean && go build qagent.go module.go master.go worker.go quota.go checkqos.go  pool.go 

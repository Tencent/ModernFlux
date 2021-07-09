<?php
/*
 * 此文件需在swoole启动之前加载
 * php版本7+
 * swoole版本2.0+ or 4.0+
 */

require_once 'protocolbuf/message/pb_message.php';
require_once 'protocolbuf/parser/pb_proto_push_interface.php';
require_once 'protocolbuf/parser/pb_proto_open_app_desc.php';


define('FC_LOG_FILENAME','/data/app/fc/log/fc.log');

FcApi::Init();

/*
 * 流控API类
 */
class FcApi {

    public static $fcsvrL5 = ['modId' => 64283201,'cmdId' => 327680];//流控服务端的L5

    public static $splqueue;//长连接的队列

    public static $local_table;

    //length 1,2,4,8 默认为4字节
    public static $local_table_columns = [
        'index'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'totalquota'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'localquota'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'localquotamax'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'localquotaver'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'restrainquota'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'updatetime'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>8, 'default'=>0],
        'now'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>8, 'default'=>0],
        'lastreport'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>8, 'default'=>0],
        'nowsum'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'maxper'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'minper'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'setquota'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'setsum'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'procquota'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'averq'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],
        'totalworkernum'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>2, 'default'=>0],
        'reportLock'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>2, 'default'=>0],
    ];
    public static $timesum_table;
    public static $timesum_table_columns = [
        'timekey'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>8, 'default'=>0],//时间key
        'sum'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],// 通过量
        'refuse'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0], //拒绝量
        'setrefuse'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],//set拒绝的量
    ];

    public static $qarray_table;
    public static $qarray_table_columns = [
        'localq'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],//时间key
    ];

    //set流控,单机访问总量计数表
    public static $setquot_table;
    public static $setquot_table_columns = [
        'set_now'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>8, 'default'=>0],//时间
        'set_nowsum'=>['type'=>swoole_table::TYPE_INT, 'lenth'=>4, 'default'=>0],//当前秒数量
    ];

    public static $cleantime = 86400;
    public static $lastcleanday = 0;

    //public static $s_quota_check=; // 流控函数调用次数
    //public static $s_quota_local_pass=; //通过的请求量
    //public static $s_quota_local_restrain=; // 配额超过10%，可调节$quota_float_percent的值
    //public static $s_quota_timeout=; // 调用aquota超时次数
    //public static $s_quota_net_err=; // 调用aquota网络失败次数
    //public static $s_clearlocalmap=; // 每天被清理的key数量
    //public static $s_quota_aver_overflow=; //使用平均数被限制的
    //public static $s_quota_take_aver=; //使用平均数通过的

    public static $num_per_index = 10;
    public static $num_per_min = 6;
    public static $report_period = 5;
    public static $quota_float_percent = 10;
    public static $min_hard_code = 1000000;
    public static $quota_query_cmd = 150;
    public static $quota_array_num = 12;
    public static $s_ratio = 100;

    public static $threshold_per_second=100; //当单机配额小于100，按100来限流
    public static $threshold_max_second=500; //未获取机器数量时，按单机500来限流
    public static $set_now=0;
    public static $set_nowsum=0;

    /*
     *初始化流控类
     *此方式仅在swoole服务启动之前执行
     */
    public static function Init(){
        self::$splqueue = new SplQueue();

        //每个key一行统计数据，申请8192，实际上单机上不太可能在线这么多key
        self::$local_table = new Swoole\Table(8192);
        foreach(self::$local_table_columns as $k=>$v){
            self::$local_table->column($k, $v['type'], $v['lenth']);
        }
        self::$local_table->create();

        //每10秒的统计量 存1分钟的 time mod 6 是流控key的6倍行数
        self::$timesum_table = new Swoole\Table(65536);
        foreach(self::$timesum_table_columns as $k=>$v){
            self::$timesum_table->column($k, $v['type'], $v['lenth']);
        }
        self::$timesum_table->create();

        //每个key存12次分配到的配额 time mod 6 是流控key的12倍行数
        self::$qarray_table = new Swoole\Table(131072);
        foreach(self::$qarray_table_columns as $k=>$v){
            self::$qarray_table->column($k, $v['type'], $v['lenth']);
        }
        self::$qarray_table->create();

        self::$setquot_table = new Swoole\Table(1024);
        foreach(self::$setquot_table_columns as $k=>$v){
            self::$setquot_table->column($k, $v['type'], $v['lenth']);
        }
        self::$setquot_table->create();
    }

    /*
     *配额管理
     *keyid：流量分配对象，可以是活动ID,活动ID+流程ID
     *totalquota：要分配的总量
     *l5_req：流控服务的L5
     */
    public static function CheckQuota($strkey, $totalquota, $l5_req=array()){
        //Attr_API(self::$s_quota_check,1);
        $ret = 0;
        $setret = 0;
        $now = time();

        $local = self::GetLocal($strkey);
        $local['strkey'] = $strkey;
        $deltaquota = 0;
        $perdelta = 0; // ms

        if(self::GetSetQuot('setquot','set_now') != $now){
            self::SetSetQuot('setquot',['set_now'=>$now,'set_nowsum'=>1]);
            self::$set_nowsum = 1;
        }else{
            self::$set_nowsum  = self::IncrSetQuot('setquot','set_nowsum');
        }

        if($now >= $local['updatetime']){
            // check total quota vary
            if($local['totalquota'] != $totalquota){
                if($local['totalquota'] !=0 ){
                    //修改了流控总量调整
                }
                $local['totalquota'] = $totalquota;
                self::SetLocal($strkey,['totalquota'=>$totalquota]);
            }
            if($now != $local['now']){
                $local['now'] = $now;

                if($local['nowsum'] > $local['maxper']){
                    $local['maxper'] = $local['nowsum'];
                }
                if($local['nowsum'] < $local['minper']){
                    $local['minper'] = $local['nowsum'];
                }
                $local['nowsum'] = 1;
                self::SetLocal($strkey,['now'=>$local['now'],'maxper'=>$local['maxper'],'minper'=>$local['minper'],'nowsum'=>$local['nowsum']]);
                //Attr_API(self::$s_quota_local_pass, 1);
            }else{
                // no quota
                $local['nowsum'] = self::IncrLocal($strkey, 'nowsum');
                //process heartbeat: the quota service is not available
                $heartbeat = ($now > $local['updatetime']) && (($now - $local['updatetime']) > 60*5);
                $threshold = $local['localquotaver'] < self::$threshold_per_second;
                if($heartbeat || $threshold){
                    if($heartbeat){
                        self::ClearLocalQarray($strkey);
                    }
                    self::ProcessByAver($local, $totalquota, $ret, $deltaquota);
                }else{
                    // heartbeat ok
                    $deltaquota = $local['localquotaver'] - $local['nowsum']; // for report
                    if($local['restrainquota'] < self::$threshold_per_second){
                        $local['restrainquota'] = self::$threshold_per_second;
                        self::SetLocal($strkey,['restrainquota' => $local['restrainquota']]);
                    }
                    if($local['restrainquota'] > 0 && $local['nowsum'] > $local['restrainquota']){
                        //Attr_API(self::$s_quota_local_restrain, 1);
                        $ret = 2;
                    }else{
                        if(($local['localquotaver'] >0) && ($local['nowsum'] > $local['localquotaver'])){
                            //Attr_API(self::$s_quota_local_pass, 1);
                            $ret = 1;
                        }else{
                            //Attr_API(self::$s_quota_local_pass, 1);
                            $ret = 0;
                        }
                    }
                }
            }
        }

        // SET
        if($ret < 2){
            if(self::QSetOverLoad($local)){
                $setret = 2;
            }
        }

        ///////////////////////////////
        // update local cache
        ///////////////////////////////
        $timekey = 0;
        $index = 0;
        self::GetTimeKey($now, $timekey, $index);
        $timekey = (int)$timekey;
        $index = (int)$index;
        $index = $index % self::$num_per_min;
        $timesum = self::GetLocalTimesum($strkey);
        //$sLog = 'timesum: '.var_export($timesum,true);
        //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
        if($timesum[$index]['timekey'] != $timekey){
            $timesum[$index]['timekey'] = $timekey;
            $timesum[$index]['sum'] = 0;
            $timesum[$index]['refuse'] = 0;
            $timesum[$index]['setrefuse'] = 0;
            if($ret<2){
                $timesum[$index]['sum'] = 1;
            }else{
                $timesum[$index]['refuse'] = 1;
            }

            if(2 == $setret){
                $timesum[$index]['setrefuse'] = 1;
            }
            self::SetOneTimesum($strkey,$index,$timesum[$index]);
        }else{
            if($ret<2){
                $timesum[$index]['sum'] = self::IncrOneTimesum($strkey,$index,'sum');
            }else{
                $timesum[$index]['refuse'] = self::IncrOneTimesum($strkey,$index,'refuse');
            }
            if(2 == $setret){
                $timesum[$index]['setrefuse'] = self::IncrOneTimesum($strkey,$index,'setrefuse');
            }
        }
        $delta = $now - $local['updatetime'];
        if($delta >= self::$report_period && self::GetLocal($strkey, 'reportLock') == 0 && self::IncrLocal($strkey, 'reportLock') == 1){
            // from quota server
            $CmdBase = new CmdBase();
            $CmdBase->set_type(BODY_TYPE::BODY_REQUEST_SIM);
            $CmdBase->set_cmd_id(self::$quota_query_cmd);
            $req = new QuotaReq();
            $req->set_key($strkey);
            $req->set_totalquota($totalquota);
            $req->set_localquota($local['localquota']);
            $req->set_id(1111);
            $req->set_nowsum($local['maxper']);
            $req->set_workernum(1);
            //$sLog = 'timesum: '.var_export($timesum,true);
            //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
            for($n=0; $n<self::$num_per_min; $n++){
                if($timesum[$n]['timekey'] > 0){
                    $item = new ReportItem();
                    $item->set_timekey($timesum[$n]['timekey']);
                    $item->set_reportnum($timesum[$n]['sum']);
                    $item->set_refuse($timesum[$n]['refuse']);
                    $item->set_setrefuse($timesum[$n]['setrefuse']);
                    $req->set_reportitem($n,$item);
                    //$sLog = 'reportitem sum '.$strkey.':'.$timesum[$n]['sum'].' refuse:'.$timesum[$n]['refuse'].' setrefuse:'.$timesum[$n]['setrefuse'];
                    //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
                }
            }

            $strreq = $req->SerializeToString();
            if($strreq){
                $CmdBase->set_body($strreq);
                $sendbuff = $CmdBase->SerializeToString();
                $l5_req = !empty($l5_req)? $l5_req : self::$fcsvrL5;
                $l5_time_out = 0.2;
                $l5_ret = L5ApiGetRoute($l5_req, $l5_time_out);
                if ($l5_ret < 0) {
                    //$sLog = 'l5_ret:'.$l5_ret.' reportLock:0';
                    //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
                    $TcpStream = false;
                }else{
                    $ip = $l5_req['hostIp'];
                    $port = $l5_req['hostPort'];
                    list($usec, $sec) = explode(' ' , microtime());
                    $start = bcadd($sec, $usec, 6);
                    $TcpStream = self::getConnet($ip, $port, 1);
                }
                //$sLog = "fc svr :$ip:$port";
                //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
                if(false === $TcpStream){
                    //$sLog = 'TcpStream:false  reportLock:0';
                    //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
                }else{
                    $sLen  = strlen($sendbuff);
                    $sendbuff = pack("N",$sLen).$sendbuff;
                    $outbuff = '';

                    $tcpRet = $TcpStream->send($sendbuff);
                    if($tcpRet === false){
                        if($TcpStream->errCode == 110) {
                            //Attr_API(self::$s_quota_timeout, 1);
                        }else{
                            //Attr_API(self::$s_quota_net_err, 1);
                        }
                        $TcpStream->close();
                        //$sLog = 'TcpStream:false  reportLock:0';
                        //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
                    }else{
                        $outbuff = $TcpStream->recv();
                        list($usec, $sec) = explode(' ' , microtime());
                        $end = bcadd($sec, $usec, 6);
                        $perdelta = bcsub($end, $start, 3) * 1000;

                        if($outbuff === false || $outbuff === ''){
                            if($TcpStream->errCode == 110) {
                                //Attr_API(self::$s_quota_timeout, 1);
                            }else{
                                //Attr_API(self::$s_quota_net_err, 1);
                            }
                            $TcpStream->close();
                            //$sLog = 'TcpStream:false  reportLock:0';
                            //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
                        }else{
                            self::freeConnet($TcpStream);
                            $outbuff = substr($outbuff,4);
                            $CmdBase = new CmdBase();
                            $CmdBase->ParseFromString($outbuff);
                            $QuotaRsp = new QuotaRsp();
                            $QuotaRsp->ParseFromString($CmdBase->body());
                            $setdata = [];
                            $local['updatetime'] = $now;
                            $setdata['updatetime'] = $local['updatetime'];
                            if(($QuotaRsp->key() == $strkey)){
                                //$sLog = 'QuotaRsp '.$strkey.' totalworkernum:'.$QuotaRsp->totalworkernum().' localq:'.$QuotaRsp->localq().' setquota:'.$QuotaRsp->setquota();
                                //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
                                $arrind = $local['index'] % self::$quota_array_num;
                                $local['index'] = self::IncrLocal($strkey, 'index');
                                $local['totalworkernum'] = $QuotaRsp->totalworkernum();
                                $setdata['totalworkernum'] = $local['totalworkernum'];
                                $localqarray = self::GetLocalQarray($strkey);
                                $localqarray[$arrind]['localq'] = $QuotaRsp->localq();
                                self::SetOneQarray($strkey,$arrind,['localq'=>$localqarray[$arrind]['localq']]);
                                //$sLog = 'localqarray: '.var_export($localqarray,true);
                                //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);

                                $local['localquotaver'] = self::GetAverQ($localqarray);
                                $setdata['localquotaver'] = $local['localquotaver'];
                                if($local['localquotaver'] > $local['localquotamaxaver']){
                                    $local['localquotamaxaver'] = $local['localquotaver'];
                                }
                                if($local['localquotaver'] < $local['localquotaminaver']){
                                    $local['localquotaminaver'] = $local['localquotaver'];
                                }
                                $setdata['localquotaminaver'] = $local['localquotaminaver'];

                                $local['restrainquota'] = (int) (($local['localquotaver'] * (100 + self::$quota_float_percent))/100);
                                $setdata['restrainquota'] = $local['restrainquota'];
                                if($local['localquota'] != $QuotaRsp->localq()){
                                    $local['localquota'] = $QuotaRsp->localq();
                                    $setdata['localquota'] = $local['localquota'];
                                }

                                // add set logic
                                $local['setquota'] = $QuotaRsp->setquota();
                                $local['setsum'] = $QuotaRsp->setsum();
                                $local['procquota'] = $QuotaRsp->procquota();

                                $setdata['setquota'] = $local['setquota'];
                                $setdata['setsum'] = $local['setsum'];
                                $setdata['procquota'] = $local['procquota'];
                            }

                            self::SetLocal($strkey, $setdata);
                        }
                    }
                }
            }
            self::SetLocal($strkey, ['reportLock'=>0]);
        }

        $nowmin = date('i',$now);
        if($nowmin != $local['lastreport']){
            $setdata = [];
            $setdata['maxper'] = 0;
            $setdata['minper'] = self::$min_hard_code;
            $setdata['lastreport'] = $nowmin;
            self::SetLocal($strkey, $setdata);
        }

        self::ClearTableData();

        if(2 == $setret){
            return $setret;
        }

        return $ret;
    }

    private static function GetLocal($strkey, $column='') {
        if($column){
            return self::$local_table->get($strkey, $column);
        }
        $local = self::$local_table->get($strkey);
        if($local === false){
            $local = [];
            foreach(self::$local_table_columns as $k=>$v){
                $local[$k] = $v['default'];
            }
            self::$local_table->set($strkey,$local);
        }
        return $local;
    }

    private static function SetLocal($strkey, $value) {
        return self::$local_table->set($strkey, $value);
    }

    private static function IncrLocal($strkey, $column) {
        return self::$local_table->incr($strkey, $column);
    }

    private static function GetLocalTimesum($strkey) {
        $timesum = [];
        for($i=0; $i<self::$num_per_min; $i++){
            $oneval = self::$timesum_table->get($strkey.'__'.$i);
            if($oneval === false){
                $oneval = [];
                foreach(self::$timesum_table_columns as $k=>$v){
                    $oneval[$k] = $v['default'];
                }
                self::$timesum_table->set($strkey.'__'.$i, $oneval);
            }
            $timesum[$i] = $oneval;
        }
        return $timesum;
    }

    private static function SetOneTimesum($strkey, $index,  $value) {
        return self::$timesum_table->set($strkey.'__'.$index, $value);
    }

    private static function IncrOneTimesum($strkey, $index,  $column) {
        return self::$timesum_table->incr($strkey.'__'.$index, $column);
    }

    private static function GetLocalQarray($strkey) {
        $qarray = [];
        for($i=0; $i<self::$quota_array_num; $i++){
            $oneval = self::$qarray_table->get($strkey.'__'.$i);
            if($oneval === false){
                $oneval = [];
                foreach(self::$qarray_table_columns as $k=>$v){
                    $oneval[$k] = $v['default'];
                }
                self::$qarray_table->set($strkey.'__'.$i, $oneval);
            }
            $qarray[$i] = $oneval;
        }
        return $qarray;
    }

    private static function SetOneQarray($strkey, $arrind,  $value) {
        return self::$qarray_table->set($strkey.'__'.$arrind, $value);
    }


    private static function GetSetQuot($strkey, $column='') {
        if($column){
            return self::$setquot_table->get($strkey, $column);
        }
        $setquot = self::$setquot_table->get($strkey);
        if($setquot === false){
            $setquot = [];
            foreach(self::$setquot_table_columns as $k=>$v){
                $setquot[$k] = $v['default'];
            }
            self::$setquot_table->set($strkey,$setquot);
        }
        return $setquot;
    }

    private static function SetSetQuot($strkey, $value) {
        return self::$setquot_table->set($strkey, $value);
    }

    private static function IncrSetQuot($strkey, $column) {
        return self::$setquot_table->incr($strkey, $column);
    }

    private static function ProcessByAver(&$local, $totalquota, &$ret, &$deltaquota){
        $averquota = 0;
        if($local['totalworkernum'] > 0){
            $averquota = $totalquota / $local['totalworkernum'];
        }else{
            $averquota = self::$threshold_max_second;
        }
        if($averquota < self::$threshold_per_second){
            $averquota = self::$threshold_per_second;
        }
        $deltaquota = $averquota - $local['nowsum']; // for report

        if($averquota > 0){
            $percent = $local['nowsum']*100/$averquota;
            if($percent >= 100 + self::$quota_float_percent){
                //Attr_API(self::$s_quota_aver_overflow, 1);
                $ret = 2;
            }else{
                if($percent >= 100){
                    $delta = $percent - 100;
                    if(self::HitProba($delta)){
                        //Attr_API(self::$s_quota_aver_overflow, 1);
                        $ret = 2;
                    }else{
                        //Attr_API(self::$s_quota_take_aver, 1);
                        $ret = 0;
                    }
                }else{
                    $ret = 0;
                }
            }
        }
        return true;
    }

    private static function ClearTableData(){
        $nowdate = date('Ymd');
        $nowTime = date('H');
        //每天下午4点清理一次
        if($nowTime == '16' && self::$lastcleanday < date('Ymd')){
            self::$lastcleanday = $nowdate;
        }else{
            return;
        }
        $clearNum = 0;
        foreach(self::$local_table as $strkey=>$local){
            //清除超过1天没有更新过的key（说明已经没有请求了,即使清除后又有请求也不会有问题，会重新上报）
            if($local['updatetime'] < time() - self::$cleantime){
                self::$local_table->del($strkey);
                for($i=0; $i<self::$num_per_min; $i++){
                    self::$timesum_table->del($strkey.'__'.$i);
                }
                for($i=0; $i<self::$quota_array_num; $i++){
                    self::$qarray_table->del($strkey.'__'.$i);
                }
                $clearNum ++;
            }
        }
        //Attr_API(self::$s_clearlocalmap, $clearNum);
    }

    private static function GetTimeKey($ttime, &$timekey, &$index){
        $left = $ttime%60;
        $index = (int)($left/self::$num_per_index);
        $base = $ttime - $left;
        $timekey = ($base + $index*self::$num_per_index);
    }

    private static function ClearLocalQarray($strkey){
        foreach(self::$qarray_table_columns as $k=>$v){
            $qarray[$k] = $v['default'];
        }
        for($i=0; $i<self::$quota_array_num; $i++){
            self::$qarray_table->set($strkey.'__'.$i, $qarray);
        }
    }

    private static function GetAverQ($qarray){
        $ret = 0;
        $num = 0;
        $quota = 0;
        for($i= 0; $i<self::$quota_array_num; $i++){
            if($qarray[$i]['localq'] > 0){
                $num ++;
                $quota += $qarray[$i]['localq'];
            }
        }

        if($num > 0){
            $ret = $quota / $num;
        }

        return (int)$ret;
    }

    private static function HitProba($target){
        if($target >= self::$s_ratio){
            return true;
        }
        $value = rand() % self::$s_ratio;
        if($value < $target){
            return true;
        }else{
            return false;
        }
    }

    private static function QSetValid(&$local){
        if($local['setquota']>0 && $local['setsum']>0){
            if(0 == $local['procquota']){
                if($local['totalworkernum'] >0){
                    $local['averq'] = $local['setquota']/$local['totalworkernum'];
                    self::SetLocal($local['strkey'], ['averq'=>$local['averq']]);
                }
            }
            if($local['procquota']>0 || $local['averq']>0){
                return true;
            }
        }
        return false;
    }

    private static function QSetOverLoad(&$local){// 0: suc1 :mid 2:over
        if(self::QSetValid($local)){
            $valid = self::$set_nowsum;
            $tmp = ($local['procquota']>0)?$local['procquota']:$local['averq'];
            $percent = (int) ($valid*100/$tmp);
            //$sLog = 'QSetValid percent'.$percent.' nowsum:'.$valid.' procquota:'. $tmp;
            //error_log(date('Y-m-d H:i:s').'pid:'.getmypid().$sLog."\n",3,FC_LOG_FILENAME);
            if($percent >= (100+self::$quota_float_percent)){
                return true;
            }
            if($percent >= 100){
                $delta = $percent - 100;
                return self::HitProba($delta);
            }
            return false;
        }
        return false;
    }

    private static function getConnet($ip, $port, $timeout){
        if(self::$splqueue->isEmpty()){
            $client = new Swoole\Coroutine\Client(SWOOLE_SOCK_TCP);
            $tcpRet = $client->connect($ip, $port, $timeout);
            if($tcpRet === false){
                $client->close();
                return $tcpRet;
            }
            return $client;
        }

        $client = self::$splqueue->dequeue(); //从队列获取连接
        if(false == $client->isConnected()){
            $client->close();
            $tcpRet = $client->connect($ip, $port, $timeout);
            if($tcpRet === false){
                $client->close();
                return $tcpRet;
            }
        }
        return $client;
    }

    private static function freeConnet($client){
        self::$splqueue->enqueue($client); //连接对象加入队列
    }
}
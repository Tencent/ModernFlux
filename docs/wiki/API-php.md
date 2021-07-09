## 1：要求版本 php版本7+ swoole版本2.0+ or 4.0+
## 2：使用说明

  include_once 'flux/phpFcApi/Fc/FcApi.class.php'; //必须在Swoole Server->start()前执行前加载流控API类文件
  
## 3：流量控制调用方法
/*
*配额管理
*keyid：流量分配对象，可以是活动ID,活动ID+流程ID;不要使用两个'_'作为分隔符，长度不超过20为宜
*totalquota：要分配的总量即每个key每秒请求量上限
*l5_req：流控服务的L5,默认值是ams的流控测试服务L5
*ret：返回值说明：
*0表示可以放行
*1表示达到配额
*2表示达到配额上浮的量，可以拒绝请求，上浮百分比FcApi::$threshold_per_second默认是10，可自行调整建议0——20
*/
$ret = FcApi::CheckQuota($strKey, $totalquota, $fcsvrL5);

## 4：示例
$strKeyArr = ['ame_77777_1111','ame_88888_1111','ame_99999_1111'];
$strKey = $strKeyArr[rand(0,2)];
$totalquota = 2000;
$test_l5 = array('modId' => 64283201,'cmdId' => 327680);//ams的流控测试服务L5
$ret = FcApi::CheckQuota($strKey, $totalquota, $test_l5);
if($ret == 2){
   return 'access limit';
}







* **Ҫ��汾**
php�汾7+
swoole�汾2.0+ or 4.0+

* **ʹ��˵��**
```php
include_once 'flux/phpFcApi/Fc/FcApi.class.php'; //������Swoole Server->start()ǰִ��ǰ��������API���ļ�
```

* **�������Ƶ��÷���**
```php
/*
*������
*keyid������������󣬿����ǻID,�ID+����ID;��Ҫʹ������'_'��Ϊ�ָ��������Ȳ�����20Ϊ��
*totalquota��Ҫ�����������ÿ��keyÿ������������
*l5_req�����ط����L5,Ĭ��ֵ��ams�����ز��Է���L5
*ret������ֵ˵����
*0��ʾ���Է���
*1��ʾ�ﵽ���
*2��ʾ�ﵽ����ϸ����������Ծܾ������ϸ��ٷֱ�FcApi::$threshold_per_secondĬ����10�������е�������0����20
*/
$ret = FcApi::CheckQuota($strKey, $totalquota, $fcsvrL5);
```

* **ʾ��**
```php
$strKeyArr = ['ame_77777_1111','ame_88888_1111','ame_99999_1111'];
$strKey = $strKeyArr[rand(0,2)];
$totalquota = 2000;
$test_l5 = array('modId' => 64283201,'cmdId' => 327680);//ams�����ز��Է���L5
$ret = FcApi::CheckQuota($strKey, $totalquota, $test_l5);
if($ret == 2){
   return 'access limit';
}
```
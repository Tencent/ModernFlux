<?php
class ReportItem extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBInt";
    $this->values["1"] = "";
    $this->fields["2"] = "PBInt";
    $this->values["2"] = "";
    $this->fields["3"] = "PBInt";
    $this->values["3"] = "";
    $this->fields["4"] = "PBInt";
    $this->values["4"] = "";
  }
  function timekey()
  {
    return $this->_get_value("1");
  }
  function set_timekey($value)
  {
    return $this->_set_value("1", $value);
  }
  function reportnum()
  {
    return $this->_get_value("2");
  }
  function set_reportnum($value)
  {
    return $this->_set_value("2", $value);
  }
  function refuse()
  {
    return $this->_get_value("3");
  }
  function set_refuse($value)
  {
    return $this->_set_value("3", $value);
  }
  function setrefuse()
  {
    return $this->_get_value("4");
  }
  function set_setrefuse($value)
  {
    return $this->_set_value("4", $value);
  }
}
class QuotaReq extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBString";
    $this->values["1"] = "";
    $this->fields["2"] = "PBString";
    $this->values["2"] = "";
    $this->fields["3"] = "ReportItem";
    $this->values["3"] = array();
    $this->fields["4"] = "PBInt";
    $this->values["4"] = "";
    $this->fields["5"] = "PBInt";
    $this->values["5"] = "";
    $this->fields["6"] = "PBString";
    $this->values["6"] = "";
    $this->fields["7"] = "PBInt";
    $this->values["7"] = "";
    $this->fields["8"] = "PBString";
    $this->values["8"] = "";
    $this->fields["9"] = "PBInt";
    $this->values["9"] = "";
    $this->fields["10"] = "PBInt";
    $this->values["10"] = "";
    $this->fields["11"] = "PBInt";
    $this->values["11"] = "";
    $this->fields["12"] = "PBString";
    $this->values["12"] = "";
  }
  function key()
  {
    return $this->_get_value("1");
  }
  function set_key($value)
  {
    return $this->_set_value("1", $value);
  }
  function referkey()
  {
    return $this->_get_value("2");
  }
  function set_referkey($value)
  {
    return $this->_set_value("2", $value);
  }
  function reportitem($offset)
  {
    return $this->_get_arr_value("3", $offset);
  }
  function add_reportitem()
  {
    return $this->_add_arr_value("3");
  }
  function set_reportitem($index, $value)
  {
    $this->_set_arr_value("3", $index, $value);
  }
  function remove_last_reportitem()
  {
    $this->_remove_last_arr_value("3");
  }
  function reportitem_size()
  {
    return $this->_get_arr_size("3");
  }
  function localquota()
  {
    return $this->_get_value("4");
  }
  function set_localquota($value)
  {
    return $this->_set_value("4", $value);
  }
  function id()
  {
    return $this->_get_value("5");
  }
  function set_id($value)
  {
    return $this->_set_value("5", $value);
  }
  function serial()
  {
    return $this->_get_value("6");
  }
  function set_serial($value)
  {
    return $this->_set_value("6", $value);
  }
  function totalquota()
  {
    return $this->_get_value("7");
  }
  function set_totalquota($value)
  {
    return $this->_set_value("7", $value);
  }
  function openid()
  {
    return $this->_get_value("8");
  }
  function set_openid($value)
  {
    return $this->_set_value("8", $value);
  }
  function uin()
  {
    return $this->_get_value("9");
  }
  function set_uin($value)
  {
    return $this->_set_value("9", $value);
  }
  function nowsum()
  {
    return $this->_get_value("10");
  }
  function set_nowsum($value)
  {
    return $this->_set_value("10", $value);
  }
  function workernum()
  {
    return $this->_get_value("11");
  }
  function set_workernum($value)
  {
    return $this->_set_value("11", $value);
  }
  function setkey()
  {
    return $this->_get_value("12");
  }
  function set_setkey($value)
  {
    return $this->_set_value("12", $value);
  }
}
class QuotaRsp extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBString";
    $this->values["1"] = "";
    $this->fields["2"] = "PBInt";
    $this->values["2"] = "";
    $this->fields["3"] = "PBInt";
    $this->values["3"] = "";
    $this->fields["4"] = "PBInt";
    $this->values["4"] = "";
    $this->fields["5"] = "PBString";
    $this->values["5"] = "";
    $this->fields["6"] = "PBInt";
    $this->values["6"] = "";
    $this->fields["7"] = "PBInt";
    $this->values["7"] = "";
    $this->fields["8"] = "PBInt";
    $this->values["8"] = "";
    $this->fields["9"] = "PBInt";
    $this->values["9"] = "";
    $this->fields["10"] = "PBInt";
    $this->values["10"] = "";
    $this->fields["11"] = "PBInt";
    $this->values["11"] = "";
    $this->fields["12"] = "PBInt";
    $this->values["12"] = "";
    $this->fields["13"] = "PBInt";
    $this->values["13"] = "";
    $this->fields["14"] = "PBInt";
    $this->values["14"] = "";
    $this->fields["15"] = "PBInt";
    $this->values["15"] = "";
    $this->fields["16"] = "PBInt";
    $this->values["16"] = "";
  }
  function key()
  {
    return $this->_get_value("1");
  }
  function set_key($value)
  {
    return $this->_set_value("1", $value);
  }
  function localq()
  {
    return $this->_get_value("2");
  }
  function set_localq($value)
  {
    return $this->_set_value("2", $value);
  }
  function totalq()
  {
    return $this->_get_value("3");
  }
  function set_totalq($value)
  {
    return $this->_set_value("3", $value);
  }
  function partnernum()
  {
    return $this->_get_value("4");
  }
  function set_partnernum($value)
  {
    return $this->_set_value("4", $value);
  }
  function serial()
  {
    return $this->_get_value("5");
  }
  function set_serial($value)
  {
    return $this->_set_value("5", $value);
  }
  function ipnum()
  {
    return $this->_get_value("6");
  }
  function set_ipnum($value)
  {
    return $this->_set_value("6", $value);
  }
  function alloctype()
  {
    return $this->_get_value("7");
  }
  function set_alloctype($value)
  {
    return $this->_set_value("7", $value);
  }
  function allocpartnum()
  {
    return $this->_get_value("8");
  }
  function set_allocpartnum($value)
  {
    return $this->_set_value("8", $value);
  }
  function alloctime()
  {
    return $this->_get_value("9");
  }
  function set_alloctime($value)
  {
    return $this->_set_value("9", $value);
  }
  function self()
  {
    return $this->_get_value("10");
  }
  function set_self($value)
  {
    return $this->_set_value("10", $value);
  }
  function total()
  {
    return $this->_get_value("11");
  }
  function set_total($value)
  {
    return $this->_set_value("11", $value);
  }
  function nowsum()
  {
    return $this->_get_value("12");
  }
  function set_nowsum($value)
  {
    return $this->_set_value("12", $value);
  }
  function totalworkernum()
  {
    return $this->_get_value("13");
  }
  function set_totalworkernum($value)
  {
    return $this->_set_value("13", $value);
  }
  function setquota()
  {
    return $this->_get_value("14");
  }
  function set_setquota($value)
  {
    return $this->_set_value("14", $value);
  }
  function setsum()
  {
    return $this->_get_value("15");
  }
  function set_setsum($value)
  {
    return $this->_set_value("15", $value);
  }
  function procquota()
  {
    return $this->_get_value("16");
  }
  function set_procquota($value)
  {
    return $this->_set_value("16", $value);
  }
}
class SetItem extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBString";
    $this->values["1"] = "";
    $this->fields["2"] = "PBInt";
    $this->values["2"] = "";
    $this->fields["3"] = "PBString";
    $this->values["3"] = array();
    $this->fields["4"] = "PBInt";
    $this->values["4"] = "";
    $this->fields["5"] = "PBInt";
    $this->values["5"] = "";
    $this->fields["6"] = "PBInt";
    $this->values["6"] = "";
    $this->fields["7"] = "PBInt";
    $this->values["7"] = "";
  }
  function setkey()
  {
    return $this->_get_value("1");
  }
  function set_setkey($value)
  {
    return $this->_set_value("1", $value);
  }
  function quota()
  {
    return $this->_get_value("2");
  }
  function set_quota($value)
  {
    return $this->_set_value("2", $value);
  }
  function iplist($offset)
  {
    $v = $this->_get_arr_value("3", $offset);
    return $v->get_value();
  }
  function append_iplist($value)
  {
    $v = $this->_add_arr_value("3");
    $v->set_value($value);
  }
  function set_iplist($index, $value)
  {
    $v = new $this->fields["3"]();
    $v->set_value($value);
    $this->_set_arr_value("3", $index, $v);
  }
  function remove_last_iplist()
  {
    $this->_remove_last_arr_value("3");
  }
  function iplist_size()
  {
    return $this->_get_arr_size("3");
  }
  function feaid()
  {
    return $this->_get_value("4");
  }
  function set_feaid($value)
  {
    return $this->_set_value("4", $value);
  }
  function refuseid()
  {
    return $this->_get_value("5");
  }
  function set_refuseid($value)
  {
    return $this->_set_value("5", $value);
  }
  function setrefuseid()
  {
    return $this->_get_value("6");
  }
  function set_setrefuseid($value)
  {
    return $this->_set_value("6", $value);
  }
  function realid()
  {
    return $this->_get_value("7");
  }
  function set_realid($value)
  {
    return $this->_set_value("7", $value);
  }
}
class SetMacInfo extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "SetItem";
    $this->values["1"] = array();
  }
  function itemlist($offset)
  {
    return $this->_get_arr_value("1", $offset);
  }
  function add_itemlist()
  {
    return $this->_add_arr_value("1");
  }
  function set_itemlist($index, $value)
  {
    $this->_set_arr_value("1", $index, $value);
  }
  function remove_last_itemlist()
  {
    $this->_remove_last_arr_value("1");
  }
  function itemlist_size()
  {
    return $this->_get_arr_size("1");
  }
}
class ProcReport extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBString";
    $this->values["1"] = "";
    $this->fields["2"] = "ReportItem";
    $this->values["2"] = array();
    $this->fields["3"] = "PBString";
    $this->values["3"] = "";
    $this->fields["4"] = "PBInt";
    $this->values["4"] = "";
  }
  function procid()
  {
    return $this->_get_value("1");
  }
  function set_procid($value)
  {
    return $this->_set_value("1", $value);
  }
  function proclist($offset)
  {
    return $this->_get_arr_value("2", $offset);
  }
  function add_proclist()
  {
    return $this->_add_arr_value("2");
  }
  function set_proclist($index, $value)
  {
    $this->_set_arr_value("2", $index, $value);
  }
  function remove_last_proclist()
  {
    $this->_remove_last_arr_value("2");
  }
  function proclist_size()
  {
    return $this->_get_arr_size("2");
  }
  function setkey()
  {
    return $this->_get_value("3");
  }
  function set_setkey($value)
  {
    return $this->_set_value("3", $value);
  }
  function workernum()
  {
    return $this->_get_value("4");
  }
  function set_workernum($value)
  {
    return $this->_set_value("4", $value);
  }
}
class SetQuotaReq extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBString";
    $this->values["1"] = "";
    $this->fields["2"] = "ReportItem";
    $this->values["2"] = array();
    $this->fields["3"] = "PBInt";
    $this->values["3"] = "";
    $this->fields["4"] = "ProcReport";
    $this->values["4"] = array();
    $this->fields["5"] = "PBInt";
    $this->values["5"] = "";
  }
  function setkey()
  {
    return $this->_get_value("1");
  }
  function set_setkey($value)
  {
    return $this->_set_value("1", $value);
  }
  function itemlist($offset)
  {
    return $this->_get_arr_value("2", $offset);
  }
  function add_itemlist()
  {
    return $this->_add_arr_value("2");
  }
  function set_itemlist($index, $value)
  {
    $this->_set_arr_value("2", $index, $value);
  }
  function remove_last_itemlist()
  {
    $this->_remove_last_arr_value("2");
  }
  function itemlist_size()
  {
    return $this->_get_arr_size("2");
  }
  function id()
  {
    return $this->_get_value("3");
  }
  function set_id($value)
  {
    return $this->_set_value("3", $value);
  }
  function procitem($offset)
  {
    return $this->_get_arr_value("4", $offset);
  }
  function add_procitem()
  {
    return $this->_add_arr_value("4");
  }
  function set_procitem($index, $value)
  {
    $this->_set_arr_value("4", $index, $value);
  }
  function remove_last_procitem()
  {
    $this->_remove_last_arr_value("4");
  }
  function procitem_size()
  {
    return $this->_get_arr_size("4");
  }
  function workernum()
  {
    return $this->_get_value("5");
  }
  function set_workernum($value)
  {
    return $this->_set_value("5", $value);
  }
}
class ProcQ extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBString";
    $this->values["1"] = "";
    $this->fields["2"] = "PBInt";
    $this->values["2"] = "";
  }
  function procid()
  {
    return $this->_get_value("1");
  }
  function set_procid($value)
  {
    return $this->_set_value("1", $value);
  }
  function procquota()
  {
    return $this->_get_value("2");
  }
  function set_procquota($value)
  {
    return $this->_set_value("2", $value);
  }
}
class SetQuotaRsp extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBString";
    $this->values["1"] = "";
    $this->fields["2"] = "PBInt";
    $this->values["2"] = "";
    $this->fields["3"] = "PBInt";
    $this->values["3"] = "";
    $this->fields["4"] = "ProcQ";
    $this->values["4"] = array();
  }
  function setkey()
  {
    return $this->_get_value("1");
  }
  function set_setkey($value)
  {
    return $this->_set_value("1", $value);
  }
  function quota()
  {
    return $this->_get_value("2");
  }
  function set_quota($value)
  {
    return $this->_set_value("2", $value);
  }
  function setsum()
  {
    return $this->_get_value("3");
  }
  function set_setsum($value)
  {
    return $this->_set_value("3", $value);
  }
  function procq($offset)
  {
    return $this->_get_arr_value("4", $offset);
  }
  function add_procq()
  {
    return $this->_add_arr_value("4");
  }
  function set_procq($index, $value)
  {
    $this->_set_arr_value("4", $index, $value);
  }
  function remove_last_procq()
  {
    $this->_remove_last_arr_value("4");
  }
  function procq_size()
  {
    return $this->_get_arr_size("4");
  }
}
?>
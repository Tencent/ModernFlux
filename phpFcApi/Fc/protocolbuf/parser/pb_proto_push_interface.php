<?php
class CMD_TYPE extends PBEnum
{
  const SVR_PUSH_ONLINE_MSG  = 1;
  const KICKOUT  = 2;
  const PEER_PUSH_ONLINE_MSG  = 3;
  const CLIENT_LOGIN  = 1001;
  const CLIENT_HEARTBEAT  = 1002;
  const PUSH_ONLINE_MSG  = 1003;
  const CLIENT_KICKOUT  = 1004;
  const CLIENT_GET_ACT_LIST  = 2004;
  const CLIENT_ATTEND_ACT  = 2005;
  const CLIENT_GET_ACT_LIST_FAST  = 2006;
  const CLIENT_ATTEND_ACT_ALL  = 2007;
  const IDIP_BATCH_REQ  = 2008;
  const AMS_ACT_CONDITION_JUDGE  = 2009;
  const GPM_PRIME_GOODS_LIST  = 2011;
  const GPM_PAY  = 2013;
  const GPM_CALLBACK  = 2014;
  const TLOGDATA_PANGGUAN_PLAYER_LOGIN  = 3004;
  const TLOGDATA_PANGGUAN_PLAYER_EXPFLOW  = 3005;
  const TLOGDATA_PANGGUAN_PLAYER_REGISTER  = 3006;
  const TLOGDATA_ALLTANK_PLAYER_LOGIN  = 3007;
  const TLOGDATA_ALLTANK_PLAYER_EXPFLOW  = 3008;
  const TLOGDATA_ALLTANK_PLAYER_REGISTER  = 3009;
  const TLOGDATA_COMMON  = 3010;
  const ACTIVITY_MODIFIED_INFORM  = 4000;
  const SDK_LOG_REPORT  = 5000;
  const SDK_STATIC_REPORT  = 5001;
  const QUERY_LUCKYSTAR  = 6001;
  const PAY_SUCCEED  = 6002;
  const READ_TCAPLUS  = 100000;
  const WRITE_TCAPLUS  = 100001;
}
class BODY_TYPE extends PBEnum
{
  const BODY_REQUEST  = 1;
  const BODY_RESPONSE  = 2;
  const BODY_REQUEST_SIM  = 3;
  const BODY_RESPONSE_SIM  = 4;
}
class CmdBase extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
    function seq_id()
  {
    return $this->_get_value("1");
  }
    public function __construct($reader=null)
    {
        parent::__construct($reader);
        $this->fields["1"] = "PBInt";
        $this->values["1"] = "";
        $this->fields["2"] = "PBInt";
        $this->values["2"] = "";
        $this->fields["3"] = "PBInt";
        $this->values["3"] = "";
        $this->fields["4"] = "PBString";
        $this->values["4"] = "";
        $this->fields["5"] = "PBInt";
        $this->values["5"] = "";
        $this->fields["6"] = "PBInt";
        $this->values["6"] = "";
        $this->fields["8"] = "PBString";
        $this->values["8"] = "";
        $this->fields["7"] = "PBString";
        $this->values["7"] = "";
    }
    function set_seq_id($value)
  {
    return $this->_set_value("1", $value);
  }
  function cmd_id()
  {
    return $this->_get_value("2");
  }
  function set_cmd_id($value)
  {
    return $this->_set_value("2", $value);
  }
  function type()
  {
    return $this->_get_value("3");
  }
  function set_type($value)
  {
    return $this->_set_value("3", $value);
  }
  function from_ip()
  {
    return $this->_get_value("4");
  }
  function set_from_ip($value)
  {
    return $this->_set_value("4", $value);
  }
  function process_id()
  {
    return $this->_get_value("5");
  }
  function set_process_id($value)
  {
    return $this->_set_value("5", $value);
  }
  function mod_id()
  {
    return $this->_get_value("6");
  }
  function set_mod_id($value)
  {
    return $this->_set_value("6", $value);
  }
  function version()
  {
    return $this->_get_value("8");
  }
  function set_version($value)
  {
    return $this->_set_value("8", $value);
  }
  function body()
  {
    return $this->_get_value("7");
  }
  function set_body($value)
  {
    return $this->_set_value("7", $value);
  }
}
class CmdBaseResp extends PBMessage
{
  var $wired_type = PBMessage::WIRED_LENGTH_DELIMITED;
  public function __construct($reader=null)
  {
    parent::__construct($reader);
    $this->fields["1"] = "PBInt";
    $this->values["1"] = "";
    $this->fields["2"] = "PBString";
    $this->values["2"] = "";
    $this->fields["3"] = "PBString";
    $this->values["3"] = "";
    $this->fields["4"] = "PBString";
    $this->values["4"] = "";
  }
  function ret()
  {
    return $this->_get_value("1");
  }
  function set_ret($value)
  {
    return $this->_set_value("1", $value);
  }
  function err_msg()
  {
    return $this->_get_value("2");
  }
  function set_err_msg($value)
  {
    return $this->_set_value("2", $value);
  }
  function resp()
  {
    return $this->_get_value("3");
  }
  function set_resp($value)
  {
    return $this->_set_value("3", $value);
  }
  function md5val()
  {
    return $this->_get_value("4");
  }
  function set_md5val($value)
  {
    return $this->_set_value("4", $value);
  }
}
?>
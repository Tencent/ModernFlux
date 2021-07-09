<?php
require_once('./pb_parser.php');
$parser = new PBParser();
$parser->parse('./ModuleReport.proto');
echo 'parse successfully!';
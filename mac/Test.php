<?php

  dl("elster_php.dylib"); # muss im richtigen Verzeichnis sein /usr/lib/...
  include("elster_php.php");
  
  
  $v = "000a";
  echo "name: " .elster::getname($v) . "  type: " . elster::gettype($v) . "\n";

  #$v = new elster();

  #elster::initcan();

?>

<?php
  
  if (!include('konfiguration.php'))
  {
    die();
  }

  $pass = PASS;
  $pass_value = PASS_VALUE;
  
  if (!array_key_exists($pass, $_GET) ||
      htmlspecialchars($_GET[$pass]) != $pass_value ||
      !array_key_exists('table', $_GET))
  {
    die('sorry');
  }

  $db_link = mysqli_connect (
                     MYSQL_HOST, 
                     MYSQL_BENUTZER, 
                     MYSQL_KENNWORT, 
                     MYSQL_DATENBANK
                    );
  if (!$db_link)
  {
    die();
  }

  $ausgabe = '';
  $db_table = $_GET['table'];
  
  $where = ' where 1=1';
  foreach ($_GET as $gkey => $value)
  {
    if ($gkey == $pass ||
        $gkey == 'table')
    {
    } else {
      $where .= ' and ';
      
      if ($gkey == 'to' || $gkey == 'from')
      {
        $val = $value;
        if (strstr($val, "."))
        {
          if (!strpos($val, ':'))
            $val .=  " 00:00:00";
          //$ausgabe .= $val . " - ";
          $dtime = DateTime::createFromFormat("d.m.Y H:i:s", $val);
          $stamp = $dtime->getTimestamp();
          //$ausgabe .= $dtime->Format("d.m.Y H:i:s") . " ";
          $val = 1000*$stamp;
        }
        if ($gkey == 'to')
          $where .= "timestamp < '" . $val . "'";
        else
          $where .= "'" . $val . "' <= timestamp";
      } else
        $where .= $gkey . '=' . $value;
    }
  }
  
  $select = "SELECT * FROM " . $_GET['table'] . $where;
  $query = mysqli_query($db_link, $select);
  if (!$query)
  {
    die("wrong select " . "'" . $select . "'" . mysql_error());
  }
  
  $ausgabe .= "#" . $select . "\n";
  
  $token = ';';
  
  $first = true;
  while ($daten = mysqli_fetch_assoc($query))
  {
    if ($first)
    {
      $names = '';
      $next_token = '';
      foreach ($daten as $gkey => $value)
      {
        $names .= $next_token . $gkey;
        $next_token = $token;
      }
      $first = false;
      $ausgabe .= $names ."\n";
    }
    if (is_array($daten))
    {
      $next_token = '';
      foreach ($daten as $key => $value)
      {
        $ausgabe .= $next_token;
        $next_token = $token;
        if ($key == 'timestamp')
        {
          $str = sprintf("%d", $value % 1000);
          while (strlen($str) < 3)
            $str = '0' . $str;
          $ausgabe .= date("d.m.Y H:i:s" , $value / 1000) . "." . $str;         
        } else {
          $use_apos = strpos($daten[$key], $token);
          if ($use_apos)
            $ausgabe .= '"';
          $ausgabe .=  $daten[$key];
          if ($use_apos)
            $ausgabe .= '"';
        }
      }
    }
    $ausgabe .= "\n";
  }

  if ($first)
  {
    $ausgabe .= "#Die Tabelle: '" . $db_table . "' ist leer!\n";
  }
 
  if (!headers_sent())
  {
    header("Content-type: text/csv");
    header('Expires: ' . gmdate('D, d M Y H:i:s') . ' GMT');
    header('Content-Disposition: attachment; filename=vz_' . $db_table . '.csv');
    header('Pragma: no-cache');
  }
  echo $ausgabe;
?>
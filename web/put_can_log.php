<?php
  
  function my_readline($putdata)
  {
    $data = '';
    $d = '';
    while (($d = fread($putdata, 1)) != '')
    {
      if ($d >= ' ')
        $data .= $d;
      if ($d == ';')
        break;
    }
    return $data;
  }
  
  if (!include('konfiguration.php'))
  {
    die();
  }

  $putdata = fopen("php://input", "r");
  
  $pass = PASS . '=' . PASS_VALUE;
  $data = my_readline($putdata);
  if (strpos($data, $pass))
  {
    fclose($putdata);
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

  $Ok = 0;
  $NotOk = 0;
  $sql = my_readline($putdata);
  while ($sql != '') 
  {
    if (mysqli_query($db_link, $sql))
      $Ok++;
    else
      $NotOk++;
    
    $sql = my_readline($putdata);
  }
         
  fclose($putdata);
         
  echo 'Ok: ' . $Ok . '   not Ok: ' . $NotOk;
?>

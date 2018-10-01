<?php
  
  if (!include('konfiguration.php'))
  {
    die();
  }
  
  $pass = PASS;
  $pass_value = PASS_VALUE;
  
  if (!array_key_exists($pass, $_GET) ||
      htmlspecialchars($_GET[$pass]) != $pass_value ||
      !array_key_exists('id', $_GET) ||
      !array_key_exists('timestamp', $_GET) ||
      !array_key_exists('value', $_GET))
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
  
  if (array_key_exists('select', $_GET))
  {
    $result = mysqli_query($db_link, "SELECT * FROM data");
  
    echo 'select: channel_id, timestamp, value<br>';
    while($row = mysqli_fetch_array($result))
    {
      $str = sprintf("%d", $row['timestamp'] % 1000);
      while (strlen($str) < 3)
        $str = '0' . $str;
      echo $row['channel_id'] . ", " . date("d.m.Y H:i:s" , $row['timestamp'] / 1000) . "." . $str . ", " . $row['value'];
      echo "<br>";
    }
  }
  
  $sql = "INSERT data(channel_id, timestamp, value) " .
         "Value(" . $_GET['id'] . "," . $_GET['timestamp'] . "," . $_GET['value'] . ")";
  
  if (mysqli_query($db_link, $sql))
  {
    echo 'done<br>';
  } else {
    echo 'not done<br>';
  }
 
?>


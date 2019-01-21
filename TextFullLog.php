<html>

<head>
<title></title>
</head>



<body>

<?php 

$strDate = date("m-d-Y");
$deviceName = $_GET["devicename"];

$logFileName = $deviceName . "-" . $strDate . ".txt";

$data = $_GET["logdata"];

if (file_exists($logFileName)) {
  $fh = fopen($logFileName, 'a');
  fwrite($fh, $data);
} else {
  $fh = fopen($logFileName, 'w');
  fwrite($fh, $deviceName . "\n");
  fwrite($fh, $strDate . "\n");  
  fwrite($fh, $data);
}
fclose($fh);

?>

</body>
</html>
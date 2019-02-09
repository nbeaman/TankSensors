<html>

<head>
<title></title>
</head>



<body>

<?php 

$Action 	= $_GET["Action"];
$FromMAC 	= $_GET["FromMAC"];


$logFileName = $FromMAC . "-GROUP.txt";

if ($Action == "add"){
	$SensorIP 	= $_GET["SensorIP"];
	if (file_exists($logFileName)) {
	  $fh = fopen($logFileName, 'a');
	  fwrite($fh, $SensorIP . ",");
	} else {
	  $fh = fopen($logFileName, 'w');
	  fwrite($fh, $SensorIP . ",");
	}
	fclose($fh);
}

if ($Action == "remove"){
	$SensorIP 	= $_GET["SensorIP"];
	if (file_exists($logFileName)) {
		$GroupIPlist = file_get_contents($logFileName);
		echo $GroupIPlist;
	}
	
	$fh = fopen($logFileName, 'w');
	$AlteredIPlist = str_replace($SensorIP . ",","",$GroupIPlist);
	fwrite($fh, $AlteredIPlist);
	fclose($fh);
}

if ($Action == "load"){
	if (file_exists($logFileName)) {
		$GroupIPlist = file_get_contents($logFileName);
		echo $GroupIPlist;
	}
}

?>

</body>
</html>
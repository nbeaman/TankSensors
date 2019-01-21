<html>

<head>
<title></title>
</head>



<body>

Welcome <?php echo $_GET["logdata"]; ?><br>
Your email address is: <?php echo $_GET["logdata"]; ?>

<?php 
$myfile = fopen("newfile.txt", "w") or die("Unable to open file!");
$txt = $_GET["logdata"];
fwrite($myfile, $txt);
fclose($myfile);
?>

</body>
</html>
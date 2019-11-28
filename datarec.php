<?php
$nodeid = $_REQUEST["nodeid"];
$temperature = $_REQUEST["temperature"];
$humidity = $_REQUEST["humidity"];
$myfile = fopen($nodeid.".node", "a") or die("Unable to open file!");

$date = new DateTime();
$timestamp = $date->getTimestamp();
$datestr = date('Y-m-d\TH:i:s', $timestamp);

$txt = $datestr.",".$temperature.",".$humidity."\n";
fwrite($myfile, $txt);
fclose($myfile);

# echo $_SERVER["REQUEST_METHOD"];
# print_r( $_GET );
# print_r( $_POST );
# print_r( $_REQUEST );
# Display the web page
# echo 'Hello ' . htmlspecialchars($_REQUEST["name"]) . '!';

?>

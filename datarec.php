<?php
$nodeid = $_REQUEST["nodeid"];
$temperature = $_REQUEST["temperature"];
$humidity = $_REQUEST["humidity"];
$dts = (int)$_REQUEST["dts"];
$myfile = fopen($nodeid.".node", "a") or die("Unable to open file!");
$tsfile = fopen($nodeid.".tim", "r+") or fopen($nodeid.".tim", "w+") or die("Unable to open file!");
$dbgfile = fopen("debug.txt", "a") or die("Unable to open file!");

$date = new DateTime();
$timestamp = $date->getTimestamp();

$npts = count($temperature);

echo filesize($tsfile);

//fseek($tsfile, 0, SEEK_SET);
$ltimestamp = fread($tsfile, 10) or print_r("read failed");

if ($ltimestamp != false) {
    $dts = (int)(($timestamp - $ltimestamp) / $npts);
}

fseek($tsfile, 0, SEEK_SET);
fwrite($tsfile, $timestamp);


$dbgtxt = date('Y-m-d\TH:i:s', $timestamp).", ".$nodeid.", ".$ltimestamp.", ".$timestamp.", ".$npts.", ".$dts.", ".$_SERVER['REMOTE_ADDR']."\n";
fwrite($dbgfile, $dbgtxt);

print_r(date('Y-m-d\TH:i:s', $timestamp).", ".$nodeid.", ".$ltimestamp.", ".$timestamp);

$timestamp = $timestamp - $dts*($npts-1);

for ($i = 0; $i < $npts; $i++) {
    $datestr = date('Y-m-d\TH:i:s', $timestamp + $dts*$i);
    $txt = $datestr.",".$temperature[$i].",".$humidity[$i]."\n";
    fwrite($myfile, $txt);
}

fclose($dbgfile);
fclose($myfile);
fclose($tsfile);

# echo $_SERVER["REQUEST_METHOD"];
# print_r( $_GET );
# print_r( $_POST );
# print_r( $_REQUEST );
# Display the web page
# echo 'Hello ' . htmlspecialchars($_REQUEST["name"]) . '!';

?>

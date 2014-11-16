<html
<head>
<title>TCP Window Calculator</title>
</head>
<body>

<?php

if ($_REQUEST['rtt']) {

    $rtt = $_REQUEST['rtt'];
    $bw = $_REQUEST['bw'];

    // win = RTT (ms) * BW (kpbs)
    // (the m and the k cancel each other out)
    $win = $rtt * $bw;

    // convert to bytes
    $win /= 8;
?>

<table border=1>

<tr>
<td>RTT:</td>
<td><?=$_REQUEST['rtt']?>ms</td>
</tr>

<tr>
<td>Bandwidth:</td>
<td><?=$_REQUEST['bw']?>kbps</td>
</tr>

<tr>
<td>Optimal TCP window size:</td>
<td><?=$win?> bytes</td>
</tr>

</table>

<hr>

<?php
} // if ($_REQUEST['rtt'])
?>


<form method="GET" action="tcpwin.php">

<table>

<tr>
<td>RTT:</td>
<td><input type="text" name="rtt" size=4 value="<?= $rtt ?>">ms</td>
</tr>
<tr>
<td>Bandwidth:</td>
<td><input type="text" name="bw" size=8 value="<?= $bw ?>">kbps</td>
</tr>
</table>

<input type="submit">

</form>

<hr>
<a href="tcpwin.phps">View source</a>

</body>
</html>

<?php
$title = "Subnetting Quiz";
$onLoad = "focus()";
$js = "
var FocusNeeded = true;

function focus() {
    // Set the focus to the network field
    if (FocusNeeded) {
        document.myform.network.focus();
    }
}
";
include("../../../header.php");
?>

<div id="sheet">

<h3>Subnetting Quiz</h3>

<b>Update 2014/09/12!</b> I've written a new, all-Javascript version of this quiz, 
please <a href="/subnetting-is-fun/">find it here.</a> Once I move this site to Amazon S3
this version of the quiz will be removed.

<br>
<br>

Please <a href="/articles/79.html">read my article</a> for some background
on this web-based quiz.  You may be also be interested in <a
href="subnet.phps">reading the source</a>.  If you
enjoy this quiz, please <a href="/articles/79.html#submitcomment">leave me a comment</a>.
Have fun!  

<hr />

<?php
#
# Since PHP doesn't support unsigned integers, doing IP math
# on a 32 bit platform is a problem.  This bites us in several 
# operations, and in the cases of generating a random netmask and
# IP I simply fall back to generating a dotted decimal value
# one octet at a time.  The alternatives (e.g. doing bit math 
# on two 16 bit values) don't really seem any more elegant.
#
# $Id: subnet.php,v 1.2 2004/07/03 06:53:49 kehlet Exp $
# Copyright 2002-2004 (c) Steven Kehlet.  All Rights Reserved.
#

class IP
{
    var $ulong;

    function IP($ulong) {
        $this->ulong = $ulong;
    }

    function getOctets() {
        $array = unpack("C4oct", $this->ulong);
        return array($array['oct1'], $array['oct2'], $array['oct3'], $array['oct4']);
    }

    function getNetwork($netmask) {
        return new IP($this->ulong & $netmask->ulong);
    }

    function getBroadcast($netmask) {
        $network = $this->getNetwork($netmask);
        return new IP($network->ulong | ~$netmask->ulong);
    }

    function toDottedDecimalString() {
        list($oct1, $oct2, $oct3, $oct4) = $this->getOctets();
        return "${oct1}.${oct2}.${oct3}.${oct4}";
    }

    function toHexString() {
        return "0x" . bin2hex($this->ulong);
    }

    function toBinaryString() {
        // decbin can't seem to handle $this->ulong, so break it up.
        list($oct1, $oct2, $oct3, $oct4) = $this->getOctets();
        $str = sprintf("%08d", decbin($oct1));
        $str .= " " . sprintf("%08d", decbin($oct2));
        $str .= " " . sprintf("%08d", decbin($oct3));
        $str .= " " . sprintf("%08d", decbin($oct4));
        return $str;
    }

    function toPrefixString() {
        // Count the bits individually, since log() doesn't 
        // support a base arg until PHP 4.3.0 (e.g. 
        // $n = log($this->ulong, 2).  Plus it probably
        // couldn't handle $this->ulong anyway.
        $array = unpack("Nulong", $this->ulong);
        $ulong = $array['ulong'];
        $n = 0;
        for ($i = 0; $i < 32; $i++) {
            if ($ulong & (1 << $i)) {
                $n++;
            }
        }
        return "/" . $n;
    }
}

// PHP doesn't support method argument overloading,
// so subclass to handle dotted decimal strings.
class DottedDecimalIP extends IP
{
    function DottedDecimalIP($str) {
        list($oct1, $oct2, $oct3, $oct4) = split('\.', $str);
        $ulong = pack("CCCC", $oct1, $oct2, $oct3, $oct4);
        $this->IP($ulong);
    }
}


#
# form submission
#
if (isset($_REQUEST['network'])) {
    $ip = trim($_REQUEST['ip']);
    $netmask = trim($_REQUEST['netmask']);
    $networkAnswer = trim($_REQUEST['network']);
    $broadcastAnswer = trim($_REQUEST['broadcast']);
    $startTime = trim($_REQUEST['startTime']);

    print "<table border=1>";

    print "<tr>\n";
    $ip = new DottedDecimalIP($ip);
    print "<td><b>IP:</b></td>\n";
    print "<td>" . $ip->toDottedDecimalString() . "</td>\n";
    print "<td>" . $ip->toBinaryString() . "</td>\n";
    print "</tr>\n";

    print "<tr>\n";
    $netmask = new DottedDecimalIP($netmask);
    print "<td><b>Subnet mask:</b></td>\n";
    print "<td>";
    print $netmask->toDottedDecimalString();
    print "<br>" . $netmask->toHexString();
    print "<br>" . $netmask->toPrefixString();
    print "</td>";
    print "<td>" . $netmask->toBinaryString() . "</td>\n";
    print "</tr>\n";

    print "<tr>\n";
    $network = $ip->getNetwork($netmask);
    print "<td><b>Network address:</b></td>\n";
    print "<td>";
    print 'You said: ' . $networkAnswer;
    print "<br>\n";
    if ($networkAnswer != $network->toDottedDecimalString()) {
        print "<b><font color=\"red\">INCORRECT!<br>";
        print $network->toDottedDecimalString();
        print "</font></b>";
    } else {
        print "<b><font color=\"green\">CORRECT!</font></b>";
    }
    print "</td>";
    print "<td>" . $network->toBinaryString() . "</td>\n";
    print "</tr>\n";

    print "<tr>\n";
    $broadcast = $ip->getBroadcast($netmask);
    print "<td><b>Broadcast address:</b></td>\n";
    print "<td>";
    print 'You said: ' . $broadcastAnswer;
    print "<br>\n";
    if ($broadcastAnswer != $broadcast->toDottedDecimalString()) {
        print "<b><font color=\"red\">INCORRECT!<br>";
        print $broadcast->toDottedDecimalString();
        print "</font></b>";
    } else {
        print "<b><font color=\"green\">CORRECT!</font></b>";
    }
    print "</td>";
    print "<td>" . $broadcast->toBinaryString() . "</td>\n";
    print "</tr>\n";

    print "</table>";

    $timeTook = time() - $startTime;
    if ($timeTook > (60 * 60)) {
        $timeTook /= (60 * 60);
        $units = "hours";
    } elseif ($timeTook > 60) {
        $timeTook /= 60;
        $units = "minutes";
    } else {
        $units = "seconds";
    }
    $timeTook = round($timeTook, 1);

    print "You answered in: " . $timeTook . " " . $units . ".\n";

    print "<hr>\n";
} 

# My PHP install on Solaris is segmentation faulting on every
# call to pow()!
function safePow($base, $exp) {
    $result = 1;
    while ($exp-- > 0) {
        $result *= $base;
    }
    return $result;
}

#
# Generate a "sane" subnet mask:
# - not: 0.0.0.0, 128.0.0.0, 255.255.255.255, 255.255.255.254
#
function generateMask() {
    $ct = 10;  # infinite loop prevention
    while ($ct-- > 0) {
        $oct1 = $oct2 = $oct3 = $oct4 = 255;
        $which = rand(1,4);
        $special_oct = 256 - safePow(2, rand(0,8));
        if ($which == 1) {
            $oct1 = $special_oct;
            $oct2 = $oct3 = $oct4 = 0;
        } elseif ($which == 2) {
            $oct2 = $special_oct;
            $oct3 = $oct4 = 0;
        } elseif ($which == 3) {
            $oct3 = $special_oct;
            $oct4 = 0;
        } elseif ($which == 4) {
            $oct4 = $special_oct;
        }
        $netmaskString = "${oct1}.${oct2}.${oct3}.${oct4}";

        # sanity check
        if ($netmaskString != "0.0.0.0" &&
            $netmaskString != "128.0.0.0" &&
            $netmaskString != "255.255.255.255" &&
            $netmaskString != "255.255.255.254") {
            # good netmask
            break;
        }
    }
    return new DottedDecimalIP($netmaskString);
}

#
# Generate a "sane" IP:
# - Class A, B, or C (i.e. first octet < 224)
# - resulting network address is not 0.0.0.0
# - resulting broadcast address is not 255.255.255.255
# - IP address is not the same as the network or broadcast address
#
function generateIP($netmask) {
    $ct = 10;  # infinite loop prevention
    while ($ct-- > 0) {
        $oct1 = rand(0,255);
        $oct2 = rand(0,255);
        $oct3 = rand(0,255);
        $oct4 = rand(0,255);
        $ipString = join('.', array($oct1, $oct2, $oct3, $oct4));
        $ip = new DottedDecimalIP($ipString);

        # sanity check
        $network = $ip->getNetwork($netmask);
        $broadcast = $ip->getBroadcast($netmask);
        $networkString = $network->toDottedDecimalString();
        $broadcastString = $broadcast->toDottedDecimalString();
        if ($oct1 < 224 &&
            $networkString != "0.0.0.0" &&
            $broadcastString != "255.255.255.255" &&
            $ipString != $networkString &&
            $ipString != $broadcastString) {
            # good IP
            break;
        }
    }
    return $ip;
}

$netmask = generateMask();
$ip = generateIP($netmask);

print "<form name=\"myform\" method=\"GET\" action=\"subnet.php\">\n";
print "<table border=1>\n";

print "<tr>\n";
print "<td><b>IP:</b></td>\n";
print "<td>" . $ip->toDottedDecimalString() . "</td>\n";
print "</tr>\n";

# easy mode always uses dotted decimal netmasks
$which = 1;
if (isset($_REQUEST['advancedSubnets'])) {
    $which = rand(1,3);
}
if ($which == 1) {
    $showNetmask = $netmask->toDottedDecimalString();
} elseif ($which == 2) {
    $showNetmask = $netmask->toHexString();
} elseif ($which == 3) {
    $showNetmask = $netmask->toPrefixString();
}
print "<tr>\n";
print "<td><b>Subnet mask:</b></td>\n";
print "<td>$showNetmask</td>\n";
print "</tr>\n";

print "<tr>\n";
print "<td><b>Network address:</b></td>\n";
print "<td><input name=\"network\" type=\"text\" onclick=\"FocusNeeded=false;\" onkeypress=\"FocusNeeded=false;\" size=16></td>\n";
print "<tr>\n";
print "<td><b>Broadcast address:</b></td>\n";
print "<td><input name=\"broadcast\" type=\"text\" size=16></td>\n";
print "</tr>\n";
print "</table>\n";

print "<input type=\"submit\" value=\"Submit\">\n";
print "<input type=\"button\" value=\"Skip\" onClick='window.location.reload()'>\n";
print "<input name=\"advancedSubnets\" type=\"checkbox\"";
if (isset($_REQUEST['advancedSubnets'])) {
    print " checked";
}
print "> Advanced subnets\n";

print "<input name=\"ip\" type=\"hidden\" value=\"" .
    $ip->toDottedDecimalString() . "\">\n";
print "<input name=\"netmask\" type=\"hidden\" value=\"" .
    $netmask->toDottedDecimalString() . "\">\n";
print "<input name=\"startTime\" type=\"hidden\" value=\"" . time() . "\">\n";
print "</form>\n";
?>

<br />

</div>

<?php include("../../../footer.php"); ?>

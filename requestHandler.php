<?php

/*
Remote Graphic Controller PHP script for handling task requests
Created by Marc Frincu
Email: marc@ieat.ro
May 10th 2007
*/

// check to see al paramteres are set
if (isset($_REQUEST["data"]) && isset($_REQUEST["taskId"]))
{
	$taskId = $_REQUEST["taskId"];
	mkdir("dir" . $taskId, 0777);
	$filename = $_REQUEST["taskId"];

	$eol = "\r\n";	
	/*
	$data =  "xrotate\t" . $_REQUEST["xrotate"] . $eol;
	$data .= "yrotate\t" . $_REQUEST["yrotate"] . $eol;
	$data .= "zrotate\t" . $_REQUEST["zrotate"] . $eol;
	$data .= "xtranslate\t" . $_REQUEST["xtranslate"] . $eol;
	$data .= "ytranslate\t" . $_REQUEST["ytranslate"] . $eol;
	$data .= "ztranslate\t" . $_REQUEST["ztranslate"] . $eol;
	$data .= "zoom\t" . $_REQUEST["zoom"] . "\n";
	$data .= "quit\t" . $_REQUEST["quit"] . "\n";
*/
	$data = $_REQUEST["data"];

	//check to see if file with name equal to uniqueId exists
	if (file_exists($filename))
	{
		$handle = fopen($filename, 'w');
		fwrite($handle, $data);
		fclose($handle);
	}
	else
	{
			//$taskId = md5(uniqid(rand(), true));

			$handle = fopen($filename, 'w');
			fwrite($handle, $data);
			fclose($handle); 

			$command = "test.exe " . $taskId;
			exec($command, $status);
	}
	echo $taskId;
}

?>
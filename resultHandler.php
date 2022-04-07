<?php

/*
Remote Graphic Controller PHP script for handling task results
Created by Marc Frincu
Email: marc@ieat.ro
May 10th 2007
*/

$file = "dir" . $_REQUEST["taskId"] . "/test" . $_REQUEST["lastId"] . ".bmp";

while (!file_exists($file))
{
}
echo $file;
?>
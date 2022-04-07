/*
Remote Graphic Controller JS Script
Created by Marc Frincu
Email: marc@ieat.ro
May 10th 2007
*/

// initialize XMLHttpRequest object
var xmlobj = null;
var loaded = false;
var initialized = false;
var lastId = 0;
var lastIdBefore = 0;
var pics = new Array();
var step = 0;
var wantResult = false;
var uniqueId = -1;
var taskItems = 0;
var isBatch = false;
var batchData = "";
var t;
var t1;

//initializes XmlHTTPRequest object. For AJAX
function init(){
    // check for existing requests
    if(xmlobj != null && xmlobj.readyState != 0&&xmlobj.readyState != 4){
        xmlobj.abort();
    }
 	if (window.XMLHttpRequest) { // Mozilla, Safari, ...
	    xmlobj = new XMLHttpRequest();
	} else if (window.ActiveXObject) { // IE
	    //try to get the most modern implementation
	    var list = ["Microsoft.XmlHttp", "MSXML2.XmlHttp", "MSXML2.XmlHttp.3.0", "MSXML2.XmlHttp.4.0", "MSXML2.XmlHttp.5.0"];
	    var ok = false;
	    var i = 5;
	    while (i >= 0 && ok == false){
		try{
		    xmlobj = new ActiveXObject(list[i]);
		    ok = true;
		}
		catch (e){}
		i--;
	    }
	}
	if (!xmlobj){
	    alert('Giving up:( Cannot create an XMLHTTP instance');
	    initialized = false;
	}
	
	// assign state handler
	xmlobj.onreadystatechange = function(){
		stateChecker();
	}
}

//Sends to the server a task
function sendRequest(file, parameters, getResult){
	parameters = "data=" + parameters + "&taskId=" + uniqueId;
    if (!initialized){
	init();
    }
    wantResult = getResult;
    // open an asynchronous socket connection
    xmlobj.open('POST', file, true);
    // send request
    xmlobj.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xmlobj.setRequestHeader("Content-length", parameters.length);
    xmlobj.setRequestHeader("Connection", "close");
    xmlobj.send(parameters);
}


// check request status
function stateChecker(){
    // if request is completed
    if(xmlobj.readyState == 4){
		if (true === wantResult){
			if (isBatch == true)
			{
				if (lastId <= taskItems)
				{
					preloadImages();
					displayImage(lastId);
					lastId++;
					document.getElementById("submitBatch").disabled = false;	
					document.getElementById("singlecomm").disabled = false;
					var params = batchData.substring(0, batchData.indexOf("endItem"));
					sendBatchItem(params);
					batchData = batchData.substring(batchData.indexOf("endItem"));
				}
			}
			else
			{
				preloadImages();
				displayImage(lastId);
				lastId++;
				document.getElementById("submit").disabled = false;	
				document.getElementById("batchcomm").disabled = false;
			}
		}
		else{
			//all we receive is an ack for the task submission with a unique ID
		}
    }
	//if toClose = true then close window
}

//Sends a request for the result
function getResult(file, parameters){
    //The parameter is usually the lastId. because we only want to get the rest of the files, not all of them
    sendRequest(file, parameters, true);
    
}

function preloadImages(){
        pics[lastId] = new Image();
        pics[lastId].src = xmlobj.responseText;
}

// preload images
function preloadImages2(){
    // get image collection
    var xml = xmlobj.responseXML;
    if (!xml || !xml.documentElement){
		//IE doesn't always see the response correctly
		//this fixes the problem
		if (window.ActiveXObject){
			xml = new ActiveXObject("MSXML2.DOMDocument");
			xml.loadXML(xmlobj.responseText);
		}
    }
    var imgcol = xml.getElementsByTagName('image');
	alert(imgcol.length);
    for(var i=0; i<imgcol.length; i++){
		
		// preload images
		//lastId refers to the last image loaded at the previous result request
        pics[lastId] = new Image();
        pics[lastId].src = imgcol[i].firstChild.nodeValue;
    }
    lastId++;
    //check if final image. if so
    //clearTimeout(t1);
    //else
    //t1 = setTimeout("getResult('resultHandler.php','lastId=" + lastId  + "')", 5000);
    
} 

//display the image with a given id
function displayImage(elemid){
    var cdiv = document.getElementById('container');
    if(!cdiv){
		createImageContainer();
    }

    var newpic = pics[elemid];
    var oldpic = document.getElementById('largepic');
    if(!oldpic){
		return;
    }
    oldpic.setAttribute('src', newpic.src);
}

//create a basic slide
function makeSlide(){
    //if browser does not support the image object, exit.
    if (!document.images){
		return;
    }
    displayImage(lastId);
    
	if (lastIdBefore < lastId){
		lastIdBefore++;
    }
    else{
		clearInterval(t);
    }
}

//creates the tag/container that will hold the image displayed on the screen
function createImageContainer(){
    // create image container
    var cdiv = document.createElement('div');
    cdiv.setAttribute('id', 'container');
    cdiv.style.border = "1px solid black";
	cdiv.style.margin = "5px";

	var img = document.createElement('img');
    img.setAttribute('width', '285');
    img.setAttribute('height', '220');
    img.setAttribute('id', 'largepic');
    cdiv.appendChild(img);
    document.getElementById('display').appendChild(cdiv);
}

//creates the parameter string that will be sent to the server script
function makeParameterString(quit){
	var eol = "\r\n";
    var s = "";
    s += "xrotate\t" + document.getElementById("xrotate").value + eol;
    s += "yrotate\t" + document.getElementById("yrotate").value + eol;
    s += "zrotate\t" + document.getElementById("zrotate").value + eol;
    s += "xtranslate\t" + document.getElementById("xtranslate").value + eol;
    s += "ytranslate\t" + document.getElementById("ytranslate").value + eol;
    s += "ztranslate\t" + document.getElementById("ztranslate").value + eol;
    s += "zoom\t" + document.getElementById("zoom").value + eol;
	s += "quit\t" + quit + eol;
    return s;    
}

//validates the input
function validate(){
    var s = "";
    if (document.getElementById("xrotate").value == ""){
		s += "Rotation X should not be empty\n";
    }
    else{
	if (!isNumeric(document.getElementById("xrotate").value)){
	    s += "Rotation X should be a number\n";
	}
    }
    if (document.getElementById('yrotate').value == ""){
	s += "Rotation Y should not be empty\n";
    }
    else{
	if (!isNumeric(document.getElementById('yrotate').value)){
	    s += "Rotation Y should be a number\n";
	}
    }

    if (document.getElementById('zrotate').value == ""){
		s += "Rotation Z should not be empty\n";
    }
    else{
	if (!isNumeric(document.getElementById('zrotate').value)){
	    s += "Rotation Z should be a number\n";
	}
    }

    if (document.getElementById('xtranslate').value == ""){
		s += "Translation X should not be empty\n";
    }
    else{
	if (!isNumeric(document.getElementById('xtranslate').value)){
	    s += "Translation X should be a number\n";
	}
    }

    if (document.getElementById('ytranslate').value == ""){
	s += "Translation Y should not be empty\n";
    }
    else{
	if (!isNumeric(document.getElementById('ytranslate').value)){
	    s += "Translation Y should be a number\n";
	}
    }

    if (document.getElementById('ztranslate').value == ""){
		s += "Translation Z should not be empty\n";
    }
    else{
	if (!isNumeric(document.getElementById('ztranslate').value)){
	    s += "Translation Z should be a number\n";
	}
    }

    if (document.getElementById('zoom').value == ""){
		s += "Zooming should not be empty\n";
    }    
    else{
	if (!isNumeric(document.getElementById('ztranslate').value)){
	    s += "Zooming should be a number\n";
	}
    }
    
    if (s != ""){
		alert(s);
		return false;
    }
    
	if (uniqueId == -1)
	{
		generateUniqueId();
	}
    //success. now we can send the request
    sendRequest("requestHandler.php", makeParameterString(0), false);
    
	document.getElementById("submit").disabled = true;		
	document.getElementById("batchcomm").disabled = true;

    //set the getResult to loop at a certain interval until we get all the images
    t1 = setTimeout("getResult('resultHandler.php','taskId=" + uniqueId  + "&lastId=" + lastId + "')", 500);
	isBatch = false;
    return true;
}

//tests if a string is a number
function isNumeric(sText){
    var validChars = "-0123456789.";
    var isNumber = true;
    var char;
    
    for (i = 0; i < sText.length && isNumber == true; i++){ 
		char = sText.charAt(i); 
		if (validChars.indexOf(char) == -1){
			isNumber = false;
		}
    }
    return isNumber;
}

//generates a unique task id based on the current time in milliseconds
function generateUniqueId()
{
	var tmp = new Date();
	uniqueId = tmp.getTime();
}

//sends a quit message to the coresponding opengl application just before unload event
window.onbeforeunload= function (evt) {
	sendRequest("requestHandler.php", makeParameterString(1), false);
}

window.onunload = function (evt) {
	document.getElementById("batchtext").value = "";
	sendRequest("requestHandler.php", makeParameterString(1), false);

}

//handles tab keys in textarea
function insertTab(event,obj) {
    var tabKeyCode = 9;
    if (event.which) // mozilla
        var keycode = event.which;
    else // ie
        var keycode = event.keyCode;
    if (keycode == tabKeyCode) {
        if (event.type == "keydown") {
            if (obj.setSelectionRange) {
                // mozilla
                var s = obj.selectionStart;
                var e = obj.selectionEnd;
                obj.value = obj.value.substring(0, s) + 
                    "\t" + obj.value.substr(e);
                obj.setSelectionRange(s + 1, s + 1);
                obj.focus();
            } else if (obj.createTextRange) {
                // ie
                document.selection.createRange().text="\t"
                obj.onblur = function() { this.focus(); this.onblur = null; };
            } else {
                // unsupported browsers
            }
        }
        if (event.returnValue) // ie ?
            event.returnValue = false;
        if (event.preventDefault) // dom
            event.preventDefault();
        return false; // should work in all browsers
    }
    return true;
}

function addBatch()
{
	var item = "xrotate	0\n" +
	"yrotate	0\n" +
	"zrotate	0\n" +
	"xtranslate	0\n" +
	"ytranslate	0\n" +
	"ztranslate	0\n" +
	"ztranslate	0\n" +
	"zoom	1\n";
	"enditem\n";

	document.getElementById("batchtext").value = item;
	
}

//validates the batch jobs to be sent to the 
function validateBatch()
{
	var s = "";
	var text = document.getElementById("batchtext").value;
	if (text == "")
	{
		s = "There should be at least one batch item in the batch job list\n";
	}
	if (s != "")
	{
		alert(s);
		return false;
	}

	taskItems = lastId;
	batchData = text;
	while ((pos = text.indexOf("enditem")) != -1)
	{
		taskItems++;
		text = text.substring(pos+1);
	}

	if (uniqueId == -1)
	{
		generateUniqueId();
	}
 
	if (taskItems > 0)
	{
		var params = batchData.substring(0, batchData.indexOf("enditem"));
		sendBatchItem(params);
		batchData = batchData.substring(batchData.indexOf("enditem"));
	}
	else
	{
		s = "Batch Item incorectly terminated. use: enditem tag\n";
	}

	if (s != "")
	{
		alert(s);
		return false;
	}
	isBatch = true;
	return true;
}

function sendBatchItem(parameters)
{
	alert(parameters + " " + lastId);
    //success. now we can send the request
    sendRequest("requestHandler.php", parameters, false);
    
	document.getElementById("submitBatch").disabled = true;		
	document.getElementById("singlecomm").disabled = true;

    //set the getResult to loop at a certain interval until we get all the images
    t = setTimeout("getResult('resultHandler.php','taskId=" + uniqueId  + "&lastId=" + lastId + "')", 500);
 
}

#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>
#include <ESPmDNS.h>
#include <typeinfo>

extern WebServer* server;  // used to check if the webserver is running

// function defaults
String humanReadableSize(uint64_t bytes);
String listFiles(FS fs, bool ishtml, String folder, bool isLittleFS);
String processor(const String& var);
String readLineFromFile(File myFile);

void loopOptionsWebUi();

void configureWebServer();
void startWebUi(bool mode_ap = false);
const char index_css[] PROGMEM = R"rawliteral(
.gg-rename {
    box-sizing: border-box;
    position: relative;
    display: inline-block;
    width: 20px;
    height: 16px;
    transform: scale(var(--ggs,1));
    background:
    linear-gradient(
    to left,currentColor 22px,
    transparent 0)
    no-repeat 6px center/2px 22px
  }

  .gg-rename::after,
    .gg-rename::before {
    content: "";
    display: block;
    box-sizing: border-box;
    position: absolute;
    width: 6px;
    height: 12px;
    border: 2px solid;
    top: 2px
  }

  .gg-rename::before {
    border-right: 0;
    border-top-left-radius: 3px;
    border-bottom-left-radius: 3px
  }

  .gg-rename::after {
    width: 10px;
    border-left: 0;
    border-top-right-radius: 3px;
    border-bottom-right-radius: 3px;
    right: 0
  }
  .gg-folder {
    cursor: pointer;
    transform: scale(var(--ggs,1))
  }
  .gg-folder,
  .gg-folder::after {
      box-sizing: border-box;
      position: relative;
      display: inline-block;
      width: 22px;
      height: 16px;
      border: 2px solid;
      border-radius: 3px
  }
  .gg-folder::after {
      content: "";
      position: absolute;
      width: 10px;
      height: 4px;
      border-bottom: 0;
      border-top-left-radius: 2px;
      border-top-right-radius: 4px;
      border-bottom-left-radius: 0;
      border-bottom-right-radius: 0;
      top: -5px
  }
  .gg-trash {
    box-sizing: border-box;
    position: relative;
    display: inline-block;
    transform: scale(var(--ggs,1));
    width: 10px;
    height: 12px;
    border: 2px solid transparent;
    box-shadow:
        0 0 0 2px,
        inset -2px 0 0,
        inset 2px 0 0;
    border-bottom-left-radius: 1px;
    border-bottom-right-radius: 1px;
    margin-top: 4px;
    margin-bottom: 2px;
  cursor: pointer;
  }
  .gg-trash::after,
  .gg-trash::before {
      content: "";
      display: block;
      box-sizing: border-box;
      position: absolute
  }
  .gg-trash::after {
      background: currentColor;
      border-radius: 3px;
      width: 16px;
      height: 2px;
      top: -4px;
      left: -5px
  }
  .gg-trash::before {
      width: 10px;
      height: 4px;
      border: 2px solid;
      border-bottom: transparent;
      border-top-left-radius: 2px;
      border-top-right-radius: 2px;
      top: -7px;
      left: -2px
  }
  .gg-data {
    transform: scale(var(--ggs,1))
  }
  .gg-data,
  .gg-data::after,
  .gg-data::before {
    box-sizing: border-box;
    position: relative;
    display: inline-block;
    border: 2px solid;
    border-radius: 50%;
    width: 14px;
    height: 14px
  }
  .gg-data::after,
  .gg-data::before {
    content: "";
    position: absolute;
    width: 6px;
    height: 6px;
    top: 2px;
    left: 2px
  }
  .gg-data::after {
    background: linear-gradient( to left,
        currentColor 8px,transparent 0)
        no-repeat bottom center/2px 8px;
    width: 22px;
    height: 22px;
    top: -6px;
    left: -6px
  }
  .gg-data,
  .gg-data::after {
    border-top-color: transparent;
    border-bottom-color: transparent
  }
  .gg-arrow-down-r {
      box-sizing: border-box;
      position: relative;
      display: inline-block;
      width: 22px;
      height: 22px;
      border: 2px solid;
      transform: scale(var(--ggs,1));
      cursor: pointer;
      border-radius: 4px
  }
  .gg-arrow-down-r::after,
  .gg-arrow-down-r::before {
      content: "";
      display: block;
      box-sizing: border-box;
      position: absolute;
      bottom: 4px
  }
  .gg-arrow-down-r::after {
      width: 6px;
      height: 6px;
      border-bottom: 2px solid;
      border-left: 2px solid;
      transform: rotate(-45deg);
      left: 6px
  }
  .gg-arrow-down-r::before {
      width: 2px;
      height: 10px;
      left: 8px;
      background: currentColor
  }

  .gg-pen {
    box-sizing: border-box;
    position: relative;
    display: inline-block;
    transform: rotate(-45deg) scale(var(--ggs, 1));
    width: 14px;
    height: 4px;
    border-right: 2px solid transparent;
    box-shadow:
      0 0 0 2px,
      inset -2px 0 0;
    border-top-right-radius: 1px;
    border-bottom-right-radius: 1px;
    margin-top: 6px;
    margin-bottom: 6px;
    cursor: pointer;
  }


  .gg-pen::after,
  .gg-pen::before {
    content: "";
    display: block;
    box-sizing: border-box;
    position: absolute;
  }

  .gg-pen::before {
    background: currentColor;
    border-left: 0;
    right: -6px;
    width: 3px;
    height: 4px;
    border-radius: 1px;
    top: 0;
  }

  .gg-pen::after {
    width: 8px;
    height: 7px;
    border-top: 4px solid transparent;
    border-bottom: 4px solid transparent;
    border-right: 7px solid;
    left: -11px;
    top: -2px;
  }

  
  body {
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
    margin: 0;
    padding: 5px;
    color: #ff3ec8;
    background-color: #202124;
  }

  .container {
    max-width: 800px;
    margin: 5px auto;
    padding: 0 5px;
  }

  h3 {
    margin: 0;
    padding: 10px 0;
    border-bottom: 1px solid #7b007b;
  }

  table {
    width: 100%;
    border-collapse: collapse;
    border-bottom: 1px solid #7b007b;
  }

  th, td {
    padding: 5px;
    border-bottom: 1px solid #7b007b;
  }

  th {
    text-align: left;
  }

  a {
    color: #ffbee0;
    text-decoration: none;
  }

  a:hover {
    text-decoration: underline;
  }

  button {
    background-color: #303134;
    color: #ff3ec8;
    border: 2px solid;
    padding: 4px 8px;
    border-radius: 4px;
    border-color: #ef007b;
    cursor: pointer;
    margin: 5px;
  }

  button:hover {
    background-color: #ffabd7;
  }

  #detailsheader, #updetailsheader {
    display: flex;
    justify-content: space-between;
  }

  @media (max-width: 768px) {
    body {
      font-size: 14px;
    }

    table {
      font-size: 12px;
    }

    th, td {
      padding: 5px;
    }

    button {
      font-size: 12px;
      padding: 6px 12px;
    }
  }
  th:first-child, td:first-child {
    width: 60%;
  }
  th:last-child, td:last-child {
    width: 150px;
    text-align: center;
  }
.float-element {
  position: absolute;
  top: 10px; /* Ajuste conforme necessário */
  right: 10px; /* Ajuste conforme necessário */
  font-size: 16px;
}

.drop-area {
  border: 2px dashed #ad007b;
  padding: 100px;
  margin-top: 50px;
  display: none;
}

.highlight {
  background-color: #303134;
  color: #ad007c65;
}

.editor-container {
  display: none;
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: #202124;
  z-index: 999;
  color: #ff3ec8;
  flex-direction: column;
}

.editor-container div {
  padding: 10px;
}

.editor-container div:first-child {
  display: flex;
  justify-content: space-between;
  padding: 10px;
  height: 50px;
  border-bottom: 1px solid #7b007b;
}

.editor-container textarea {
  box-sizing: border-box;
  background-color: #303134;
  color: #ff3ec8;
  border: 1px solid #7b007b;
  padding: 10px;
  font-size: 16px;
  height: auto;
  font-family: monospace;
  width: 100%;
  flex: 1;
  white-space: pre;
  overflow-wrap: normal;
  overflow: auto;
}
)rawliteral";

const char index_js[] PROGMEM = R"rawliteral(
var buttonsInitialized = false;

function WifiConfig() {
  let wifiSsid = prompt("Please enter the Username of your network", "admin");
  let wifiPwd = prompt("Please enter the Password of your network", "M%L4unch3r");
  if (wifiSsid == null || wifiSsid == "" || wifiPwd == null) {
    window.alert("Invalid User or Password");
  } else {
    xmlhttp = new XMLHttpRequest();
    xmlhttp.open("GET", "/wifi?usr=" + wifiSsid + "&pwd=" + wifiPwd, false);
    xmlhttp.send();
    document.getElementById("status").innerHTML = xmlhttp.responseText;
  }
}

function serialCmd() {
  let cmd = prompt("Enter a serial command", "");
  if (cmd == null || cmd == "" || cmd == null) {
    window.alert("empty command, nothing sent");
  } else {
    const ajax5 = new XMLHttpRequest();
    const formdata5 = new FormData();
    formdata5.append("cmnd", cmd);
    ajax5.open("POST", "/cm", false);
    ajax5.send(formdata5);
    //document.getElementById("status").innerHTML = ajax5.responseText;
    window.alert(ajax5.responseText);
  }
}

function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function () { window.open("/logged-out", "_self"); }, 500);
}

function rebootButton() {
  if (confirm("Confirm Restart?!")) {
    xmlhttp = new XMLHttpRequest();
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/reboot", true);
    xhr.send();
  }
}

function listFilesButton(folders, fs = 'LittleFS', userRequest = false) {
  xmlhttp = new XMLHttpRequest();
  document.getElementById("actualFolder").value = "";
  document.getElementById("actualFolder").value = folders;
  document.getElementById("actualFS").value = fs;

  xmlhttp.onload = function () {
    if (xmlhttp.status === 200) {
      document.getElementById("details").innerHTML = xmlhttp.responseText;
    } else {
      console.error('Requests Error: ' + xmlhttp.status);
    }
  };
  xmlhttp.onerror = function () {
    console.error('Erro na rede ou falha na requisição.');
  };

  xmlhttp.open("GET", "/listfiles?fs=" + fs + "&folder=" + folders, true);
  xmlhttp.send();

  //must first select sd card or littlefs to get access to upload and create
  if (!buttonsInitialized) {
    if (userRequest) {
      if (fs == 'SD') {
        document.getElementById("detailsheader").innerHTML = "<h3>SD Files<h3>";
      } else if (fs == 'LittleFS') {
        document.getElementById("detailsheader").innerHTML = "<h3>LittleFS Files<h3>";
      }

      document.getElementById("updetailsheader").innerHTML = "<h3>Folder Actions:  <button onclick=\"showUploadButtonFancy('" + folders + "')\">Upload File</button><button onclick=\"showCreateFolder('" + folders + "')\">Create Folder</button><button onclick=\"showCreateFile('" + folders + "')\">Create File</button><h3>"
      document.getElementById("updetails").innerHTML = "";
      _("drop-area").style.display = "block";
      buttonsInitialized = true;
      document.getElementById("status").innerHTML = "";
    }
  } else {
    if (userRequest) {
      if (fs == 'SD') {
        document.getElementById("detailsheader").innerHTML = "<h3>SD Files<h3>";
      } else if (fs == 'LittleFS') {
        document.getElementById("detailsheader").innerHTML = "<h3>LittleFS Files<h3>";
      }
    }
  }


}

function renameFile(filePath, oldName) {
  var actualFolder = document.getElementById("actualFolder").value;
  var fs = document.getElementById("actualFS").value;
  let fileName = prompt("Enter the new name: ", oldName);
  if (fileName == null || fileName == "") {
    window.alert("Invalid Name");
  } else {
    const ajax5 = new XMLHttpRequest();
    const formdata5 = new FormData();
    formdata5.append("fs", fs);
    formdata5.append("filePath", filePath);
    formdata5.append("fileName", fileName);
    ajax5.open("POST", "/rename", false);
    ajax5.send(formdata5);
    document.getElementById("status").innerHTML = ajax5.responseText;

    var fs = document.getElementById("actualFS").value;
    listFilesButton(actualFolder, fs, true);
  }
}

function sendIrFile(filePath) {
  if (!confirm("Confirm spamming all codes inside the file?")) return;
  var actualFolder = document.getElementById("actualFolder").value;
  var fs = document.getElementById("actualFS").value;
  const ajax5 = new XMLHttpRequest();
  const formdata5 = new FormData();
  formdata5.append("cmnd", "ir tx_from_file " + filePath);
  ajax5.open("POST", "/cm", false);
  ajax5.send(formdata5);
  document.getElementById("status").innerHTML = ajax5.responseText;
  var fs = document.getElementById("actualFS").value;
  listFilesButton(actualFolder, fs, true);
}

function sendSubFile(filePath) {
  if (!confirm("Confirm sending the codes inside the file?")) return;
  var actualFolder = document.getElementById("actualFolder").value;
  var fs = document.getElementById("actualFS").value;
  const ajax5 = new XMLHttpRequest();
  const formdata5 = new FormData();
  formdata5.append("cmnd", "subghz tx_from_file " + filePath);
  ajax5.open("POST", "/cm", false);
  ajax5.send(formdata5);
  document.getElementById("status").innerHTML = ajax5.responseText;
  var fs = document.getElementById("actualFS").value;
  listFilesButton(actualFolder, fs, true);
}

function runJsFile(filePath) {
  if (!confirm("Confirm executing the selected JS script?")) return;
  var actualFolder = document.getElementById("actualFolder").value;
  var fs = document.getElementById("actualFS").value;
  const ajax5 = new XMLHttpRequest();
  const formdata5 = new FormData();
  formdata5.append("cmnd", "js " + filePath);
  ajax5.open("POST", "/cm", false);
  ajax5.send(formdata5);
  document.getElementById("status").innerHTML = ajax5.responseText;
  var fs = document.getElementById("actualFS").value;
  listFilesButton(actualFolder, fs, true);
}

function runBadusbFile(filePath) {
  if (!confirm("Confirm executing the selected DuckyScript on the machine connected via USB?")) return;
  var actualFolder = document.getElementById("actualFolder").value;
  var fs = document.getElementById("actualFS").value;
  const ajax5 = new XMLHttpRequest();
  const formdata5 = new FormData();
  formdata5.append("cmnd", "badusb run_from_file " + filePath);
  ajax5.open("POST", "/cm", false);
  ajax5.send(formdata5);
  document.getElementById("status").innerHTML = ajax5.responseText;
  var fs = document.getElementById("actualFS").value;
  listFilesButton(actualFolder, fs, true);
}

function decryptAndType(filePath) {
  if (!confirm("Type decrypted file contents on the machine connected via USB?")) return;
  if (!cachedPassword) cachedPassword = prompt("Enter decryption password: ", cachedPassword);
  if (!cachedPassword) return;  // cancelled
  var actualFolder = document.getElementById("actualFolder").value;
  var fs = document.getElementById("actualFS").value;
  const ajax5 = new XMLHttpRequest();
  const formdata5 = new FormData();
  formdata5.append("cmnd", "crypto type_from_file " + filePath + " " + cachedPassword);
  ajax5.open("POST", "/cm", false);
  ajax5.send(formdata5);
  document.getElementById("status").innerHTML = ajax5.responseText;
  var fs = document.getElementById("actualFS").value;
  listFilesButton(actualFolder, fs, true);
}
function downloadDeleteButton(filename, action) {
  /* fs actions: create (folder), createfile, delete, download */
  var fs = document.getElementById("actualFS").value;
  var urltocall = "/file?name=" + filename + "&action=" + action + "&fs=" + fs;
  var actualFolder = document.getElementById("actualFolder").value;
  var option;
  if (action == "delete") {
    option = confirm("Do you really want to DELETE the file: " + filename + " ?\n\nThis action can't be undone!");
  }

  xmlhttp = new XMLHttpRequest();
  if (option == true || action == "create" || action == "createfile") {
    xmlhttp.open("GET", urltocall, false);
    xmlhttp.send();
    document.getElementById("status").innerHTML = xmlhttp.responseText;
    var fs = document.getElementById("actualFS").value;
    listFilesButton(actualFolder, fs, true);
  }

  if (action == "edit") {
    xmlhttp.open("GET", urltocall, false);
    xmlhttp.send();

    if (xmlhttp.status === 200) {
      document.getElementById("editor").value = xmlhttp.responseText;
      document.getElementById("editor-file").innerHTML = filename;
      document.querySelector('.editor-container').style.display = 'flex';
    } else {
      console.error('Requests Error: ' + xmlhttp.status);
    }
  }

  if (action == "download") {
    document.getElementById("status").innerHTML = "";
    window.open(urltocall, "_blank");
  }
}


function cancelEdit() {
  document.querySelector('.editor-container').style.display = 'none';
  document.getElementById("editor").value = "";
  document.getElementById("status").innerHTML = "";
}

function showCreateFolder(folders) {
  var fs = document.getElementById("actualFS").value;
  var uploadform = "";
  //document.getElementById("updetailsheader").innerHTML = "<h3>Create new Folder<h3>"
  document.getElementById("status").innerHTML = "";
  uploadform =
    "<p>Creating folder at: <b>" + folders + "</b>" +
    "<input type=\"hidden\" id=\"folder\" name=\"folder\" value=\"" + folders + "\">" +
    "<input type=\"text\" name=\"foldername\" id=\"foldername\">" +
    "<button onclick=\"CreateFolder()\">Create Folder</button>" +
    "</p>";
  document.getElementById("updetails").innerHTML = uploadform;
}



function CreateFolder() {
  var folderName = "";
  folderName = document.getElementById("folder").value + "/" + document.getElementById("foldername").value;
  downloadDeleteButton(folderName, 'create');
}

function showCreateFile(folders) {
  var fs = document.getElementById("actualFS").value;
  var uploadform = "";
  //document.getElementById("updetailsheader").innerHTML = "<h3>Create new File<h3>"
  document.getElementById("status").innerHTML = "";
  uploadform =
    "<p>Creating file at: <b>" + folders + "</b>" +
    "<input type=\"hidden\" id=\"folder\" name=\"folder\" value=\"" + folders + "\">" +
    "<input type=\"text\" name=\"filename\" id=\"filename\">" +
    "<button onclick=\"CreateFile()\">Create File</button>" +
    "</p>";
  document.getElementById("updetails").innerHTML = uploadform;
}


function CreateFile() {
  var fileName = "";
  fileName = document.getElementById("folder").value + "/" + document.getElementById("filename").value;
  downloadDeleteButton(fileName, 'createfile');
}


function showUploadButtonFancy(folders) {
  //document.getElementById("updetailsheader").innerHTML = "<h3>Upload File<h3>"
  document.getElementById("status").innerHTML = "";
  var uploadform =
    "<p>Send file to " + folders + "</p>" +
    "<form id=\"upload_form\" enctype=\"multipart/form-data\" method=\"post\">" +
    "<input type=\"hidden\" id=\"folder\" name=\"folder\" value=\"" + folders + "\">" +
    "<input type=\"checkbox\" name=\"encryptCheckbox\" id=\"encryptCheckbox\"> Encrypted<br>" +
    "<input type=\"file\" name=\"file1\" id=\"file1\" onchange=\"uploadFile('" + folders + "', 'SD')\"><br>" +
    "<progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:100%;\"></progress>" +
    "<h3 id=\"status\"></h3>" +
    "<p id=\"loaded_n_total\"></p>" +
    "</form>";
  document.getElementById("updetails").innerHTML = uploadform;
}

function _(el) {
  return document.getElementById(el);
}

var cachedPassword = "";

function uploadFile(folder) {
  var fs = document.getElementById("actualFS").value;
  var folder = _("folder").value;
  var files = _("file1").files; // Extract files from input element

  var formdata = new FormData();

  var encrypted = _("encryptCheckbox").checked;
  if (encrypted) {
    cachedPassword = prompt("Enter encryption password (do not lose it, cannot be recovered): ", cachedPassword);
    formdata.append("password", cachedPassword);
  }

  for (var i = 0; i < files.length; i++) {
    formdata.append("files[]", files[i]); // Append each file to form data
  }
  formdata.append("folder", folder);

  var ajax = new XMLHttpRequest();
  ajax.upload.addEventListener("progress", progressHandler, false);
  ajax.addEventListener("load", completeHandler, false);
  ajax.addEventListener("error", errorHandler, false);
  ajax.addEventListener("abort", abortHandler, false);
  ajax.open("POST", "/upload" + fs);
  ajax.send(formdata);
}


function saveFile() {
  var fs = document.getElementById("actualFS").value;
  var folder = document.getElementById("actualFolder").value;
  var fileName = document.getElementById("editor-file").innerText;
  var fileContent = document.getElementById("editor").value;

  const formdata = new FormData();
  formdata.append("fs", fs);
  formdata.append("name", fileName);
  formdata.append("content", fileContent);

  const ajax5 = new XMLHttpRequest();
  ajax5.open("POST", "/edit", false);
  ajax5.send(formdata);

  document.getElementById("status").innerText = ajax5.responseText;
  listFilesButton(folder, fs, true);
}


// Drag and drop event listeners
window.addEventListener("load", function () {
  var dropArea = _("drop-area");
  dropArea.addEventListener("dragenter", dragEnter, false);
  dropArea.addEventListener("dragover", dragOver, false);
  dropArea.addEventListener("dragleave", dragLeave, false);
  dropArea.addEventListener("drop", drop, false);
});

function dragEnter(event) {
  event.stopPropagation();
  event.preventDefault();
  this.classList.add("highlight");
}

function dragOver(event) {
  event.stopPropagation();
  event.preventDefault();
  this.classList.add("highlight");
}

function dragLeave(event) {
  event.stopPropagation();
  event.preventDefault();
  this.classList.remove("highlight");
}
var fileQueue = [];
var currentFileIndex = 0;

function drop(event, folder) {
  event.stopPropagation();
  event.preventDefault();
  _("drop-area").classList.remove("highlight");

  fileQueue = event.dataTransfer.files;
  currentFileIndex = 0;
  var fs = document.getElementById("actualFS").value;

  var uploadform =
    "<p>Send file to " + folder + "</p>" +
    "<form id=\"upload_form\" enctype=\"multipart/form-data\" method=\"post\">" +
    "<progress id=\"progressBar\" value=\"0\" max=\"100\" style=\"width:100%;\"></progress>" +
    "<h3 id=\"status\"></h3>" +
    "<p id=\"loaded_n_total\"></p>" +
    "</form>";
  document.getElementById("updetails").innerHTML = uploadform;

  if (fileQueue.length > 0) {
    uploadNextFile(folder, fs);
  }
}

function uploadNextFile(folder, fs) {
  if (currentFileIndex >= fileQueue.length) {
    console.log("Upload complete");
    listFilesButton(folder, fs, true);
    return;
  }

  var file = fileQueue[currentFileIndex];
  var formdata = new FormData();
  formdata.append("file", file);
  formdata.append("folder", folder);

  var ajax = new XMLHttpRequest();
  ajax.upload.addEventListener("progress", progressHandler, false);
  ajax.addEventListener("load", completeHandler, false);
  ajax.addEventListener("error", errorHandler, false);
  ajax.addEventListener("abort", abortHandler, false);
  ajax.open("POST", "/upload" + fs);
  ajax.send(formdata);
}

function progressHandler(event) {
  _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes";
  var percent = (event.loaded / event.total) * 100;
  _("progressBar").value = Math.round(percent);
  if (percent >= 100) {
    _("status").innerHTML = "Please wait, writing file to filesystem";
  }
}
function completeHandler(event) {
  _("progressBar").value = 0;
  if (fileQueue.length > 0) {
    currentFileIndex++;
    if (currentFileIndex <= fileQueue.length) {
      document.getElementById("status").innerHTML = "Uploaded " + currentFileIndex + " of " + fileQueue.length + " files.";
    }
    uploadNextFile(document.getElementById("actualFolder").value, document.getElementById("actualFS").value);
  }
  else {
    _("status").innerHTML = "Upload Complete";
    var actualFolder = document.getElementById("actualFolder").value
    document.getElementById("status").innerHTML = "File Uploaded";
    var fs = document.getElementById("actualFS").value;
    listFilesButton(actualFolder, fs, true);
  }
}
function errorHandler(event) {
  _("status").innerHTML = "Upload Failed";
  if (fileQueue.length > 0) {
    currentFileIndex++;
    document.getElementById("status").innerHTML = "Uploaded " + i + " of " + files.length + " files, please wait.";
    uploadNextFile(document.getElementById("actualFolder").value, document.getElementById("actualFS").value);
  }
}
function abortHandler(event) {
  _("status").innerHTML = "inUpload Aborted";
  if (fileQueue.length > 0) {
    currentFileIndex++;
    document.getElementById("status").innerHTML = "Uploaded " + i + " of " + files.length + " files, please wait.";
    uploadNextFile(document.getElementById("actualFolder").value, document.getElementById("actualFS").value);
  }
}

window.addEventListener("load", function () {
  var actualFolder = document.getElementById("actualFolder").value
  var fs = document.getElementById("actualFS").value;
  document.getElementById("status").innerHTML = "Please select the storage you want to manage (SD or LittleFS).";
  listFilesButton(actualFolder, fs, true);
});


document.getElementById("editor").addEventListener("keydown", function (e) {
  if (e.key === 's' && e.ctrlKey) {
    e.preventDefault();
    saveFile();
  }

  // tab
  if (e.key === 'Tab') {
    e.preventDefault();
    var cursorPos = document.getElementById("editor").selectionStart;
    var textBefore = document.getElementById("editor").value.substring(0, cursorPos);
    var textAfter = document.getElementById("editor").value.substring(cursorPos);
    document.getElementById("editor").value = textBefore + "  " + textAfter;
    document.getElementById("editor").selectionStart = cursorPos + 2;
    document.getElementById("editor").selectionEnd = cursorPos + 2;
  }

});

document.getElementById("editor").addEventListener("keyup", function (e) {
  if (e.key === 'Escape') {
    cancelEdit();
  }

  // map special characters to their closing pair
  map_chars = {
    "(": ")",
    "{": "}",
    "[": "]",
    '"': '"',
    "'": "'",
    "`": "`",
    "<": ">"
  };

  // if the key pressed is a special character, insert the closing pair
  if (e.key in map_chars) {
    var cursorPos = document.getElementById("editor").selectionStart;
    var textBefore = document.getElementById("editor").value.substring(0, cursorPos);
    var textAfter = document.getElementById("editor").value.substring(cursorPos);
    document.getElementById("editor").value = textBefore + map_chars[e.key] + textAfter;
    document.getElementById("editor").selectionStart = cursorPos;
    document.getElementById("editor").selectionEnd = cursorPos;
  }

});
)rawliteral";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <link rel="stylesheet" href="style.css">
</head>
<body>
  <div class="container">
    <div class="float-element"><a onclick="logoutButton()" href='javascript:void(0);'>[X]</a></div>
    <h1 align="center">BRUCE Firmware</h1>
    <p>Firmware for offensive pranks and pentest studies and analysis. For educational purposes only. Don't use in environments where you are not allowed. All responsibilities for irresponsible usage of this firmware rest on your fin, sharky. Sincerely, Bruce.</p>
    <p>Firmware version: %FIRMWARE%</p>
    <p>SD Free Storage: <span id="freeSD">%FREESD%</span> | Used: <span id="usedSD">%USEDSD%</span> | Total: <span id="totalSD">%TOTALSD%</span></p>
    <p>LittleFS Free Storage: <span id="freeSD">%FREELittleFS%</span> | Used: <span id="usedSD">%USEDLittleFS%</span> | Total: <span id="totalSD">%TOTALLittleFS%</span></p>
    <p>
    <form id="save" enctype="multipart/form-data" method="post">
      <input type="hidden" id="actualFolder" name="actualFolder" value="/">
      <input type="hidden" id="actualFS" name="actualFS" value="LittleFS">
    </form>
    <button onclick="rebootButton()">Reboot</button>
    <button onclick="WifiConfig()">Usr/Pass</button>
    <button onclick="serialCmd()">SerialCmd</button>
    <button onclick="listFilesButton('/', 'SD', true)">SD Files</button>
    <button onclick="listFilesButton('/', 'LittleFS', true)">LittleFS</button>

    </p>
    <p id="detailsheader"></p>
    <p id="details"></p>
    <p id="updetailsheader"></p>
    <p id="updetails"></p>
    <div id="drop-area" class="drop-area" ondrop="drop(event, document.getElementById('actualFolder').value)">
        <p style="text-align: center;">Drag and drop files here</p>
    </div>
    <p id="status"></p>

    <div class="editor-container">
      <div>
        <button onclick="cancelEdit()"><strong>⬅</strong> Back</button>
        <h2>WebUi Editor</h2>
        <span></span>
      </div>
      <div>
        file: <span id="editor-file"></span>
      </div>
      <textarea id="editor" spellcheck="false"></textarea>
      <div>
        <button onclick="saveFile()">✔️​ Save</button>
        <button onclick="cancelEdit()">❌​ Cancel</button>
      </div>
    </div>

  </div>

<script src="script.js"></script>
</body>
</html>
)rawliteral";

const char logout_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="UTF-8">
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      margin: 0;
      padding: 20px;
      color: #ad007b;
      background-color: #202124;
    }

    h3 {
      margin: 0;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255, 255, 255, 0.1);
    }
  </style>
</head>
<body>
  <h3><a href="/">Log Back In</a></h3>
</body>
</html>
)rawliteral";


const char page_404[] PROGMEM = R"rawliteral(
<script language="javascript">
<!--
document.write(unescape('%3C%68%74%6D%6C%3E%0A%3C%68%65%61%64%3E%0A%3C%74%69%74%6C%65%3E%53%69%6D%70%6C%65%20%34%30%34%20%45%72%72%6F%72%20%50%61%67%65%20%44%65%73%69%67%6E%3C%2F%74%69%74%6C%65%3E%0A%3C%6C%69%6E%6B%20%68%72%65%66%3D%22%68%74%74%70%73%3A%2F%2F%66%6F%6E%74%73%2E%67%6F%6F%67%6C%65%61%70%69%73%2E%63%6F%6D%2F%63%73%73%3F%66%61%6D%69%6C%79%3D%52%6F%62%6F%74%6F%3A%37%30%30%22%20%72%65%6C%3D%22%73%74%79%6C%65%73%68%65%65%74%22%3E%0A%3C%73%74%79%6C%65%3E%0A%68%31%7B%0A%66%6F%6E%74%2D%73%69%7A%65%3A%38%30%70%78%3B%0A%66%6F%6E%74%2D%77%65%69%67%68%74%3A%38%30%30%3B%0A%74%65%78%74%2D%61%6C%69%67%6E%3A%63%65%6E%74%65%72%3B%0A%66%6F%6E%74%2D%66%61%6D%69%6C%79%3A%20%27%52%6F%62%6F%74%6F%27%2C%20%73%61%6E%73%2D%73%65%72%69%66%3B%0A%7D%0A%68%32%0A%7B%0A%66%6F%6E%74%2D%73%69%7A%65%3A%32%35%70%78%3B%0A%74%65%78%74%2D%61%6C%69%67%6E%3A%63%65%6E%74%65%72%3B%0A%66%6F%6E%74%2D%66%61%6D%69%6C%79%3A%20%27%52%6F%62%6F%74%6F%27%2C%20%73%61%6E%73%2D%73%65%72%69%66%3B%0A%6D%61%72%67%69%6E%2D%74%6F%70%3A%2D%34%30%70%78%3B%0A%7D%0A%70%7B%0A%74%65%78%74%2D%61%6C%69%67%6E%3A%63%65%6E%74%65%72%3B%0A%66%6F%6E%74%2D%66%61%6D%69%6C%79%3A%20%27%52%6F%62%6F%74%6F%27%2C%20%73%61%6E%73%2D%73%65%72%69%66%3B%0A%66%6F%6E%74%2D%73%69%7A%65%3A%31%32%70%78%3B%0A%7D%0A%0A%2E%63%6F%6E%74%61%69%6E%65%72%0A%7B%0A%77%69%64%74%68%3A%33%30%30%70%78%3B%0A%6D%61%72%67%69%6E%3A%20%30%20%61%75%74%6F%3B%0A%6D%61%72%67%69%6E%2D%74%6F%70%3A%31%35%25%3B%0A%7D%0A%3C%2F%73%74%79%6C%65%3E%0A%3C%2F%68%65%61%64%3E%0A%3C%62%6F%64%79%3E%0A%3C%64%69%76%20%63%6C%61%73%73%3D%22%63%6F%6E%74%61%69%6E%65%72%22%3E%0A%3C%68%31%3E%34%30%34%3C%2F%68%31%3E%0A%3C%68%32%3E%50%61%67%65%20%4E%6F%74%20%46%6F%75%6E%64%3C%2F%68%32%3E%0A%3C%70%3E%54%68%65%20%50%61%67%65%20%79%6F%75%20%61%72%65%20%6C%6F%6F%6B%69%6E%67%20%66%6F%72%20%64%6F%65%73%6E%27%74%20%65%78%69%73%74%20%6F%72%20%61%6E%20%6F%74%68%65%72%20%65%72%72%6F%72%20%6F%63%63%75%72%65%64%2E%20%47%6F%20%74%6F%20%3C%61%20%68%72%65%66%3D%22%2F%22%3E%48%6F%6D%65%20%50%61%67%65%2E%3C%2F%61%3E%3C%2F%70%3E%0A%3C%21%2D%2D%20%59%6F%75%20%6A%75%73%74%20%73%63%72%61%74%63%68%65%64%20%74%68%65%20%73%75%72%66%61%63%65%2E%2E%2E%20%73%20%68%20%61%20%72%20%6B%20%79%20%2D%2D%21%3E%0A%3C%2F%64%69%76%3E%0A%3C%2F%62%6F%64%79%3E%0A%3C%2F%68%74%6D%6C%3E'));
//-->
</script>

)rawliteral";

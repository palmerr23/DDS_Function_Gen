/*
  adapted from 
  esp32-asyncwebserver-fileupload-example

  https://github.com/smford/esp32-asyncwebserver-fileupload-example/tree/master/example-02
*/

void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
String humanReadableSize(const size_t bytes);
String listFiles(bool ishtml);

bool __shouldReboot = false;            // schedule a reboot

void configureUtilityPages() {
  // configure web server

  // run handleUpload function when any file is uploaded
  server.onFileUpload(handleUpload); 

  server.on("/utility", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + + " " + request->url();
    Serial.println(logmessage);
    request->send_P(200, "text/html", utility_html);   
  });

  server.on("/listfiles", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);
    request->send(200, "text/plain", listFiles(true));
  });

  server.on("/file", HTTP_GET, [](AsyncWebServerRequest * request) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    if (request->hasParam("name") && request->hasParam("action")) {
      const char *fileName = request->getParam("name")->value().c_str();
      const char *fileAction = request->getParam("action")->value().c_str();

      logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url() + "?name=" + String(fileName) + "&action=" + String(fileAction);

      if (!FILESYS.exists(fileName)) {
        Serial.println(logmessage + " ERROR: file does not exist");
        request->send(400, "text/plain", "ERROR: file does not exist");
      } else {
        Serial.println(logmessage + " file exists");
        if (strcmp(fileAction, "download") == 0) {
          logmessage += " downloaded";
          request->send(FILESYS, fileName, "application/octet-stream");
        } else if (strcmp(fileAction, "delete") == 0) {
          logmessage += " deleted";
          FILESYS.remove(fileName);
          request->send(200, "text/plain", "Deleted File: " + String(fileName));
        } else {
          logmessage += " ERROR: invalid action param supplied";
          request->send(400, "text/plain", "ERROR: invalid action param supplied");
        }
        Serial.println(logmessage);
      }
    } else {
      Serial.println(logmessage);
      request->send(400, "text/plain", "ERROR: name and action params required");
    }
  });
}


// handles uploads to the filserver
void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (!index) {
    logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = FILESYS.open("/" + filename, "w");
    Serial.println(logmessage);
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    Serial.println(logmessage);
  }

  if (final) {
    logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    Serial.println(logmessage);
    request->redirect("/utility");
  }
}

// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(bool ishtml) {
  FSInfo fs_info;
  String returnText = "";
  if (!ishtml) 
    returnText += "Files:\n";
  File root = FILESYS.open("/", "r");
  File foundfile = root.openNextFile();

  if (ishtml) {
    returnText += "<table><tr style='padding-left:10px'><th align='left' style='padding-left:10px'>Name</th><th align='left' style='padding-left:10px'>Size</th><th></th><th></th></tr>";
  }
  if(!foundfile)
   if (!ishtml) 
      returnText += " No files found\n";
    else
      returnText += "<tr><td colspan=4 style='padding-left:10px'> No files found</td></tr>";

  while (foundfile) {
    if (ishtml) {
      returnText += "<tr align='left' style='padding-left:10px'><td style='padding-left:10px'>" + String(foundfile.name()) + "</td><td style='padding-left:10px'>" + humanReadableSize(foundfile.size()) + "</td>";
      returnText += "<td style='padding-left:10px'><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
      returnText += "<td style='padding-left:10px'><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";
    } else {
      returnText += " " + String(foundfile.name()) + " (" + humanReadableSize(foundfile.size()) + ")\n";
    }
    foundfile = root.openNextFile();
  }
  if (ishtml) {
    returnText += "</table>";
  }
  root.close();
  foundfile.close();
  FILESYS.info(fs_info);
  if (ishtml) {
    returnText += "<BR>" + humanReadableSize(fs_info.totalBytes) + " file system<BR>";
    returnText += humanReadableSize(fs_info.usedBytes) + " used, including directory structure (" + humanReadableSize(fs_info.blockSize) + " block size)<BR>";
  }
  else
  {
    returnText += " " + humanReadableSize(fs_info.totalBytes) + " file system\n";
    returnText += " " + humanReadableSize(fs_info.usedBytes) + " used, including directory structure (" + humanReadableSize(fs_info.blockSize) + " block size)\n";
  }
  return returnText;
}

void rebootPico(String message) {
  Serial.println("Rebooting Pico:"); Serial.println(message);
  delay(5000);
  rp2040.reboot();
}


// Make size of files human readable
// source: https://github.com/CelliesProjects/minimalUploadAuthESP32
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

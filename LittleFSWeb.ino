/* ----------------------------------------------------------------------------

LittleFSWeb.ino : Web interface to the LittleFS filesystem.

  Use /LittleFSWeb/LittleFSWeb.html to
    have an overview of the files and directories in LittleFS 
    create and delete files and directories
    rename files and directories
    upload and download files
    copy and edit files
    and for a clean start you can format which deletes everything

  For your main app : 

  After creating your webserver named MyWebServer with :
    AsyncWebServer MyWebServer(80);               // Create web server
  call:
    LittleFSWebinit(adminuser, adminpassword);    // Setup LittleFSWeb server
    LittleFSWebconfigureWebServer(MyWebServer);   // Provide web services

  and you have your LittleFSWeb available on url /LittleFSWeb/LittleFSWeb.html

  NOTES:

    You can use the editor to change /LittleFSWeb/LittleFSWeb.html itself but
    safer is :
    - make a copy to for example /LittleFSWeb/MyTest.html
    - edit and test that file until you are happy with it
    - edit and save your result in /LittleFSWeb/LittleFSWeb.html    

LittleFS.h is part of the esp8266 core for Arduino environment.
LittleFSWeb.h is the file which goes with this LittleFSWeb.ino

---------------------------------------------------------------------------- */

#include <LittleFS.h>
#include "LittleFSWeb.h"

// ====== LittleFSWeb is protected by username and password

String LittleFSWebuser = "admin";
String LittleFSWebpassword = "admin";

// ===== Used by LittleFSWebCopyFile (supports 1 concurrent copy action)

bool LittleFSCopyStatus = false;
//String LittleFSCopyStatus = "-";                // 1 concurrent copy
//#define LittleFSCopying "Copying"               // value while copying
//#define LittleFSCopyCompleted "Copy Completed"  // value after copy completed

// ====== LittleFSWebEdit (Open file to save once and then save file in chunks)

bool LittleFSWebEditFileOpen = false;
File LittleFSWebEditFile;

// ------------------------------------------------------- Initialise Web Server

// ====== LittleFSWebinit (use username and password as set in main app)

void LittleFSWebinit(String &username, String &password) {

  LittleFSWebuser = username;
  LittleFSWebpassword = password;
}

// -------------------------------------------------------- Configure Web Server

// ====== Configure web server

void LittleFSWebConfigureWebServer(AsyncWebServer &server) {

  _SERIAL_PRINTLN(F("Configuring LittleFSWeb ..."));

  // ----- Put LittleFSWeb.html on web server when it does not exist.

  if (!LittleFS.exists(F("/LittleFSWeb/LittleFSWeb.html"))) {
    File file = LittleFS.open(F("/LittleFSWeb/LittleFSWeb.html"), "w");
    file.print(FPSTR(LittleFSWeb));
    file.close();
  }

  // ----- Add web services

  // 'Easter Egg' LittleFSWeb.html which is in the file can also be used like:

  server.on("/LittleFSWebFixIt", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", LittleFSWeb);
  });

  // Logs you out

  server.on("/LittleFSWeb/logout", HTTP_GET, LittleFSWebLogout);

  // LittleFS format action

  server.on("/LittleFSWeb/format", HTTP_GET, LittleFSWebFormat);

  // Get directory and file details which LittleFSWeb.html converts to table

  server.on("/LittleFSWeb/listfiles", HTTP_GET, LittleFSWebListFiles);

  // Directory functions new directory and delete directory tree

  server.on("/LittleFSWeb/dir", HTTP_GET, LittleFSWeb_Dir_New_Del);

  // File actions new, copy, delete, rename and directory rename

  server.on("/LittleFSWeb/file", HTTP_GET, LittleFSWeb_New_Copy_Del_Ren);

  // Step 1 Handle File uploads : set upload directory for file uploads

  server.on("/LittleFSWeb/setUploadDirectory",
            HTTP_GET, LittleFSWebUploadSetDirectory);

  // Step 2 Handle File uploads : actual file uploads

  server.onFileUpload(LittleFSWebUploadFile);

  // Fetch file via /LittleFSWebFetch/ for editor and download
  // (Work around proc_processor so we can edit and download files with %'s)

  server.on("/LittleFSWebFetch*", HTTP_GET, LittleFSWebFetchFile);

  // Save edited file (2 functions)

  server.on(
    "/LittleFSWeb/saveFile",
    HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200);  // function 1 : send 200
    },
    LittleFSWebSaveFile);  // function 2 : asynchronous save

  server.on("/LittleFSWeb/lastSaveStatus", HTTP_GET, LittleFSWebLastSaveStatus);

}

// -------------------------------------------------------- Some basic functions

// ====== Get File System Info

int LittleFSWebFsSize(char what) {
  int result;
  FSInfo fsinfo;
  LittleFS.info(fsinfo);
  switch (what) {
    case 't': result = fsinfo.totalBytes; break;
    case 'u': result = fsinfo.usedBytes; break;
    case 'f': result = (fsinfo.totalBytes - fsinfo.usedBytes); break;
    default: result = fsinfo.totalBytes; break;
  }
  return result;
}

// ====== Clear Directory

void LittleFSWeb_ClearDir(const char *path) {
  Dir dir = LittleFS.openDir(path);
  while (dir.next()) {
    if (dir.isDirectory()) {
      // Recursive delete
      LittleFSWeb_ClearDir((String(path) + F("/") + dir.fileName()).c_str());
    } else {
      LittleFS.remove(String(path) + F("/") + dir.fileName());
    }
  }
  LittleFS.rmdir(String(path));
}

// ====== Check authentication

bool LittleFSWebCheckLogon(AsyncWebServerRequest *request, bool log) {
  // used by server.on functions to decide whether a user has logged on
  // log is false for file upload and save functions to avoid massive logging
  bool state = request->authenticate(
    LittleFSWebuser.c_str(),
    LittleFSWebpassword.c_str());
  if (log && (!state)) { _SERIAL_PRINTLN(F("Is not authenticated")); }
  return state;
}

// ---------------------------------------------------- LittleFSWeb web services

// ====== Logout

void LittleFSWebLogout(AsyncWebServerRequest *request) {
  request->requestAuthentication();
  request->send(401);
}

// ====== Format

void LittleFSWebFormat(AsyncWebServerRequest *request) {
  if (LittleFSWebCheckLogon(request, true)) {
    if (request->hasParam(F("name")) && request->hasParam(F("action"))) {
      const char *name = request->getParam(F("name"))->value().c_str();
      const char *action = request->getParam(F("action"))->value().c_str();
      if ((strcmp(name, "Format") == 0) && (strcmp(action, "format") == 0)) {
        LittleFS.format();
        if (LittleFS.begin()) {
          _SERIAL_PRINTLN(F(" LittleFS Formatted !!"));
          request->send(200, F("text/plain"),
                        F("LittleFS Formatted"));
        } else {
          _SERIAL_PRINTLN(F(" LittleFS Format Failed !!"));
          request->send(400, F("text/plain"),
                        F("ERROR: LittleFS Format Failed !!"));
        }
      }
    } else {
      request->send(400, F("text/plain"),
                    F("ERROR: name and action params required"));
    }
  } else {
    _SERIAL_PRINTLN(F(" Authentication Failed"));
    return request->requestAuthentication();
  }
}

// ====== LittleFSWeb.html asks for overview of all directory and file details.

uint16_t LittleFSWeb_dirs;   // number of directories
uint16_t LittleFSWeb_files;  // number of files
uint32_t LittleFSWeb_bytes;  // total bytes used

String LittleFSWebFileList;  // The list with all info

void LittleFSWebListFiles(AsyncWebServerRequest *request) {
  if (LittleFSWebCheckLogon(request, true)) {
    LittleFSWebCreateFileList("/", "|__", true);
    request->send(200, "text/html", LittleFSWebFileList);
  } else {
    _SERIAL_PRINTLN(F(" Authentication Failed"));
    request->requestAuthentication();
  }
}

// LittleFSWebCreateFileList is a recursive function.

void LittleFSWebCreateFileList(String path, String indent, bool level0) {
  if (level0) {
    _SERIAL_PRINTF_P1(PSTR("\nListing contents for '%s'...\n"), path);
    LittleFSWeb_dirs = 0;
    LittleFSWeb_files = 0;
    LittleFSWeb_bytes = 0;
    LittleFSWebFileList = "";
  }
  Dir dir = LittleFS.openDir(path);
  while (dir.next()) {
    if (LittleFSWebFileList != "") { LittleFSWebFileList.concat("|"); }
    LittleFSWebFileList.concat(path);
    LittleFSWebFileList.concat(dir.fileName());
    if (dir.isDirectory()) {
      _SERIAL_PRINTF_P2(PSTR("%s%s\n"),
                        path.c_str(),
                        dir.fileName().c_str());
      ++LittleFSWeb_dirs;
      LittleFSWebFileList.concat(F(",[Dir]"));
      LittleFSWebCreateFileList(path + dir.fileName() + F("/"),
                                indent + indent, false);
    } else {
      _SERIAL_PRINTF_P2(PSTR("%-40s %+10s\n"),
                        (path + dir.fileName()).c_str(),
                        LittleFSWebFileSize(dir.fileSize()));
      ++LittleFSWeb_files;
      LittleFSWebFileList.concat(F(","));
      LittleFSWebFileList.concat(LittleFSWebFileSize(dir.fileSize()));
    }
    LittleFSWeb_bytes = LittleFSWeb_bytes + (uint32_t)dir.fileSize();
  }
  if (level0) {
    LittleFSWebFileList.concat(F("|Counts,"));
    LittleFSWebFileList.concat(String(LittleFSWeb_dirs));
    LittleFSWebFileList.concat(F(","));
    LittleFSWebFileList.concat(String(LittleFSWeb_files));

    LittleFSWebFileList.concat(F(","));  // Used
//    LittleFSWebFileList.concat(LittleFSWebFileSize(LittleFSWeb_bytes));
    LittleFSWebFileList.concat(LittleFSWebFileSize(LittleFSWebFsSize('u')));
    
    LittleFSWebFileList.concat(F(","));  // Available
//    LittleFSWebFileList.concat(LittleFSWebFileSize(LittleFSWebFsSize('t')
//                                                   - LittleFSWeb_bytes));
    LittleFSWebFileList.concat(LittleFSWebFileSize(LittleFSWebFsSize('f')));

    LittleFSWebFileList.concat(F(","));  // Total FS size
    LittleFSWebFileList.concat(LittleFSWebFileSize(LittleFSWebFsSize('t')));
    LittleFSWebFileList.concat(F("|LittleFSCopyStatus,"));

    if (LittleFSCopyStatus) {
      LittleFSWebFileList.concat(F("Copying"));
    } else {
      LittleFSWebFileList.concat(F("Not Copying"));
    }

    _SERIAL_PRINTF_P2(PSTR("%+40s %+10s\n"),
                      (String(LittleFSWeb_dirs) + F(" Dirs; ")
                       + String(LittleFSWeb_files) + F(" Files > "))
                        .c_str(),
                      (LittleFSWebFileSize(LittleFSWebFsSize('u'))).c_str());
    _SERIAL_PRINTF_P2(PSTR("%+40s %+10s\n"),
                      String(F(" Available > ")).c_str(),
                      (LittleFSWebFileSize(LittleFSWebFsSize('f'))).c_str());
    _SERIAL_PRINTF_P2(PSTR("%+40s %+10s\n\n"),
                      String(F(" Total > ")).c_str(),
                      (LittleFSWebFileSize(LittleFSWebFsSize('t'))).c_str());
    _SERIAL_PRINTF_P1(PSTR("Listing complete for '%s'\n"), path);
  }
}

String LittleFSWebFileSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + F(" B");
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + F(" KB");
  else return String(bytes / 1024.0 / 1024.0) + F(" MB");
}

// ====== Directory : new and delete

void LittleFSWeb_Dir_New_Del(AsyncWebServerRequest *request) {
  if (LittleFSWebCheckLogon(request, true)) {

    const char *dirName = request->getParam(F("name"))->value().c_str();
    const char *dirAction = request->getParam(F("action"))->value().c_str();
    // New Dir
    if (strcmp(dirAction, "newdir") == 0) {
      if (LittleFS.exists(dirName)) {
        request->send(400,
                      F("text/plain"),
                      F("<font color=red>ERROR: directory ")
                        + String(dirName) + F(" already exists</font>"));
      } else {
        // check name length
        String newnameShort;
        if (String(dirName).lastIndexOf(F("/")) == -1) {
          newnameShort = String(dirName);
        } else {
          uint8_t start = String(dirName).lastIndexOf(F("/")) + 1;
          newnameShort = String(dirName).substring(start);
        }
        if (newnameShort.length() > 31) {
          request->send(200,
                        F("text/plain"),
                        F("<font color=red>ERROR: name too long (max 31): ")
                          + String(dirName) + F("</font>"));
        } else {
          LittleFS.mkdir(dirName);
          if (LittleFS.exists(dirName)) {
            request->send(200,
                          F("text/plain"),
                          F("Created Directory: ") + String(dirName));
          } else {
            request->send(
              200,
              F("text/plain"),
              F("<font color=red>ERROR: Failed To Created Directory: ")
                + String(dirName) + F("</font>"));
          }
        }
      }
    }
    // Del Dir
    if (strcmp(dirAction, "deldir") == 0) {
      if (!LittleFS.exists(dirName)) {
        request->send(
          400,
          F("text/plain"),
          F("<font color=red>ERROR: directory does not exist</font>"));
      } else {
        LittleFSWeb_ClearDir(dirName);
        request->send(200,
                      F("text/plain"),
                      F("Deleted Directory: ")
                        + String(dirName));
      }
    }
  } else {
    _SERIAL_PRINTLN(F(" Authentication Failed"));
    return request->requestAuthentication();
  }
}

// ====== File new, copy, delete, rename and directory rename.

void LittleFSWeb_New_Copy_Del_Ren(AsyncWebServerRequest *request) {
  if (LittleFSWebCheckLogon(request, true)) {
    if (request->hasParam(F("name")) && request->hasParam(F("action"))) {
      const char *fileName = request->getParam(F("name"))->value().c_str();
      const char *fileAction = request->getParam(F("action"))->value().c_str();
      String fileActionStr = fileAction;

      if (!LittleFS.exists(fileName)) {

        // New file

        if (fileActionStr.substring(0, 7) == F("newfile")) {
          File file = LittleFS.open(fileName, "w");
          file.print("");
          file.close();
          request->send(
            200, F("text/plain"),
            F("Created File: ") + String(fileName));
        } else {
          request->send(
            400, F("text/plain"),
            F("<font color=red>ERROR: file does not exist</font>"));
        }
      } else {

        if (fileActionStr.substring(0, 4) == F("copy")) {

          //Copy file

          // check if copy fits in free space
          auto file = LittleFS.open(fileName, "r");
          size_t filesize = file.size();
          file.close();

          //_SERIAL_PRINTLN("Size : "+String(filesize ));
          //_SERIAL_PRINTLN("Free : "+String(LittleFSWebFsSize('f') ));
          if (filesize < LittleFSWebFsSize('f')) {
            String newname = fileActionStr.substring(5);
            // check name length
            String newnameShort;
            if (newname.lastIndexOf(F("/")) == -1) {
              newnameShort = newname;
            } else {
              newnameShort = newname.substring(newname.lastIndexOf(F("/")) + 1);
            }
            if (newnameShort.length() > 31) {
              request->send(
                200, F("text/plain"),
                F("<font color=red>ERROR: name too long (max 31): ")
                  + String(newname) + F("</font>"));
            } else {
              if (!LittleFS.exists(newname)) {
                request->send(
                  200, F("text/plain"),
                  F("Copying File: '") + String(fileName)
                    + F("' to '") + newname + F("' "));
                LittleFSWeb_CopyFile(fileName, newname.c_str());
              } else {
                request->send(
                  200, F("text/plain"),
                  F("<font color=red>ERROR: ")
                    + String(newname) + F(" is already there</font>"));
              }
            }
          } else {
            request->send(200, F("text/plain"),
                          F("<font color=red>ERROR: Disk Full</font>"));
          }

        } else if (strcmp(fileAction, "delete") == 0) {

          // Delete file

          LittleFS.remove(fileName);
          request->send(200, F("text/plain"),
                        F("Deleted File: ") + String(fileName));

        } else if (fileActionStr.substring(0, 6) == F("rename")) {

          // Rename file / directory

          String newname = fileActionStr.substring(7);
          // check name length
          String newnameShort;
          if (newname.lastIndexOf(F("/")) == -1) {
            newnameShort = newname;
          } else {
            newnameShort = newname.substring(newname.lastIndexOf(F("/")) + 1);
          }
          if (newnameShort.length() > 31) {
            request->send(
              200, F("text/plain"),
              F("<font color=red>ERROR: name too long (max 31): ")
                + String(newname) + F("</font>"));
          } else {
            if (!LittleFS.exists(newname)) {
              LittleFS.rename(fileName, newname.c_str());
              if (LittleFS.exists(newname)) {
                request->send(
                  200, F("text/plain"),
                  F("Renamed File: '") + String(fileName)
                    + F("' to '") + newname + F("' "));
              } else {
                request->send(
                  200, F("text/plain"),
                  F("<font color=red>ERROR: File Rename Failed: '")
                    + String(fileName) + F("' to '") + newname + F("</font>"));
              }
            } else {
              request->send(
                200, F("text/plain"),
                F("<font color=red>ERROR: ")
                  + String(newname) + F(" is already there</font>"));
            }
          }
        } else {
          request->send(
            400, F("text/plain"),
            F("<font color=red>ERROR: invalid action param supplied</font>"));
        }
      }
    } else {
      request->send(
        400, F("text/plain"),
        F("<font color=red>ERROR: name and action params required</font>"));
    }
  } else {
    _SERIAL_PRINTLN(F(" Authentication Failed"));
  }
}

// ====== File Upload : Set folder to upload to

String LittleFSWebUploadDirectory = "/";  // Default to root folder

void LittleFSWebUploadSetDirectory(AsyncWebServerRequest *request) {
  if (LittleFSWebCheckLogon(request, true)) {
    if (request->hasParam(F("name"))) {
      LittleFSWebUploadDirectory = request->getParam(F("name"))->value();
      _SERIAL_PRINTLN(F("LittleFSWebUploadDirectory: ")
                      + LittleFSWebUploadDirectory);
      request->send(200, F("text/plain"), String(LittleFSWebFsSize('f')));
    } else {
      request->send(400, F("text/plain"), F("ERROR: directory name required"));
    }
  } else {
    _SERIAL_PRINTLN(F(" Authentication Failed"));
    return request->requestAuthentication();
  }
}

// ====== File Upload : Handle file upload

// ICACHE_RAM_ATTR

void LittleFSWebUploadFile(
  AsyncWebServerRequest *request, String filename, size_t index,
  uint8_t *data, size_t len, bool final) {
  // make sure authenticated before allowing upload
  if (LittleFSWebCheckLogon(request, false)) {
    // check name length
    String newnameShort;
    if (filename.lastIndexOf(F("/")) == -1) {
      newnameShort = filename;
    } else {
      newnameShort = filename.substring(filename.lastIndexOf(F("/")) + 1);
    }
    if (newnameShort.length() > 31) {
      request->send(
        200, F("text/plain"),
        F("<font color=red>ERROR: name too long (max 31): ")
          + String(filename) + F("</font>"));
    } else {

      // Receive the file

      if (!index) {  // Open file and remember file handle in request
        _SERIAL_PRINTLN(F("Uploading  : ")
                        + LittleFSWebUploadDirectory + String(filename));
        request->_tempFile = LittleFS.open(
          LittleFSWebUploadDirectory + filename, "w");
      }

      if (len) {  // We receive some data so add that to the open file
        request->_tempFile.write(data, len);
        _SERIAL_PRINTLN(F("Uploading : ") + LittleFSWebUploadDirectory
                        + String(filename) + F(" index = ")
                        + String(index) + F(" len = ") + String(len));
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      }

      // We received the last part
      if (final) {
        _SERIAL_PRINTLN(F("Uploaded  : ")
                        + LittleFSWebUploadDirectory + String(filename)
                        + F(", size: ") + String(index + len));
        request->_tempFile.close();
        request->redirect(F("/"));
      }
    }
  } else {
    _SERIAL_PRINTLN(F(" Authentication Failed"));
    return request->requestAuthentication();
  }
}

// ====== Editor function : Fetch file

void LittleFSWebFetchFile(AsyncWebServerRequest *request) {
  String path = request->url();
  path.replace(F("/LittleFSWebFetch/"), F("/"));
  request->send(LittleFS, path);  // no type so we can fetch any file.
}

// ====== Editor function : Save the file

size_t LittleFSWebSaveFileSize;
int editorFileSize;
String editorSessionID="aaaaaaa";
String runningSessionID = "bbbbbbbb";
bool editorLastSaveOke = false; // Is true when last save is ok.
void LittleFSWebSaveFile(
  AsyncWebServerRequest *request, String filename, size_t index,
  uint8_t *data, size_t len, bool final) {

  if (LittleFSWebCheckLogon(request, false)) {

    // parameter 'String filename' = "blob" ; filename = 'param' filename
    int params = request->params();
    for (int i = 0; i < params; i++) {
      const AsyncWebParameter *p = request->getParam(i);
      if (p->isPost()) {
        // HTTP POST all other values
        if (p->name() == F("filename")) { filename = p->value().c_str(); }
        if (p->name() == F("sessionID")) { editorSessionID = p->value().c_str(); }
        if (p->name() == F("fileSize")) { editorFileSize = p->value().toInt(); }
      }
    }
    // _SERIAL_PRINTLN("runningSessionID: "+runningSessionID+" editorSessionID: "+editorSessionID);
    if (!LittleFSWebEditFileOpen || (runningSessionID != editorSessionID)) {
      runningSessionID = editorSessionID;
      _SERIAL_PRINTF_P1(PSTR("File Save Start:     %s\n"), filename.c_str());
      _SERIAL_PRINTLN("editorSessionID: "+editorSessionID);
      _SERIAL_PRINTLN("editorFileSize: "+String(editorFileSize));

      if (LittleFSWebEditFileOpen) { LittleFSWebEditFile.close();}

       // editorSessionID will be renamed to filename
      LittleFSWebEditFile = LittleFS.open(editorSessionID+".txt", "w");
      LittleFSWebEditFileOpen = true;
      LittleFSWebSaveFileSize = len;
      editorLastSaveOke = false;
    } else {
      LittleFSWebSaveFileSize += len;
    }

    for (size_t i = 0; i < len; i++) {
      LittleFSWebEditFile.write(data[i]);
      request->_tempFile.write(data[i]);
    }

    if (final) {
      // This is the last part of a chunk
      // Check if this is the last part of the last chunk
      int params = request->params();
      bool LastChunk = false;
      for (int i = 0; i < params; i++) {
        const AsyncWebParameter *p = request->getParam(i);
        if (p->name() == F("filename")) { filename = p->value().c_str(); }
        if (p->name() == F("isLastChunk")) { LastChunk = p->value() == "true"; }
      }

      if (LastChunk) {  // this is the last part of the last chunk
        _SERIAL_PRINTF_P2(
          PSTR("File Save Completed: %s, %u Bytes\n"),
          filename.c_str(), LittleFSWebSaveFileSize);
        LittleFSWebEditFile.close();
        LittleFSWebEditFileOpen = false;
        editorLastSaveOke = (editorFileSize == LittleFSWebSaveFileSize);
        LittleFS.rename(editorSessionID+".txt", filename.c_str());
      }
    }

  } else {
    _SERIAL_PRINTLN(F(" Authentication Failed"));
    return request->requestAuthentication();
  }
}

// ====== Editor function : Last File Save status

void LittleFSWebLastSaveStatus(AsyncWebServerRequest *request) {
  if (editorLastSaveOke) {
    request->send(200, F("text/plain"),F("oke"));
  } else {
    request->send(200, F("text/plain"),F("error"));
  }
}

// ====== Copy file

/*

Copying a file by reading and writing everything in a loop blocks everything.

This is why the copy function copyFile is setup with a ticker where 
the ticker routine only copies a part during each time it is active until
the copy work is finished.

*/

#include <Ticker.h>

// Define a Ticker object
Ticker LittleFSWeb_CopyFileTicker;
int LittleFSWeb_CopyFileTickerInterval = 1;  // a chunk each 1 ms

struct LittleFSWeb_FileCopyState {
  File originalFile;
  File newFile;
  String pathfrom;
  String pathto;
  uint8_t progress;
  long int bytescopied;
};

LittleFSWeb_FileCopyState LittleFSWeb_CopyState;

void LittleFSWeb_CopyFile(const char *pathfrom, const char *pathto) {
  // Initialize the state machine with file paths
  LittleFSWeb_CopyState.pathfrom = String(pathfrom);
  LittleFSWeb_CopyState.pathto = String(pathto);
  // Reset file objects in case they are already open
  LittleFSWeb_CopyState.originalFile = File();
  LittleFSWeb_CopyState.newFile = File();
  // Initialise the counters
  LittleFSWeb_CopyState.progress = 0;
  LittleFSWeb_CopyState.bytescopied = 0;
  // Create 'dummy' parameters, see other copyFile function below
  // Start the copy machine

  LittleFSWeb_CopyFileTicker.attach_ms(
    LittleFSWeb_CopyFileTickerInterval,
    LittleFSWeb_CopyFileStateMachine);
}

void LittleFSWeb_CopyFileStateMachine() {

  // Define a buffer size for 1 chunk
  // The lager the value the lager the impact on other code

  const size_t bufferSize = 32;
  uint8_t buffer[bufferSize];

  // Start by opening the files when not already open

  if ((!LittleFSWeb_CopyState.originalFile)
      && (LittleFSWeb_CopyState.progress == 0)) {
    _SERIAL_PRINTLN(F("Start copy for : ")
                    + String(LittleFSWeb_CopyState.pathfrom)
                    + F(" to ") + String(LittleFSWeb_CopyState.pathto));
    LittleFSWeb_CopyState.progress = 1;
    LittleFSCopyStatus = true;  // copy is running
    // Open the original file
    LittleFSWeb_CopyState.originalFile =
      LittleFS.open(LittleFSWeb_CopyState.pathfrom.c_str(), "r");
    if (!LittleFSWeb_CopyState.originalFile) {
      _SERIAL_PRINTLN(F("- failed to open file for reading ")
                      + String(LittleFSWeb_CopyState.pathfrom));
      return;
    }

    // Open the new file
    LittleFSWeb_CopyState.newFile =
      LittleFS.open(LittleFSWeb_CopyState.pathto.c_str(), "w");
    if (!LittleFSWeb_CopyState.newFile) {
      _SERIAL_PRINTLN(F("- failed to create new file ")
                      + String(LittleFSWeb_CopyState.pathto));
      LittleFSWeb_CopyState.originalFile.close();
      return;
    }
  }

  // Copy data in chunks

  if (LittleFSWeb_CopyState.originalFile.available()) {
    // There is still something to copy
    size_t bytesRead =
      LittleFSWeb_CopyState.originalFile.read(buffer, bufferSize);
    LittleFSWeb_CopyState.newFile.write(buffer, bytesRead);
    LittleFSWeb_CopyState.bytescopied += bytesRead;
    // Do not use the next line in real production.
    // Large files cause a lot of overhead.
    // _SERIAL_PRINTLN(F("Copied bytes: ") + String(bytesRead));
  } else {

    // Everything was read and copied so we finished the job
    if (LittleFSWeb_CopyState.progress == 1) {
      // Close files and reset state
      _SERIAL_PRINTLN(
        F("Copied: ") + String(LittleFSWeb_CopyState.bytescopied)
        + F(" bytes for ") + String(LittleFSWeb_CopyState.pathfrom)
        + F(" to ") + String(LittleFSWeb_CopyState.pathto));
      LittleFSWeb_CopyState.originalFile.close();
      LittleFSWeb_CopyState.newFile.close();
      LittleFSWeb_CopyState.originalFile = File();  // Reset file objects
      LittleFSWeb_CopyState.newFile = File();
      // Stop the Ticker the next time it runs
      // because when I stop it now the files may not be closed
      LittleFSWeb_CopyState.progress = 2;
    } else {
      LittleFSWeb_CopyState.progress = 0;
      LittleFSCopyStatus = false;  // copy is finished
      LittleFSWeb_CopyFileTicker.detach();
    }
  }
}

String LittleFSWebJsonInfo() {
  String json = "";

//  json.concat(F("\" , \"LittleFS_Total\":\""));
//  json.concat(LittleFSWebFileSize(LittleFSWebFsSize('t')));
  json.concat(F("\" , \"LittleFS_Used\":\""));
  json.concat(LittleFSWebFileSize(LittleFSWebFsSize('u')));
  json.concat(F("\" , \"LittleFS_Free\":\""));
  json.concat(LittleFSWebFileSize(LittleFSWebFsSize('f')));

  return json;
}

// ----------------------------------------------------------------------------

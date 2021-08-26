////////////////////////////////////////////////////////////////////
// littleFS.h 
// html help
//
// https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html
// https://www.mischianti.org/2020/06/22/wemos-d1-mini-esp8266-integrated-littlefs-filesystem-part-5/
// http://www.cplusplus.com/reference/
//
// to overwrite time()
// LittleFS.setTimeCallback(getLOCtime);    
//
//
// An ESP32/8266 clock that 
// uses the SETENV variable and in-built time functions
// https://github.com/G6EJD/ESP_Simple_Clock_Functions
// https://github.com/G6EJD/ESP_Simple_Clock_Functions/blob/master/ESP_Simple_Clock.ino
//
////////////////////////////////////////////////////////////////////

#include <FS.h>
#include <LittleFS.h>
#include <time.h>

const String month_name[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
const String day_name[7] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};

//_____________PRINT________________________________

void print_xtime(const char * xdesc, time_t xtime) {
//time_t xnow = time(nullptr);
//struct tm* p_tm = localtime(&xnow);
struct tm * tmstruct = localtime(&xtime);
Serial.printf(" %s : %d-%02d-%02d %02d:%02d:%02d\n", 
                 xdesc,
                 (tmstruct->tm_year) + 1900, 
                 (tmstruct->tm_mon) + 1, 
                 tmstruct->tm_mday, 
                 tmstruct->tm_hour, 
                 tmstruct->tm_min, 
                 tmstruct->tm_sec);
}


void printFSinfo() {
FSInfo info;
    LittleFS.info(info);
    Serial.println("---------------------------------");
    Serial.print("LittleFS.info");
    Serial.print("LittleFS  block size: ");
    Serial.println(info.blockSize);
    Serial.print("LittleFS   page size: ");
    Serial.println(info.pageSize);
    Serial.print("LittleFS total bytes: ");
    Serial.println(info.totalBytes);
    Serial.print("LittleFS  used bytes: ");
    Serial.println(info.usedBytes);
    Serial.print("LittleFS maxOpenFiles: ");
    Serial.println(info.maxOpenFiles);
    Serial.print("LittleFS maxPathLength: ");
    Serial.println(info.maxPathLength);
    Serial.println("---------------------------------");
}

 
//format LittleFS
void formatFS() {
  Serial.println("Formatting LittleFS filesystem");
  LittleFS.format();
} 

bool mountFS() { 
  Serial.println("Mount LittleFS");
  //--if error format  
  if (!LittleFS.begin()) {
   formatFS();  
  }
  //--doneERROR
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return false;
  }
  //--doneOK
  return true;
}



void listDir(const char * dirname) {
  Serial.printf("Listing directory: %s\n", dirname);

  Dir root = LittleFS.openDir(dirname);

  while (root.next()) {
    File file = root.openFile("r");
    Serial.print("  FILE: ");
    Serial.print(root.fileName());
    Serial.print("  SIZE: ");
    Serial.print(file.size());
    time_t cr = file.getCreationTime();
    time_t lw = file.getLastWrite();
    file.close();
    struct tm * tmstruct = localtime(&cr);
    Serial.printf("    CREATION: %d-%02d-%02d %02d:%02d:%02d\n", 
                 (tmstruct->tm_year) + 1900, 
                 (tmstruct->tm_mon) + 1, 
                 tmstruct->tm_mday, 
                 tmstruct->tm_hour, 
                 tmstruct->tm_min, 
                 tmstruct->tm_sec);
    tmstruct = localtime(&lw);
    Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", 
                 (tmstruct->tm_year) + 1900, 
                 (tmstruct->tm_mon) + 1, 
                 tmstruct->tm_mday, 
                 tmstruct->tm_hour, 
                 tmstruct->tm_min, 
                 tmstruct->tm_sec);
  }
}


char * path[10];

 
int getlistDir(const char * dirname) {
  Serial.printf("getFileList: %s\n", dirname);
  Dir root = LittleFS.openDir(dirname);

int i = 0;

  while (root.next()) {
    i++;
    File file = root.openFile("r");
    path[i] = root.fileName; 

    Serial.print("  FILE: ");
    Serial.print(path[i]);
    Serial.print("  SIZE: ");
    Serial.print(file.size());     
  }
 return i;
}




void readFile(const char * path) {
  Serial.printf("Reading file: %s\n", path);

  File file = LittleFS.open(path, "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = LittleFS.open(path, "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  //delay(2000); // Make sure the CREATE and LASTWRITE times are different
  file.close();
}

void appendFile(const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}

void renameFile(const char * path1, const char * path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (LittleFS.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(const char * path) {
  Serial.printf("Deleting file: %s\n", path);
  if (LittleFS.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}


//______________TESTFStimestamp_______________________________

boolean testFStimestamp() {
  //1a---test timestamp [dir, delete, write, read, ...etc]
    printFSinfo();
    listDir("/");  
    deleteFile("/hello1.txt");
  
  //1b----
    //LittleFS.setTimeCallback(getLOCtime);    //deprecated
    writeFile("/hello1.txt", "Hello1 ");
    appendFile("/hello1.txt", "World!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    listDir("/");

  //1bb------------
    Serial.println("---------------------------------");
    Serial.println("The timestamp should be valid above");

    Serial.println("Now unmount and remount and perform the same operation.");
    Serial.println("Timestamp should be valid, data should be good.");
    LittleFS.end();

    Serial.println("---------------------------------");

   //1c----
    Serial.println("Now remount it");
    if (!LittleFS.begin()) {
      Serial.println("LittleFS mount failed");
    return false;
    }
  
  //1d------ 
    listDir("/");
    readFile("/hello1.txt");
    return true;
}


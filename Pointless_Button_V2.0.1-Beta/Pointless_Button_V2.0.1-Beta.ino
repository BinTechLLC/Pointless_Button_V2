/*
      Pointless Button V2 - Version History

      V2.0.0    - Connect Buttons (this build uses 4 buttons and 4 LEDs)
                - Base WiFi and WiFi reset functions
                - Multi SSID Failover functions
                - Basic Firebase Read Functions
                - Basic Count Functions
                - Basic Firebase Write and Update Functions
                - Release and Beta Version Checker

      V2.0.1    - Add LED Success / Fail Indicators for Boot and Updates
                - (Not going to add, have had a lot of people mash multiple / all buttons at a time, would cause an unwanted reboot. ##Add Button Combo to Trigger a Reboot
                - Add Current SSID String and Firebase Entry
                - Move Boot Cycle to its own String and Firebase Entry
                - Add Firebase Read Entry for Serial Debug Output and Serial Count Output
                Added NETBIOS name configuration to pull from pbName string so it no longer shows "esp32-arduino" on the network.
                - *Add Remote LED Test Trigger
                - *Local Web GUI
                - *

      V2.1.0    - *Create Python Server for Time Keeping and Last Boot Recording
                - *

*/
//-----------------------------------------------------------------------------------------------------------------------
//                               Default Node Values
//-----------------------------------------------------------------------------------------------------------------------
String currentLocalVersionNumber = "V2.0.1-Beta";
String currentReleaseVersionNumber;
String currentBetaReleaseVersionNumber;
int writeAsNetPB = false; // Set this to true if creating new node.
String pbName = "PB001-Alpha";
String serialDebugOutput;
String serialCountOutput;
//-----------------------------------------------------------------------------------------------------------------------
//                               Libraries
//-----------------------------------------------------------------------------------------------------------------------
#include <WiFi.h>
#include <FirebaseESP32.h>
//-----------------------------------------------------------------------------------------------------------------------
//                               Credentials and Links
//-----------------------------------------------------------------------------------------------------------------------
/*
#define FIREBASE_HOST "https://YourRTDHostname-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "YourAPIKey"
const char* ssid2     = "SSIDName1";
const char* ssidpass2 = "SSIDPassword1";
const char* ssid1     = "SSIDName2";
const char* ssidpass1 = "SSIDPassword2";
*/
#define FIREBASE_HOST "https://pointlessbuttonv2-2022-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "Bddm9oBrsBKmjzKv9iUtwqO7JjpgJBwb4ObBqOra"
const char* ssid2     = "BinTech LLC";
const char* ssidpass2 = "FuckYouBitch123!@#";
const char* ssid1     = "KB-N20U";
const char* ssidpass1 = "RollYourButt123!@#";
const char* externalHostname = "api.ipify.org";
//-----------------------------------------------------------------------------------------------------------------------
//                               Definitions and States
//-----------------------------------------------------------------------------------------------------------------------
#define greenButton1 12
#define greenButton2 14
#define redButton1 27
#define redButton2 26
#define greenLED1 25
#define greenLED2 33
#define redLED1 32
#define redLED2 13
int greenButton1State;
int greenButton2State;
int redButton1State;
int redButton2State;
int g1Press;
int g2Press;
int r1Press;
int r2Press;
int g1LEDState;
int g2LEDState;
int r1LEDState;
int r2LEDState;
unsigned long g1Count;
unsigned long g2Count;
unsigned long r1Count;
unsigned long r2Count;
unsigned long g1CountRead;
unsigned long g2CountRead;
unsigned long r1CountRead;
unsigned long r2CountRead;
String currentState;
String lastState;
IPAddress lip;
IPAddress eip;
String ep;
int defaultSystemDelayInMilliseconds = 50;
int defaultUpdateDelayInSeconds = 300;
int defaultFirebaseReadDelay = 50;
int defaultFirebaseWriteDelay = 50;
int portNumber = 80;
int wifiResetCycles = 40;  // 20 per SSID.
int connectResetCount;
int connectRunCommand;
int defaultDelayCount;
int defaultDelayCountCycle;
int isRunning;
unsigned long bootCycles;
int writeBootCycles;
int firebaseCycle;
int bootErrorCount;
int updateErrorCount;
String currentSSID;
String rebootNode;
WiFiServer server(portNumber);
FirebaseData fbdo;
//-----------------------------------------------------------------------------------------------------------------------
//                               Void Call Reset Function
//-----------------------------------------------------------------------------------------------------------------------
void(* resetFunc) (void) = 0;  // This Is The Software Reset Function, Just Like A Reboot. It Needs To Be Above All Other Voids To Be Called From Them
//-----------------------------------------------------------------------------------------------------------------------
//                               Void Test LEDs
//-----------------------------------------------------------------------------------------------------------------------
void testLEDs() {
  digitalWrite(greenLED1, HIGH);
  delay(500);
  digitalWrite(greenLED2, HIGH);
  delay(500);
  digitalWrite(redLED1, HIGH);
  delay(500);
  digitalWrite(redLED2, HIGH);
  delay(500);
}
//-----------------------------------------------------------------------------------------------------------------------
//                               String Get External IP
//-----------------------------------------------------------------------------------------------------------------------
String getExternalIP() {
  delay(500);
  WiFiClient client;
  if (client.connect("api.ipify.org", 80))
  {
    client.println("GET / HTTP/1.0");
    client.println("Host: api.ipify.org");
    client.println();
  } else {
    return String();
  }
  delay(5000);
  String line;
  line = "";
  while (client.available())
  {
    line = client.readStringUntil('\n');
  }
  return line;
}
//-----------------------------------------------------------------------------------------------------------------------
//                               Void WiFi First Connect
//-----------------------------------------------------------------------------------------------------------------------
void wifiFirstConnect() {
  g1LEDState = 1;
  connectResetCount = 0;
  //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname(pbName.c_str());
  if (serialDebugOutput == "true") {
    Serial.println(String("NETBIOS Name: ")+String(pbName));
  }
  while (WiFi.status() != WL_CONNECTED) {
    connectResetCount ++ ;
    if (g1LEDState == 1) {
      digitalWrite(greenLED1, LOW);
      g1LEDState = 0;
    }
    else if (g1LEDState == 0) {
      digitalWrite(greenLED1, HIGH);
      g1LEDState = 1;
    }
    if (connectResetCount <= 19) {
      if (connectRunCommand == 0) {
        if (serialDebugOutput == "true") {
          Serial.println(String("Connecting to ") + ssid1);
        }
        WiFi.disconnect(true, true);
        delay(500);
        WiFi.begin(ssid1, ssidpass1);
        currentSSID = ssid1;
        connectRunCommand = 1;
      }
    }
    else if (connectResetCount >= 20 && connectResetCount <= 39) {
      if (connectRunCommand == 1) {
        connectRunCommand = 2;
      }
      if (connectRunCommand == 2) {
        if (serialDebugOutput == "true") {
          Serial.println(String("Connecting to ") + ssid2);
        }
        WiFi.disconnect(true, true);
        delay(500);
        WiFi.begin(ssid2, ssidpass2);
        currentSSID = ssid2;
        connectRunCommand = 3;
      }
    }
    else if (connectResetCount >= 40) {
      if (serialDebugOutput == "true") {
        Serial.println("Resetting");
      }
      delay(1000);
      resetFunc();
    }
    delay(500);
  }
  firebaseCycle = 0;
  digitalWrite(greenLED1, HIGH);
  digitalWrite(greenLED2, LOW);
  WiFi.setSleep(false);
  Serial.println("Getting Internal IP.");
  lip = WiFi.localIP();
  if (serialDebugOutput == "true") {
    Serial.println(lip);
  }
  delay(500);
  Serial.println("Getting External IP.");
  String(eip) = getExternalIP();
  if (serialDebugOutput == "true") {
    Serial.println(eip);
  }
  digitalWrite(greenLED1, LOW);
  digitalWrite(greenLED2, HIGH);
  delay(500);
  Serial.println("Starting Firebase Connection.");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  // Start The Firebase Connection
  Firebase.reconnectWiFi(true);  // Set Firebase To Reconnect If Wireless Fails And Reconnects
  digitalWrite(greenLED1, HIGH);
  digitalWrite(greenLED2, LOW);
  delay(500);
  Serial.println("Reading Firebase Initial Values.");
  //----------------------------------------------------------------------------------------------------------------------- Read Default System Delay In Milliseconds.
  while (firebaseCycle == 0) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/11-Default_System_Delay_In_Milliseconds")) {  // This Will Read The Directory
      defaultSystemDelayInMilliseconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default System Delay In Milliseconds, ");
        Serial.println(defaultSystemDelayInMilliseconds);

      }
      firebaseCycle = 1;
    }
    else {
      Serial.println("Read Failed of Default System Delay In Milliseconds.");
      firebaseCycle = 0;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Update Delay In Seconds.
  while (firebaseCycle == 1) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/12-Default_Update_Delay_In_Seconds")) {  // This Will Read The Directory
      defaultUpdateDelayInSeconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default Update Delay In Seconds.");
        Serial.println(defaultUpdateDelayInSeconds);
      }
      firebaseCycle = 2;
    }
    else {
      Serial.println("Read Failed of Default Update Delay In Seconds.");
      firebaseCycle = 1;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Write Delay.
  while (firebaseCycle == 2) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/13-Default_Firebase_Write_Delay")) {  // This Will Read The Directory
      defaultFirebaseWriteDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default Firebase Write Delay, ");
        Serial.println(defaultFirebaseWriteDelay);
      }
      firebaseCycle = 3;
    }
    else {
      Serial.println("Read Failed of Default Firebase Write Delay.");
      firebaseCycle = 2;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Read Delay.
  while (firebaseCycle == 3) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/14-Default_Firebase_Read_Delay")) {  // This Will Read The Directory
      defaultFirebaseReadDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default Firebase Read Delay, ");
        Serial.println(defaultFirebaseReadDelay);
      }
      firebaseCycle = 4;
    }
    else {
      Serial.println("Read Failed of Default Firebase Read Delay.");
      firebaseCycle = 3;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Current Release Number
  while (firebaseCycle == 4) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getString(fbdo, "/00-Global/00-Current_Release_Number")) {  // This Will Read The Directory
      currentReleaseVersionNumber = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Current Release Version Number, ");
        Serial.println(currentReleaseVersionNumber);
      }
      firebaseCycle = 5;
    }
    else {
      Serial.println("Read Failed of Current Release Version Number.");
      firebaseCycle = 4;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Current Beta Version
  while (firebaseCycle == 5) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getString(fbdo, "/00-Global/01-Current_Beta_Release_Number")) {  // This Will Read The Directory
      currentBetaReleaseVersionNumber = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Current Beta Release Version Number, ");
        Serial.println(currentBetaReleaseVersionNumber);
      }
      firebaseCycle = 6;
    }
    else {
      Serial.println("Read Failed of Current Beta Release Version Number.");
      firebaseCycle = 5;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Check For Update and Beta
  if (currentReleaseVersionNumber == currentLocalVersionNumber) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (serialDebugOutput == "true") {
      Serial.println("# # # # # You are running the most recent version.");
      Serial.println(String("Current Local: ") + currentLocalVersionNumber);
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
    }
  }
  else if (currentReleaseVersionNumber != currentLocalVersionNumber) {
    if (serialDebugOutput == "true") {
      Serial.println("# # # # # Your version does not match the current version.");
      Serial.println(String("Current Local: ") + currentLocalVersionNumber);
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
    }
  }
  if (currentReleaseVersionNumber == currentBetaReleaseVersionNumber) {
    if (serialDebugOutput == "true") {
      Serial.println("# # # # # There are no Beta versions available.");
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
      Serial.println(String("Current Beta: ") + currentBetaReleaseVersionNumber);
    }
  }
  else if (currentReleaseVersionNumber != currentBetaReleaseVersionNumber) {
    if (serialDebugOutput == "true") {
      Serial.println("# # # # # There is a newer Beta version available.");
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
      Serial.println(String("Current Beta: ") + currentBetaReleaseVersionNumber);
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Port Number
  while (firebaseCycle == 6) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/15-Port_Number")) {  // This Will Read The Directory
      portNumber = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Port Number, ");
        Serial.println(portNumber);
      }
      firebaseCycle = 7;
    }
    else {
      Serial.println("Read Failed of Port Number.");
      firebaseCycle = 6;
      bootErrorCount ++;
    }
  }
  //-----------------------------------------------------------------------------------------------------------------------Read WiFi Reset Cycles
  while (firebaseCycle == 7) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/16-WiFi_Reset_Cycles")) {  // This Will Read The Directory
      wifiResetCycles = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of WiFi Reset Cycles, ");
        Serial.println(wifiResetCycles);
      }
      firebaseCycle = 8;
    }
    else {
      Serial.println("Read Failed of WiFi Reset Cycles.");
      firebaseCycle = 7;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Reboot Node
  while (firebaseCycle == 8) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/19-Reboot_Node")) {  // This Will Read The Directory
      rebootNode = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Reboot Node, ");
        Serial.println(rebootNode);
      }
      firebaseCycle = 9;
      if (rebootNode == "true") {
        if (serialDebugOutput == "true") {
          Serial.println("Reboot Triggered Remotely");
        }
        delay(1000);
        resetFunc();
      }
    }
    else {
      Serial.println("Read Failed of Reboot Node.");
      firebaseCycle = 8;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 1
  while (firebaseCycle == 9) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/01-Count1")) {  // This Will Read The "/01-....." Directory
      g1Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Green 1 Count, ");
        Serial.println(g1Count);
      }
      firebaseCycle = 10;
    }
    else {
      Serial.println("Read Failed of Green 1 Count.");
      firebaseCycle = 9;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 2
  while (firebaseCycle == 10) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/02-Count2")) {  // This Will Read The "/02-....." Directory
      g2Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Green 2 Count, ");
        Serial.println(g2Count);
      }
      firebaseCycle = 11;
    }
    else {
      Serial.println("Read Failed of Green 2 Count.");
      firebaseCycle = 10;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 3
  while (firebaseCycle == 11) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/03-Count3")) {  // This Will Read The "/03-....." Directory
      r1Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Red 1 Count, ");
        Serial.println(r1Count);
      }
      firebaseCycle = 12;
    }
    else {
      Serial.println("Read Failed of Red 1 Count.");
      firebaseCycle = 11;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 4
  while (firebaseCycle == 12) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/04-Count4")) {  // This Will Read The "/04-....." Directory
      r2Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Red 2 Count, ");
        Serial.println(r2Count);
      }
      firebaseCycle = 13;
    }
    else {
      Serial.println("Read Failed of Red 2 Count.");
      firebaseCycle = 12;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Boot Cycles
  while (firebaseCycle == 13) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/22-Boot_Count")) {  // This Will Read The "/04-....." Directory
      bootCycles = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Boot Cycles, ");
        Serial.println(bootCycles);
      }
      firebaseCycle = 14;
    }
    else {
      Serial.println("Read Failed of Boot Cycles.");
      writeBootCycles = false;
      firebaseCycle = 13;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Serial Debug Output Setting
  while (firebaseCycle == 14) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/23-Serial_Debug_Output")) {  // This Will Read The "/04-....." Directory
      serialDebugOutput = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Serial Debug Output, ");
        Serial.println(serialDebugOutput);
      }
      firebaseCycle = 15;
    }
    else {
      Serial.println("Read Failed of Serial Debug Output.");
      writeBootCycles = false;
      firebaseCycle = 14;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count Output Setting
  while (firebaseCycle == 15) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/24-Serial_Count_Output")) {  // This Will Read The "/04-....." Directory
      serialCountOutput = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Serial Count Output, ");
        Serial.println(serialCountOutput);
      }
      firebaseCycle = 16;
    }
    else {
      Serial.println("Read Failed of Serial Count Output.");
      writeBootCycles = false;
      firebaseCycle = 15;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- #### WRITE FUNCTIONS ###############################################
  //----------------------------------------------------------------------------------------------------------------------- Write Local Version Number
  while (firebaseCycle == 16) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.setString(fbdo, "/01-Counters/" + pbName + "/20-Current_Local_Version_Number", currentLocalVersionNumber)) {
      delay(defaultFirebaseWriteDelay);  // Defined At The Top
      if (serialDebugOutput) {
        Serial.print("Write Successful of Current Local Version Number, ");
        Serial.println(currentLocalVersionNumber);
      }
      firebaseCycle = 17;
    }
    else {
      Serial.println("Write Failed of Current Local Version Number.");
      firebaseCycle = 16;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Write Locak IP
  while (firebaseCycle == 17) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    String lip2 = WiFi.localIP().toString();
    if (Firebase.setString(fbdo, "/01-Counters/" + pbName + "/17-LocalIP", lip2)) {
      delay(defaultFirebaseWriteDelay);  // Defined At The Top
      if (serialDebugOutput) {
        Serial.print("Write Successful of Local IP, ");
        Serial.println(lip);
      }
      firebaseCycle = 18;
    }
    else {
      Serial.println("Write Failed of Local IP.");
      firebaseCycle = 17;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Write External IP
  while (firebaseCycle == 18) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.setString(fbdo, "/01-Counters/" + pbName + "/18-ExternalIP", eip)) {
      delay(defaultFirebaseWriteDelay);  // Defined At The Top
      if (serialDebugOutput) {
        Serial.print("Write Successful of External IP, ");
        Serial.println(eip);
      }
      firebaseCycle = 19;
    }
    else {
      Serial.println("Write Failed of External IP.");
      firebaseCycle = 18;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Write Boot Cycles
  while (firebaseCycle == 19) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (bootCycles >= 1) {
      delay(5);
      bootCycles + 1;
      delay(5);
      if (Firebase.setString(fbdo, "/01-Counters/" + pbName + "/21-Current_Connected_SSID", currentSSID)) {
        delay(defaultFirebaseWriteDelay);  // Defined At The Top
        if (serialDebugOutput) {
          Serial.print("Write Successful of Current SSID: ");
          Serial.println(currentSSID);
        }
        firebaseCycle = 20;
      }
      else {
        Serial.println("Write Failed of Current SSID.");
        firebaseCycle = 19;
        bootErrorCount ++;
      }
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Write Boot Cycles
  while (firebaseCycle == 20) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (bootCycles >= 1) {
      delay(5);
      bootCycles ++;
      delay(5);
      if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/22-Boot_Count", bootCycles)) {
        delay(defaultFirebaseWriteDelay);  // Defined At The Top
        if (serialDebugOutput) {
          Serial.print("Write Successful of Boot Cycles.");
          Serial.println(bootCycles);
        }
        firebaseCycle = 21;
      }
      else {
        Serial.println("Write Failed of Boot Cycles.");
        firebaseCycle = 20;
        bootErrorCount ++;
      }
    }
  }
  digitalWrite(greenLED1, HIGH);
  digitalWrite(greenLED2, HIGH);
  delay(500);
  delay(1000);
  if (bootErrorCount <= 0) {
    if (serialDebugOutput) {
      Serial.println("All Firebase First Boot Variables Sent / Received Successfully");
    }
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, HIGH);
    digitalWrite(redLED1, LOW);
    digitalWrite(redLED2, LOW);
  }
  else if (bootErrorCount >= 1) {
    if (serialDebugOutput) {
      Serial.println(String("All Firebase First Boot Variables Sent / Received Successfully with retry errors: ") + String(bootErrorCount));
    }
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, HIGH);
    digitalWrite(redLED1, HIGH);
    digitalWrite(redLED2, LOW);
  }
  delay(500);
  digitalWrite(greenLED1, LOW);
  digitalWrite(greenLED2, LOW);
  digitalWrite(redLED1, LOW);
  digitalWrite(redLED2, LOW);
}
//-----------------------------------------------------------------------------------------------------------------------
//                               Void Write Firebase Updates
//-----------------------------------------------------------------------------------------------------------------------
void writeFirebaseUpdate() {
  updateErrorCount = 0;
  if (serialDebugOutput == "true") {
    Serial.println(" # # # # # Firebase Update Triggered");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default System Delay In Milliseconds.
  while (firebaseCycle == 0) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/00-PB_Name")) {  // This Will Read The Directory
      String pbNameRead = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of PB Name, ");
        Serial.println(pbNameRead);
      }
      firebaseCycle = 1;
    }
    else {
      Serial.println("Read Failed of PB Name.");
      firebaseCycle = 0;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default System Delay In Milliseconds.
  while (firebaseCycle == 1) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/11-Default_System_Delay_In_Milliseconds")) {  // This Will Read The Directory
      defaultSystemDelayInMilliseconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default System Delay In Milliseconds, ");
        Serial.println(defaultSystemDelayInMilliseconds);
      }
      firebaseCycle = 2;
    }
    else {
      Serial.println("Read Failed of Default System Delay In Milliseconds.");
      firebaseCycle = 1;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Update Delay In Seconds.
  while (firebaseCycle == 2) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/12-Default_Update_Delay_In_Seconds")) {  // This Will Read The Directory
      defaultUpdateDelayInSeconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default Update Delay In Seconds, ");
        Serial.println(defaultUpdateDelayInSeconds);
      }
      firebaseCycle = 3;
    }
    else {
      Serial.println("Read Failed of Default Update Delay In Seconds.");
      firebaseCycle = 2;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Write Delay.
  while (firebaseCycle == 3) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/13-Default_Firebase_Write_Delay")) {  // This Will Read The Directory
      defaultFirebaseWriteDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default Firebase Write Delay, ");
        Serial.println(defaultFirebaseWriteDelay);
      }
      firebaseCycle = 4;
    }
    else {
      Serial.println("Read Failed of Default Firebase Write Delay.");
      firebaseCycle = 3;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Read Delay.
  while (firebaseCycle == 4) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/14-Default_Firebase_Read_Delay")) {  // This Will Read The Directory
      defaultFirebaseReadDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Default Firebase Read Delay, ");
        Serial.println(defaultFirebaseReadDelay);
      }
      firebaseCycle = 5;
    }
    else {
      Serial.println("Read Failed of Default Firebase Read Delay.");
      firebaseCycle = 4;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Port Number
  while (firebaseCycle == 5) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/15-Port_Number")) {  // This Will Read The Directory
      portNumber = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Port Number, ");
        Serial.println(portNumber);
      }
      firebaseCycle = 6;
    }
    else {
      Serial.println("Read Failed of Port Number.");
      firebaseCycle = 5;
      updateErrorCount ++;
    }
  }
  //-----------------------------------------------------------------------------------------------------------------------Read WiFi Reset Cycles
  while (firebaseCycle == 6) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/16-WiFi_Reset_Cycles")) {  // This Will Read The Directory
      wifiResetCycles = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of WiFi Reset Cycles, ");
        Serial.println(wifiResetCycles);
      }
      firebaseCycle = 7;
    }
    else {
      Serial.println("Read Failed of WiFi Reset Cycles.");
      firebaseCycle = 6;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Reboot Node
  while (firebaseCycle == 7) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/19-Reboot_Node")) {  // This Will Read The Directory
      rebootNode = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Reboot Node, ");
        Serial.println(rebootNode);
      }
      firebaseCycle = 8;
      if (rebootNode == "true") {
        if (serialDebugOutput == "true") {
          Serial.println("Reboot Triggered Remotely");
        }
        delay(1000);
        resetFunc();
      }
    }
    else {
      Serial.println("Read Failed of Reboot Node.");
      firebaseCycle = 7;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Serial Debug Output Setting
  while (firebaseCycle == 8) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/23-Serial_Debug_Output")) {  // This Will Read The "/04-....." Directory
      serialDebugOutput = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Serial Debug Output, ");
        Serial.println(serialDebugOutput);
      }
      firebaseCycle = 9;
    }
    else {
      Serial.println("Read Failed of Serial Debug Output.");
      writeBootCycles = false;
      firebaseCycle = 8;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count Output Setting
  while (firebaseCycle == 9) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/24-Serial_Count_Output")) {  // This Will Read The "/04-....." Directory
      serialCountOutput = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Serial Count Output, ");
        Serial.println(serialCountOutput);
      }
      firebaseCycle = 10;
    }
    else {
      Serial.println("Read Failed of Serial Count Output.");
      writeBootCycles = false;
      firebaseCycle = 9;
      bootErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 1
  while (firebaseCycle == 10) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/01-Count1")) {  // This Will Read The "/01-....." Directory
      g1CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Green 1 Count, ");
        Serial.println(g1Count);
      }
      firebaseCycle = 11;
    }
    else {
      Serial.println("Read Failed of Green 1 Count.");
      firebaseCycle = 10;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 2
  while (firebaseCycle == 11) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/02-Count2")) {  // This Will Read The "/02-....." Directory
      g2CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Green 2 Count, ");
        Serial.println(g2Count);
      }
      firebaseCycle = 12;
    }
    else {
      Serial.println("Read Failed of Green 2 Count.");
      firebaseCycle = 11;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 3
  while (firebaseCycle == 12) {
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, LOW);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/03-Count3")) {  // This Will Read The "/03-....." Directory
      r1CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Red 1 Count, ");
        Serial.println(r1Count);
      }
      firebaseCycle = 13;
    }
    else {
      Serial.println("Read Failed of Red 1 Count.");
      firebaseCycle = 12;
      updateErrorCount ++;
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 4
  while (firebaseCycle == 13) {
    digitalWrite(greenLED1, LOW);
    digitalWrite(greenLED2, HIGH);
    if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/04-Count4")) {  // This Will Read The "/04-....." Directory
      r2CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(defaultFirebaseReadDelay);
      if (serialDebugOutput == "true") {
        Serial.print("Read Successful of Red 2 Count, ");
        Serial.println(r2Count);
      }
      firebaseCycle = 14;
    }
    else {
      Serial.println("Read Failed of Red 2 Count.");
      firebaseCycle = 13;
      updateErrorCount ++;
    }
  }
  if (g1Count >= (g1CountRead + 1) or g2Count >= (g2CountRead + 1) or r1Count >= (r1CountRead + 1) or r2Count >= (r2CountRead + 1)) {
    //----------------------------------------------------------------------------------------------------------------------- Write Count 1
    while (firebaseCycle == 14) {
      digitalWrite(greenLED1, HIGH);
      digitalWrite(greenLED2, LOW);
      if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/01-Count1", g1Count)) {
        delay(defaultFirebaseWriteDelay);  // Defined At The Top
        if (serialDebugOutput) {
          Serial.print("Write Successful of New Green 1 Count, ");
          Serial.println(g1Count);
        }
        firebaseCycle = 15;
      }
      else {
        Serial.println("Write Failed of New Green 1 Count.");
        firebaseCycle = 14;
        updateErrorCount ++;
      }
    }
    //----------------------------------------------------------------------------------------------------------------------- Write Count 2
    while (firebaseCycle == 15) {
      digitalWrite(greenLED1, LOW);
      digitalWrite(greenLED2, HIGH);
      if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/02-Count2", g2Count)) {
        delay(defaultFirebaseWriteDelay);  // Defined At The Top
        if (serialDebugOutput) {
          Serial.print("Write Successful of New Green 2 Count, ");
          Serial.println(g2Count);
        }
        firebaseCycle = 16;
      }
      else {
        Serial.println("Write Failed of New Green 2 Count.");
        firebaseCycle = 15;
        updateErrorCount ++;
      }
    }
    //----------------------------------------------------------------------------------------------------------------------- Write Count 3
    while (firebaseCycle == 16) {
      digitalWrite(greenLED1, HIGH);
      digitalWrite(greenLED2, LOW);
      if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/03-Count3", r1Count)) {
        delay(defaultFirebaseWriteDelay);  // Defined At The Top
        if (serialDebugOutput) {
          Serial.print("Write Successful of New Red 1 Count, ");
          Serial.println(r1Count);
        }
        firebaseCycle = 17;
      }
      else {
        Serial.println("Write Failed of New Red 1 Count.");
        firebaseCycle = 16;
        updateErrorCount ++;
      }
    }
    //----------------------------------------------------------------------------------------------------------------------- Write Count 4
    while (firebaseCycle == 17) {
      digitalWrite(greenLED1, LOW);
      digitalWrite(greenLED2, HIGH);
      if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/04-Count4", r2Count)) {
        delay(defaultFirebaseWriteDelay);  // Defined At The Top
        if (serialDebugOutput) {
          Serial.print("Write Successful of New Red 2 Count, ");
          Serial.println(r2Count);
        }
        firebaseCycle = 18;
      }
      else {
        Serial.println("Write Failed of New Red 2 Count.");
        firebaseCycle = 17;
        updateErrorCount ++;
      }
    }
  }
  if (updateErrorCount <= 0) {
    if (serialDebugOutput) {
      Serial.println(" # # # # # Finished Successfully");
    }
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, HIGH);
    digitalWrite(redLED1, LOW);
    digitalWrite(redLED2, LOW);
  }
  if (updateErrorCount >= 1) {
    if (serialDebugOutput) {
      Serial.println(String(" # # # # # Finished with retry errors: ") + String(updateErrorCount));
    }
    digitalWrite(greenLED1, HIGH);
    digitalWrite(greenLED2, HIGH);
    digitalWrite(redLED1, LOW);
    digitalWrite(redLED2, HIGH);
  }
  delay(500);
  digitalWrite(greenLED1, LOW);
  digitalWrite(greenLED2, LOW);
  digitalWrite(redLED1, LOW);
  digitalWrite(redLED2, LOW);
}
//-----------------------------------------------------------------------------------------------------------------------
//                               Void Run Button Logic
//-----------------------------------------------------------------------------------------------------------------------
void runButtonLogic() {
  //--------------------------------- Green 1
  if (greenButton1State == 0) {
    if (g1Press == 0) {
      digitalWrite(greenLED1, HIGH);
      g1Count ++;
      g1Press = 1;
    }
    else if (g1Press == 1) {
      g1Press = 1;
    }
  }
  else if (greenButton1State == 1) {
    digitalWrite(greenLED1, LOW);
    g1Press = 0;
  }
  //--------------------------------- Green 2
  if (greenButton2State == 0) {
    if (g2Press == 0) {
      digitalWrite(greenLED2, HIGH);
      g2Count ++;
      g2Press = 1;
    }
    else if (g2Press == 1) {
      g2Press = 1;
    }
  }
  else if (greenButton2State == 1) {
    digitalWrite(greenLED2, LOW);
    g2Press = 0;
  }
  //--------------------------------- Red 1
  if (redButton1State == 0) {
    if (r1Press == 0) {
      digitalWrite(redLED1, HIGH);
      r1Count ++;
      r1Press = 1;
    }
    else if (r1Press == 1) {
      r1Press = 1;
    }
  }
  else if (redButton1State == 1) {
    digitalWrite(redLED1, LOW);
    r1Press = 0;
  }
  //--------------------------------- Red 2
  if (redButton2State == 0) {
    if (r2Press == 0) {
      digitalWrite(redLED2, HIGH);
      r2Count ++;
      r2Press = 1;
    }
    else if (r2Press == 1) {
      r2Press = 1;
    }
  }
  else if (redButton2State == 1) {
    digitalWrite(redLED2, LOW);
    r2Press = 0;
  }
}
//-----------------------------------------------------------------------------------------------------------------------
//                               Void Setup
//-----------------------------------------------------------------------------------------------------------------------
void setup() {
  serialCountOutput = "true";
  serialDebugOutput = "true";
  firebaseCycle = 0;
  if (serialDebugOutput == "true") {
    Serial.begin(115200);
    if (!Serial) {
      serialDebugOutput = "false";
    }
    if (serialDebugOutput == "true") {
      Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
      Serial.println(" - Starting - " + pbName + "      New Node Enabled?    " + writeAsNetPB);
    }
  }
  pinMode(greenButton1, INPUT_PULLUP);
  pinMode(greenButton2, INPUT_PULLUP);
  pinMode(redButton1, INPUT_PULLUP);
  pinMode(redButton2, INPUT_PULLUP);
  pinMode(greenLED1, OUTPUT);
  pinMode(greenLED2, OUTPUT);
  pinMode(redLED1, OUTPUT);
  pinMode(redLED2, OUTPUT);
  if (serialDebugOutput == "true") {
    Serial.println("Testing LEDs");
  }
  digitalWrite(greenLED1, LOW);
  digitalWrite(greenLED2, LOW);
  digitalWrite(redLED1, LOW);
  digitalWrite(redLED2, LOW);
  WiFi.disconnect();
  wifiFirstConnect();
  testLEDs();
}
//-----------------------------------------------------------------------------------------------------------------------
//                               Void Loop
//-----------------------------------------------------------------------------------------------------------------------
void loop() {
  writeBootCycles = 0;
  if (WiFi.status() != 3) {
    WiFi.disconnect();
    delay(1000);
    wifiFirstConnect();
  }
  defaultDelayCountCycle ++;
  if (defaultDelayCountCycle >= 20) {
    defaultDelayCount ++;
    if (serialDebugOutput == "true") {
      if (serialCountOutput == "true") {
        Serial.println(defaultDelayCount);
      }
    }
    defaultDelayCountCycle = 0;
  }
  if (defaultDelayCount >= defaultUpdateDelayInSeconds) {

    defaultDelayCount = 0;
    defaultDelayCountCycle = 0;
    firebaseCycle = 0;
    writeFirebaseUpdate();
  }
  greenButton1State = digitalRead(greenButton1);
  greenButton2State = digitalRead(greenButton2);
  redButton1State = digitalRead(redButton1);
  redButton2State = digitalRead(redButton2);
  currentState = (String("G1: ") + g1Count + String("    G2: ") + g2Count + String("    R1: ") + r1Count + String("    R2: ") + r2Count);
  runButtonLogic();
  //--------------------------------- Print Debug To Serial
  if (serialDebugOutput == "true") {
    if (lastState != currentState) {
      lastState = currentState;
      Serial.println(currentState);
    }
  }
  delay(defaultSystemDelayInMilliseconds);
}

/*
      Pointless Button V2 - Version History

      V2.0.0    - Connect Buttons (this build uses 4 buttons and 4 LEDs)
                - Base WiFi and WiFi reset functions
                - Multi SSID Failover functions
                - Basic Firebase Read Functions
                - Basic Count Functions
                - Basic Firebase Write and Update Functions
                - Release and Beta Version Checker
      V2.0.1    - *Local Web GUI
                
*/
//-----------------------------------------------------------------------------------------------------------------------
//                               Default Node Values
//-----------------------------------------------------------------------------------------------------------------------
String currentLocalVersionNumber = "V2.0.0";
String currentReleaseVersionNumber;
String currentBetaReleaseVersionNumber;
int writeAsNetPB = false; // Set this to true if creating new node.
String pbName = "PB001-Alpha";
int serialDebugOutput = true;
//-----------------------------------------------------------------------------------------------------------------------
//                               Libraries
//-----------------------------------------------------------------------------------------------------------------------
#include <WiFi.h>
#include <FirebaseESP32.h>
//-----------------------------------------------------------------------------------------------------------------------
//                               Credentials and Links
//-----------------------------------------------------------------------------------------------------------------------
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
  digitalWrite(greenLED1, HIGH);
  g1LEDState = 1;
  connectResetCount = 0;
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
        if (serialDebugOutput == true) {
          Serial.println(String("Connecting to ") + ssid1);
        }
        WiFi.disconnect();
        delay(500);
        WiFi.begin(ssid1, ssidpass1);
        connectRunCommand = 1;
      }
    }
    else if (connectResetCount >= 20 && connectResetCount <= 39) {
      if (connectRunCommand == 1) {
        connectRunCommand = 2;
      }
      if (connectRunCommand == 2) {
        if (serialDebugOutput == true) {
          Serial.println(String("Connecting to ") + ssid2);
        }
        WiFi.disconnect();
        delay(500);
        WiFi.begin(ssid2, ssidpass2);
        connectRunCommand = 3;
      }
    }
    else if (connectResetCount >= 40) {
      if (serialDebugOutput == true) {
        Serial.println("Resetting");
      }
      delay(1000);
      resetFunc();
    }
    delay(500);
  }
  digitalWrite(greenLED1, HIGH);
  WiFi.setSleep(false);
  Serial.println("Getting Internal IP.");
  lip = WiFi.localIP();
  if (serialDebugOutput == true) {
    Serial.println(lip);
  }
  delay(500);
  Serial.println("Getting External IP.");
  String(eip) = getExternalIP();
  if (serialDebugOutput == true) {
    Serial.println(eip);
  }
  digitalWrite(greenLED2, HIGH);
  delay(500);
  Serial.println("Starting Firebase Connection.");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  // Start The Firebase Connection
  Firebase.reconnectWiFi(true);  // Set Firebase To Reconnect If Wireless Fails And Reconnects
  digitalWrite(redLED1, HIGH);
  delay(500);
  Serial.println("Reading Firebase Initial Values.");
  //----------------------------------------------------------------------------------------------------------------------- Read Default System Delay In Milliseconds.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/11-Default_System_Delay_In_Milliseconds")) {  // This Will Read The Directory
    defaultSystemDelayInMilliseconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default System Delay In Milliseconds, ");
      Serial.println(defaultSystemDelayInMilliseconds);
    }
  }
  else {
    Serial.println("Read Failed of Default System Delay In Milliseconds.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Update Delay In Seconds.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/12-Default_Update_Delay_In_Seconds")) {  // This Will Read The Directory
    defaultUpdateDelayInSeconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default Update Delay In Seconds.");
      Serial.println(defaultUpdateDelayInSeconds);
    }
  }
  else {
    Serial.println("Read Failed of Default Update Delay In Seconds.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Write Delay.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/13-Default_Firebase_Write_Delay")) {  // This Will Read The Directory
    defaultFirebaseWriteDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default Firebase Write Delay, ");
      Serial.println(defaultFirebaseWriteDelay);
    }
  }
  else {
    Serial.println("Read Failed of Default Firebase Write Delay.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Read Delay.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/14-Default_Firebase_Read_Delay")) {  // This Will Read The Directory
    defaultFirebaseReadDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default Firebase Read Delay, ");
      Serial.println(defaultFirebaseReadDelay);
    }
  }
  else {
    Serial.println("Read Failed of Default Firebase Read Delay.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Current Release Number
  if (Firebase.getString(fbdo, "/00-Global/00-Current_Release_Number")) {  // This Will Read The Directory
    currentReleaseVersionNumber = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Current Release Version Number, ");
      Serial.println(currentReleaseVersionNumber);
    }
  }
  else {
    Serial.println("Read Failed of Current Release Version Number.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Current Beta Version
  if (Firebase.getString(fbdo, "/00-Global/01-Current_Beta_Release_Number")) {  // This Will Read The Directory
    currentBetaReleaseVersionNumber = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Current Beta Release Version Number, ");
      Serial.println(currentBetaReleaseVersionNumber);
    }
  }
  else {
    Serial.println("Read Failed of Current Beta Release Version Number.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Check For Update and Beta
  if (currentReleaseVersionNumber == currentLocalVersionNumber) {
    if (serialDebugOutput == true) {
      Serial.println("# # # # # You are running the most recent version.");
      Serial.println(String("Current Local: ") + currentLocalVersionNumber);
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
    }
  }
  else if (currentReleaseVersionNumber != currentLocalVersionNumber) {
    if (serialDebugOutput == true) {
      Serial.println("# # # # # Your version does not match the current version.");
      Serial.println(String("Current Local: ") + currentLocalVersionNumber);
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
    }
  }
  if (currentReleaseVersionNumber == currentBetaReleaseVersionNumber) {
    if (serialDebugOutput == true) {
      Serial.println("# # # # # There are no Beta versions available.");
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
      Serial.println(String("Current Beta: ") + currentBetaReleaseVersionNumber);
    }
  }
  else if (currentReleaseVersionNumber != currentBetaReleaseVersionNumber) {
    if (serialDebugOutput == true) {
      Serial.println("# # # # # There is a newer Beta version available.");
      Serial.println(String("Current Release: ") + currentReleaseVersionNumber);
      Serial.println(String("Current Beta: ") + currentBetaReleaseVersionNumber);
    }
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Port Number
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/15-Port_Number")) {  // This Will Read The Directory
    portNumber = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Port Number, ");
      Serial.println(portNumber);
    }
  }
  else {
    Serial.println("Read Failed of Port Number.");
  }
  //-----------------------------------------------------------------------------------------------------------------------Read WiFi Reset Cycles
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/16-WiFi_Reset_Cycles")) {  // This Will Read The Directory
    wifiResetCycles = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of WiFi Reset Cycles, ");
      Serial.println(wifiResetCycles);
    }
  }
  else {
    Serial.println("Read Failed of WiFi Reset Cycles.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Reboot Node
  if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/19-Reboot_Node")) {  // This Will Read The Directory
    rebootNode = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Reboot Node, ");
      Serial.println(rebootNode);
    }
    if (rebootNode == "true") {
      if (serialDebugOutput == true) {
        Serial.println("Reboot Triggered Remotely");
      }
      delay(1000);
      resetFunc();
    }
  }
  else {
    Serial.println("Read Failed of Reboot Node.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 1
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/01-Count1")) {  // This Will Read The "/01-....." Directory
    g1Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Green 1 Count, ");
      Serial.println(g1Count);
    }
  }
  else {
    Serial.println("Read Failed of Green 1 Count.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 2
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/02-Count2")) {  // This Will Read The "/02-....." Directory
    g2Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Green 2 Count, ");
      Serial.println(g2Count);
    }
  }
  else {
    Serial.println("Read Failed of Green 2 Count.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 3
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/03-Count3")) {  // This Will Read The "/03-....." Directory
    r1Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Red 1 Count, ");
      Serial.println(r1Count);
    }
  }
  else {
    Serial.println("Read Failed of Red 1 Count.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 4
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/04-Count4")) {  // This Will Read The "/04-....." Directory
    r2Count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Red 2 Count, ");
      Serial.println(r2Count);
    }
  }
  else {
    Serial.println("Read Failed of Red 2 Count.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 10 (Boot Cycles)
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/10-Count10")) {  // This Will Read The "/04-....." Directory
    bootCycles = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Boot Cycles, ");
      Serial.println(bootCycles);
      if (isRunning == 0) {
        writeBootCycles = true;
        isRunning = 1;
      }
      else {
        writeBootCycles = false;
      }
    }
  }
  else {
    Serial.println("Read Failed of Boot Cycles.");
    writeBootCycles = false;
  }
  //----------------------------------------------------------------------------------------------------------------------- #### WRITE FUNCTIONS ###############################################
  //----------------------------------------------------------------------------------------------------------------------- Write Local Version Number
  if (Firebase.setString(fbdo, "/01-Counters/" + pbName + "/20-Current_Local_Version_Number", currentLocalVersionNumber)) {
    delay(defaultFirebaseWriteDelay);  // Defined At The Top
    if (serialDebugOutput) {
      Serial.print("Write Successful of Current Local Version Number, ");
      Serial.println(currentLocalVersionNumber);
    }
  }
  else {
    Serial.println("Write Failed of Current Local Version Number.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Write Locak IP
  String lip2 = WiFi.localIP().toString();
  if (Firebase.setString(fbdo, "/01-Counters/" + pbName + "/17-LocalIP", lip2)) {
    delay(defaultFirebaseWriteDelay);  // Defined At The Top
    if (serialDebugOutput) {
      Serial.print("Write Successful of Local IP, ");
      Serial.println(lip);
    }
  }
  else {
    Serial.println("Write Failed of Local IP.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Write External IP
  if (Firebase.setString(fbdo, "/01-Counters/" + pbName + "/18-ExternalIP", eip)) {
    delay(defaultFirebaseWriteDelay);  // Defined At The Top
    if (serialDebugOutput) {
      Serial.print("Write Successful of External IP, ");
      Serial.println(eip);
    }
  }
  else {
    Serial.println("Write Failed of External IP.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Write Boot Cycles
  if (writeBootCycles == true) {
    if (bootCycles >= 1) {
      delay(5);
      bootCycles ++;
      delay(5);
      if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/10-Count10", bootCycles)) {
        delay(defaultFirebaseWriteDelay);  // Defined At The Top
        if (serialDebugOutput) {
          Serial.print("Write Successful of Boot Cycles (Count10), ");
          Serial.println(bootCycles);
        }
      }
      else {
        Serial.println("Write Failed of Boot Cycles (Count10).");
      }
    }
  }
  digitalWrite(redLED2, HIGH);
  delay(500);
  delay(1000);
  digitalWrite(greenLED1, LOW);
  digitalWrite(greenLED2, LOW);
  digitalWrite(redLED1, LOW);
  digitalWrite(redLED2, LOW);
}
//-----------------------------------------------------------------------------------------------------------------------
//                               Void Write Firebase Updates
//-----------------------------------------------------------------------------------------------------------------------
void writeFirebaseUpdate() {
  //----------------------------------------------------------------------------------------------------------------------- Read Default System Delay In Milliseconds.
  if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/00-PB_Name")) {  // This Will Read The Directory
    String pbNameRead = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of PB Name, ");
      Serial.println(pbNameRead);
    }
  }
  else {
    Serial.println("Read Failed of PB Name.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default System Delay In Milliseconds.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/11-Default_System_Delay_In_Milliseconds")) {  // This Will Read The Directory
    defaultSystemDelayInMilliseconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default System Delay In Milliseconds, ");
      Serial.println(defaultSystemDelayInMilliseconds);
    }
  }
  else {
    Serial.println("Read Failed of Default System Delay In Milliseconds.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Update Delay In Seconds.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/12-Default_Update_Delay_In_Seconds")) {  // This Will Read The Directory
    defaultUpdateDelayInSeconds = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default Update Delay In Seconds, ");
      Serial.println(defaultUpdateDelayInSeconds);
    }
  }
  else {
    Serial.println("Read Failed of Default Update Delay In Seconds.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Write Delay.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/13-Default_Firebase_Write_Delay")) {  // This Will Read The Directory
    defaultFirebaseWriteDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default Firebase Write Delay, ");
      Serial.println(defaultFirebaseWriteDelay);
    }
  }
  else {
    Serial.println("Read Failed of Default Firebase Write Delay.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Default Firebase Read Delay.
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/14-Default_Firebase_Read_Delay")) {  // This Will Read The Directory
    defaultFirebaseReadDelay = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Default Firebase Read Delay, ");
      Serial.println(defaultFirebaseReadDelay);
    }
  }
  else {
    Serial.println("Read Failed of Default Firebase Read Delay.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Port Number
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/15-Port_Number")) {  // This Will Read The Directory
    portNumber = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Port Number, ");
      Serial.println(portNumber);
    }
  }
  else {
    Serial.println("Read Failed of Port Number.");
  }
  //-----------------------------------------------------------------------------------------------------------------------Read WiFi Reset Cycles
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/16-WiFi_Reset_Cycles")) {  // This Will Read The Directory
    wifiResetCycles = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of WiFi Reset Cycles, ");
      Serial.println(wifiResetCycles);
    }
  }
  else {
    Serial.println("Read Failed of WiFi Reset Cycles.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Reboot Node
  if (Firebase.getString(fbdo, "/01-Counters/" + pbName + "/19-Reboot_Node")) {  // This Will Read The Directory
    rebootNode = fbdo.stringData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Reboot Node, ");
      Serial.println(rebootNode);
    }
    if (rebootNode == "true") {
      if (serialDebugOutput == true) {
        Serial.println("Reboot Triggered Remotely");
      }
      delay(1000);
      resetFunc();
    }
  }
  else {
    Serial.println("Read Failed of Reboot Node.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 1
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/01-Count1")) {  // This Will Read The "/01-....." Directory
    g1CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Green 1 Count, ");
      Serial.println(g1Count);
    }
  }
  else {
    Serial.println("Read Failed of Green 1 Count.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 2
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/02-Count2")) {  // This Will Read The "/02-....." Directory
    g2CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Green 2 Count, ");
      Serial.println(g2Count);
    }
  }
  else {
    Serial.println("Read Failed of Green 2 Count.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 3
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/03-Count3")) {  // This Will Read The "/03-....." Directory
    r1CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Red 1 Count, ");
      Serial.println(r1Count);
    }
  }
  else {
    Serial.println("Read Failed of Red 1 Count.");
  }
  //----------------------------------------------------------------------------------------------------------------------- Read Count 4
  if (Firebase.getInt(fbdo, "/01-Counters/" + pbName + "/04-Count4")) {  // This Will Read The "/04-....." Directory
    r2CountRead = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
    delay(defaultFirebaseReadDelay);
    if (serialDebugOutput == true) {
      Serial.print("Read Successful of Red 2 Count, ");
      Serial.println(r2Count);
    }
  }
  else {
    Serial.println("Read Failed of Red 2 Count.");
  }
  if (g1Count >= (g1CountRead + 1) or g2Count >= (g2CountRead + 1) or r1Count >= (r1CountRead + 1) or r2Count >= (r2CountRead + 1)) {
    //----------------------------------------------------------------------------------------------------------------------- Write Count 1
    if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/01-Count1", g1Count)) {
      delay(defaultFirebaseWriteDelay);  // Defined At The Top
      if (serialDebugOutput) {
        Serial.print("Write Successful of New Green 1 Count, ");
        Serial.println(g1Count);
      }
    }
    else {
      Serial.println("Write Failed of New Green 1 Count.");
    }
    //----------------------------------------------------------------------------------------------------------------------- Write Count 2
    if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/02-Count2", g2Count)) {
      delay(defaultFirebaseWriteDelay);  // Defined At The Top
      if (serialDebugOutput) {
        Serial.print("Write Successful of New Green 2 Count, ");
        Serial.println(g2Count);
      }
    }
    else {
      Serial.println("Write Failed of New Green 2 Count.");
    }
    //----------------------------------------------------------------------------------------------------------------------- Write Count 3
    if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/03-Count3", r1Count)) {
      delay(defaultFirebaseWriteDelay);  // Defined At The Top
      if (serialDebugOutput) {
        Serial.print("Write Successful of New Red 1 Count, ");
        Serial.println(r1Count);
      }
    }
    else {
      Serial.println("Write Failed of New Red 1 Count.");
    }
    //----------------------------------------------------------------------------------------------------------------------- Write Count 4
    if (Firebase.setInt(fbdo, "/01-Counters/" + pbName + "/04-Count4", r2Count)) {
      delay(defaultFirebaseWriteDelay);  // Defined At The Top
      if (serialDebugOutput) {
        Serial.print("Write Successful of New Red 2 Count, ");
        Serial.println(r2Count);
      }
    }
    else {
      Serial.println("Write Failed of New Red 2 Count.");
    }
  }
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
  if (serialDebugOutput == true) {
    Serial.begin(115200);
    if (!Serial) {
      serialDebugOutput = false;
    }
    if (serialDebugOutput == true) {
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
  if (serialDebugOutput == true) {
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
    if (serialDebugOutput == true) {
      Serial.println(defaultDelayCount);
    }
    defaultDelayCountCycle = 0;
  }
  if (defaultDelayCount >= defaultUpdateDelayInSeconds) {
    if (serialDebugOutput == true) {
      Serial.println(" # # # # # Firebase Update Triggered");
    }
    defaultDelayCount = 0;
    defaultDelayCountCycle = 0;
    writeFirebaseUpdate();
  }
  greenButton1State = digitalRead(greenButton1);
  greenButton2State = digitalRead(greenButton2);
  redButton1State = digitalRead(redButton1);
  redButton2State = digitalRead(redButton2);
  currentState = (String("G1: ") + g1Count + String("    G2: ") + g2Count + String("    R1: ") + r1Count + String("    R2: ") + r2Count);
  runButtonLogic();
  //--------------------------------- Print Debug To Serial
  if (serialDebugOutput == true) {
    if (lastState != currentState) {
      lastState = currentState;
      Serial.println(currentState);
    }
  }
  delay(defaultSystemDelayInMilliseconds);
}

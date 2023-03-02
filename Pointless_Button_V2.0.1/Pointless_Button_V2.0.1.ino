/*  Version List

    Version 1.0     - Base WiFi Connection
    Version 1.1     - WiFi Reconnect Function
    Version 1.2     - Add Button And LED Function
    Version 1.3     - Firebase Read Integration
    Version 1.4     - Firebase Write Integration
    Version 1.5     - Fix Web Server And Add 'pbNumberj Integration
    Version 1.6     - Fix Fail To Connect On First Boot Issue
    Version 1.7     - Fixed IP Address Write String Issues
                    - Fixed Firebase Read / Write Failure Instances
                    - Added LED Status To Represent Firebase Read / Write In Progress
    Version 1.8OC   - Version 1.8 Original Copy
    Version 1.8     - Rewrite Code And Remove Unused Variables
                    - Add First Read / Write Function Before Everything Else Runs
                        - Can't Get It To Start WiFiServer server(portNumber);
                          With Firebase Read
    Version 1.9     - Add New Node Auto Write Defaults To Firebase
                    - 1.9.10 - Added Net Suffix System From Beta # To x.x.##
    Version 1.10.1  - Fix the Client Web Server
            1.10.2  - Add Client Server Port From Firebase
            1.10.3  - Added More Commenting
            1.10.4  - Add Enable / Disable Serial Debug Log Function
            1.10.5  - Test Upload PB Number Write To Firebase
                    - Test Read Default Write Delay From Firebase
                    - Test Read WiFi Reset Counter From Firebase
            1.10.6  - Updated To Secondary Build For Longevity Testing
            1.10.7  - Update Default Update Time To Seconds In Firebase
                    - Update WiFi Reset Cycles Count To Seconds In Firebase
            1.10.8  - A Lot Better Commenting
            1.10.9  - Add Current Update Version Read From Firebase As Global Variable In Firebase
                    - Also Made Single Directory Writer Program For ESP32 As Well To Help With Writing Directory Trees Or Values
            1.10.10 - 


    Version 2.0.0   - Migrate From Send / Receive Boot Up Voids To Unified "bootFirebase();"
                    - Migrate From Send / Receive Auto Call Voids To Unified "updateFirebase();"
                        - Add Port Number Update And Server Start During Update Period
*/
//--------------------------------------------------------------------------------------------------
//                      Libraries
//--------------------------------------------------------------------------------------------------
#include <WiFi.h>
#include <FirebaseESP32.h>
//--------------------------------------------------------------------------------------------------
//                      All Credentials
//--------------------------------------------------------------------------------------------------
//------------------------------------------------- Firebase Host and Token
#define FIREBASE_HOST "https://YourTRDOnFirebase-rtdb.firebaseio.com"
#define FIREBASE_AUTH "YourAuthKeyFromFirebase"
//------------------------------------------------- Local WiFi Credentials
//const char* ssid     = "Test2021";
//const char* password = "Test2021Pass";
//------------------------------------------------- Get External IP Address (Recommended not to change)
const char* hostName = "api.ipify.org";
//--------------------------------------------------------------------------------------------------
//                      First Run and Node Information
//--------------------------------------------------------------------------------------------------
String versionNumber = "V2.1.1";  // Current Version Of Software
int newPB = false;              // Set this to true if you are adding a new node, also change phNumber
String pbNumber = "PB001-Alpha";      // Pointless Button Node Number, Change This When Adding New Nodes
int serialDebugLogging = true;  // This Enables Serial Debug Printouts, It Is Set To False If No Serial Is Detected In void setup()
//--------------------------------------------------------------------------------------------------
//                      All Variables
//--------------------------------------------------------------------------------------------------
#define ledPin 18  // This Is The Physical Pin Definition For The LED
#define buttonPin 0  // This Is The Physical Pin Definition For The Physical Button
int buttonState;  // This Is The Pysical Button State, It Also Controls The pressState
int pressState;  // This Is The Press State That Stops Double Up Clicks From Holding It
long count;  // This Is The Count String, Its What Holds The Pointless Count
IPAddress lip;  // This Is The IP Address String Type For The Local IP Address
IPAddress eip;  // This Is The IP Address String Type For The External IP Address
String ep;  // This Sets A Second String For The External IP Address To Be Stored In
int defaultDelay = 10;  // Default Is 10, Anyting Higher Than 100 Has Button Read Errors
int initialReadFailureDelay = 5000;  // Time In ms Between Next Read Operation To Firebase After Failure
int initialReadSuccessDelay = 1000;  // Time In ms Between Next Read Operation To Firebase After Success
int initialWriteSuccessDelay = 5000;  // Time In ms Between Next Write Operation To Firebase After Failure
int initialWriteFailureDelay = 5000;  // Time In ms Between Next Write Operation To Firebase After Success
int defaultWriteDelay = 300;  // Time Between Data Updates To Firebase In Seconds (300 s Is Default (300 s = 5 Minutes))
int wifiResetCounter = 10;  // Time WiFi Will Wait For A Connection Before Resetting In Seconds (6 s Is Default)
int portNumber = 90;  // This is the default value and can be changed in Firebase later
WiFiServer server(portNumber);  // This just defines the "server" before it has a port read from Firebase after setting up a new node
int firstReadMode;  // This Is The Mode State For The First Firebase Read
int firstWriteMode;  // This Is The Mode State For The First Firebase Write
int noConnectReset;  // This Is The Current Count For The Wireless Reset Counter
int sendTime;  // This Is The Current millis() For The Default Write Delay Time
int prevSendTime;  // This Is The Previous millis() For The Default Write Delay Time
int firebaseState;  // This Is The Display State During Firebase Uploads For Web Client
int bootReadMode;  // Boot Read Firebase Mode State For Unified Boot In V2.X.X
int updateReadMode;  // Global Update Firebase Mode State For Unified Update In V2.X.X
String currentRelease;
FirebaseData fbdo;
//--------------------------------------------------------------------------------------------------
//                      Void Reset Function (Keep Above All Other Voids)
//--------------------------------------------------------------------------------------------------
void(* resetFunc) (void) = 0;  // This Is The Software Reset Function, Just Like A Reboot. It Needs To Be Above All Other Voids To Be Called From Them
//--------------------------------------------------------------------------------------------------
//                      String Get External IP
//--------------------------------------------------------------------------------------------------
String getIp() {
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
//--------------------------------------------------------------------------------------------------
//                      Void First WiFi Connect
//--------------------------------------------------------------------------------------------------
void firstWiFiConnect() {
  digitalWrite(ledPin, LOW);  // Set The LED To Low To Show Work In Progress
  firebaseState = 1;  // Set Client Server State To Standby
  noConnectReset = 0;  // Reset The Base WiFi Reconnect Counter
  if (serialDebugLogging) {  // This if Statement just says if serialDebugLogging Is true, Then Print This, That Is How All Of These if Statements Work
    Serial.print("Connecting WiFi: ");
    Serial.print(ssid);
    Serial.print(" ");
  }
  WiFi.begin(ssid, password);  // Starting The WiFi Connection On The Radio
  while (WiFi.status() != WL_CONNECTED) {  // While It Is Not Connected, Do This
    noConnectReset ++;  // Add 1 To The Base WiFi Reconnect Counter
    if (serialDebugLogging) {
      Serial.print("*");
    }
    delay(500);
    if (noConnectReset >= (wifiResetCounter * 2)) { // If The Connection Is Not Made Within Default Peramiter On Bootup, Use Software Reset
      if (serialDebugLogging) {
        Serial.println("");
        Serial.println("Device Reset Limit Reached. Resetting.");
      }
      delay(1000);
      resetFunc();  // Software Reset Call
    }
  }  // If It Connects Before The Limit Is Reached, It Will Readch This
  WiFi.setSleep(false);  // Said To Force It To Not Go Idle
  firebaseState = 0;  // Set Client Server State To Live
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  // Start The Firebase Connection
  Firebase.reconnectWiFi(true);  // Set Firebase To Reconnect If Wireless Fails And Reconnects
  if (serialDebugLogging) {
    Serial.println("");
    Serial.print("WiFi connected: ");
  }
  lip = WiFi.localIP();  // This Sets The Local IP Address Variable
  if (serialDebugLogging) {
    Serial.print(lip);
    Serial.println("");
  }
  String(eip) = (getIp());  // This Calls The String To Get The Public IP Address Variable
  if (serialDebugLogging) {
    Serial.print("External: ");
    Serial.print(eip);
    Serial.println("");
  }
  //------------------------------------------------- LIP
  delay(1000);
}
//--------------------------------------------------------------------------------------------------
//                      Void Reconnect WiFi
//--------------------------------------------------------------------------------------------------
void reconnectWiFi() {
  digitalWrite(ledPin, LOW);  // Set The LED To Low To Show Work In Progress
  firebaseState = 1;  // Set Client Server State To Standby
  noConnectReset = 0;  // Reset The Base WiFi Reconnect Counter
  WiFi.disconnect();  // Just To Be Safe On A Reconnect, I Like To Try And Force A Disconnect Before Trying A Reconnect
  if (serialDebugLogging) {
    Serial.print("Reconnecting WiFi: ");
    Serial.print(ssid);
    Serial.print(" ");
  }
  WiFi.begin(ssid, password);  // Trying The Wireless Network Again
  while (WiFi.status() != 3) {  // While It Is Not Connected
    if (serialDebugLogging) {
      Serial.print(".");
    }
    noConnectReset ++;
    delay(500);
    if (noConnectReset >= (wifiResetCounter * 2)) { // Check Reset Counter And Use Software Reset Again If It Fails
      if (serialDebugLogging) {
        Serial.println("");
        Serial.println("Device Reset Limit Reached. Resetting.");
      }
      delay(1000);
      resetFunc();  // Call Software Reset Function
    }
  }
  WiFi.setSleep(false);  // Said To Keep It From Going Idle
  firebaseState = 1;  // Set Client Server State To Standby
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);  // Connect To Firebase Again
  Firebase.reconnectWiFi(true);  // Set It To Reconnect Again
  if (serialDebugLogging) {
    Serial.println("");
    Serial.print("WiFi reconnected: ");
  }
  lip = WiFi.localIP();  // Set The Local IP Address Again
  if (serialDebugLogging) {
    Serial.print(lip);
    Serial.println("");
  }
  String(eip) = (getIp());  // Set The External IP Address Again
  if (serialDebugLogging) {
    Serial.print("External: ");
    Serial.print(eip);
    Serial.println("");
    Serial.println("Reconnected........................................");
  }
  delay(1000);
}
//--------------------------------------------------------------------------------------------------
//                      Void First Read Firebase
//--------------------------------------------------------------------------------------------------
void firstReadFirebase() {
  digitalWrite(ledPin, LOW);  // Set LED To Low To Show Work In Progress
  firebaseState = 1;  // Set Client Server State To Standby
  //------------------------------------------------- Pointless Count
  while (firstReadMode == 0) {  // If Reading This String, Do This
    if (serialDebugLogging) {
      Serial.print("Initial Count Read ");
    }
    if (Firebase.getInt(fbdo, "/Counters/" + pbNumber + "/01-PointlessCount")) {  // This Will Read The "/01-....." Directory
      count = fbdo.intData();  // If The Read Was Successful, Count Will Be Updated With The String Data Received
      delay(initialReadSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(count);
      }
      firstReadMode = 1;  // If Successful Move To Next Mode
    }
    else {
      if (serialDebugLogging) {  // If Error
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialReadFailureDelay);  // Defined At The Top
      firstReadMode = 0;  // If Unsuccessful, Do It Again
    }
  }
  //------------------------------------------------- Port Number
  while (firstReadMode == 1) {
    if (serialDebugLogging) {
      Serial.print("Initial Port Number Read ");
    }
    if (Firebase.getInt(fbdo, "/Counters/" + pbNumber + "/02-PortNumber")) {
      portNumber = fbdo.intData();
      server.begin(portNumber);  // This Starts The Updated Cliet Web Server Immediatly After Receiving Updated Port From Firebase, Only Once At Boot Pre-V2.X.X
      delay(initialReadSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(portNumber);
      }
      firstReadMode = 2;  // If Successful, Move On
    }
    else {
      if (serialDebugLogging) {  // If Error
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
        delay(initialReadFailureDelay);  // Defined At The Top
      }
      firstReadMode = 1;  // If Unsuccessful, Do It Again
    }
  }
  //------------------------------------------------- Read Default Write Delay
  while (firstReadMode == 2) {
    if (serialDebugLogging) {
      Serial.print("Default Update Delay Read ");
    }
    if (Firebase.getInt(fbdo, "/Counters/" + pbNumber + "/10-DefaultUpdateDelay")) {
      defaultWriteDelay = fbdo.intData();  // Reading Auto Update Delay And Updating The Variable
      delay(initialReadSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(defaultWriteDelay);
      }
      firstReadMode = 3;  // If Successful, Move On
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
        delay(initialReadFailureDelay);  // Defined At The Top
      }
      firstReadMode = 2;  // If Unsuccessful, Do It Again
    }
  }
  //------------------------------------------------- Read WiFi Reset Counter
  while (firstReadMode == 3) {
    if (serialDebugLogging) {
      Serial.print("Default WiFi Reset Cycles Read ");
    }
    if (Firebase.getInt(fbdo, "/Counters/" + pbNumber + "/11-WiFiResetCycles")) {
      wifiResetCounter = fbdo.intData();  // Reading WiFi Reset Counter And Updating Its Variable
      delay(initialReadSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(wifiResetCounter);
      }
      firstReadMode = 4;  // If Successful, Move On
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
        delay(initialReadFailureDelay);  // Defined At The Top
      }
      firstReadMode = 3;  // If Unsuccessful, Do It Again
    }
  }
  //------------------------------------------------- End First Read
  if (firstReadMode >= 4) {  // If The Number Is Set Higher Than The Ammount Of Steps In Process, Finish And Move On
    firebaseState = 0;  // Setting Client Web Server To Live
    firstReadMode = 4;
  }
}
//--------------------------------------------------------------------------------------------------
//                      Void First Write Firebase
//--------------------------------------------------------------------------------------------------
void firstWriteFirebase() {
  digitalWrite(ledPin, LOW);  // Setting LED To Low To Show Work In Progress
  firebaseState = 1;  // Setting Cllient Web Server To Standby
  //------------------------------------------------- Will Only Write If newPB is true
  while (firstWriteMode == 0) {
    if (serialDebugLogging) {
      Serial.print("Initial Write Count ");
    }
    if (Firebase.setInt(fbdo, "/Counters/" + pbNumber + "/01-PointlessCount", 0)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(0);
      }
      firstWriteMode = 1;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialWriteFailureDelay);  // Defined At The Top
      firstWriteMode = 0;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Will Only Write If newPB is true
  while (firstWriteMode == 1) {
    if (serialDebugLogging) {
      Serial.print("Initial Write Port Number ");
    }
    if (Firebase.setInt(fbdo, "/Counters/" + pbNumber + "/02-PortNumber", portNumber)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(portNumber);
      }
      firstWriteMode = 2;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialWriteFailureDelay);  // Defined At The Top
      firstWriteMode = 1;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Will Only Write If newPB is true
  while (firstWriteMode == 2) {
    if (serialDebugLogging) {
      Serial.print("Write Initial Default Write Delay ");
    }
    if (Firebase.setInt(fbdo, "/Counters/" + pbNumber + "/10-DefaultUpdateDelay", defaultWriteDelay)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(defaultWriteDelay);
      }
      newPB = false;
      firstWriteMode = 3;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialWriteFailureDelay);  // Defined At The Top
      firstWriteMode = 2;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Will Only Write If newPB is true
  while (firstWriteMode == 3) {
    if (serialDebugLogging) {
      Serial.print("Write Initial Default WiFi Reset Cycles ");
    }
    if (Firebase.setInt(fbdo, "/Counters/" + pbNumber + "/11-WiFiResetCycles", wifiResetCounter)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(wifiResetCounter);
      }
      newPB = false;
      firstWriteMode = 4;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialWriteFailureDelay);  // Defined At The Top
      firstWriteMode = 3;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Write PB Number
  while (firstWriteMode == 4) {
    if (serialDebugLogging) {
      Serial.print("Initial Write PB Number ");
    }
    if (Firebase.setString(fbdo, "/Counters/" + pbNumber + "/00-PB_Number", pbNumber)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(pbNumber);
      }
      newPB = false;
      firstWriteMode = 5;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialWriteFailureDelay);  // Defined At The Top
      firstWriteMode = 4;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Write Local IP
  while (firstWriteMode == 5) {
    if (serialDebugLogging) {
      Serial.print("Initial Write LIP ");
    }
    String lip2 = WiFi.localIP().toString();
    if (Firebase.setString(fbdo, "/Counters/" + pbNumber + "/03-LocalIP", lip2)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(lip2);
      }
      firstWriteMode = 6;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialWriteFailureDelay);  // Defined At The Top
      firstWriteMode = 5;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Write External IP
  while (firstWriteMode == 6) {
    ep = getIp();
    if (serialDebugLogging) {
      Serial.print("Initial Write EIP ");
    }
    if (Firebase.setString(fbdo, "/Counters/" + pbNumber + "/04-ExternalIP", ep)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(ep);
      }
      firstWriteMode = 7;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
      }
      delay(initialWriteFailureDelay);  // Defined At The Top
      firstWriteMode = 6;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Read Current Release Version
  while (firstWriteMode == 7) {
    if (serialDebugLogging) {
      Serial.print("Reading Current Release Version ");
    }
    if (Firebase.getString(fbdo, "/00-Global/00-Current_Release_Number")) {
      currentRelease = fbdo.stringData();
      delay(initialReadSuccessDelay);
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(currentRelease);
        delay(1000);
      }
      firstWriteMode = 8;  // If Successful, Move On
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
        delay(initialReadFailureDelay);  // Defined At The Top
      }
      firstWriteMode = 7;  // If Unsuccessful, Do It Again
    }
  }
  //------------------------------------------------- Write Local Version Number
  while (firstWriteMode == 8) {
    if (serialDebugLogging) {
      Serial.print("Writing Local Release Version ");
    }
    if (Firebase.setString(fbdo, "/Counters/" + pbNumber + "/14-VersionNumber", versionNumber)) {
      delay(initialWriteSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful,   ");
        Serial.println(versionNumber);
      }
      firstWriteMode = 9;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
        delay(initialWriteFailureDelay);  // Defined At The Top
      }
      firstWriteMode = 8;  // If Unsuccessful
    }
  }
  //------------------------------------------------- End First Write
  if (firstWriteMode >= 9) {  // When The Mode Is Higher Than Steps Needed, Move On
    firebaseState = 0;
    firstWriteMode = 9;
  }
}
//--------------------------------------------------------------------------------------------------
//                      Void Read Firebase Count
//--------------------------------------------------------------------------------------------------
void readFirebaseCount() {  // This Void Is The Current Boot Update Read Void
  digitalWrite(ledPin, LOW);
  firebaseState = 1;
  if (serialDebugLogging) {
    Serial.print("Update Read Count ");
  }
  if (Firebase.getInt(fbdo, "/Counters/" + pbNumber + "/01-PointlessCount")) {
    delay(1000);
    if (serialDebugLogging) {
      Serial.print("Success, ");
      Serial.println(fbdo.intData());
    }
    firebaseState = 0;
    count = fbdo.intData();
  }
  else {
    if (serialDebugLogging) {
      Serial.print("Error, ");
      Serial.println(fbdo.errorReason());
    }
    delay(5000);
    firebaseState = 1;
    readFirebaseCount();
  }
}
//--------------------------------------------------------------------------------------------------
//                      Void Read Firebase Updates
//--------------------------------------------------------------------------------------------------
void readFirebaseGlobals() {  // This Void Is The Current Auto Updare Read Void
  //------------------------------------------------- Read Default Update Delay
  while (updateReadMode == 0) {
    if (serialDebugLogging) {
      Serial.print("Default Update Delay Read ");
    }
    if (Firebase.getInt(fbdo, "/Counters/" + pbNumber + "/10-DefaultUpdateDelay")) {
      defaultWriteDelay = fbdo.intData();
      delay(initialReadSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(defaultWriteDelay);
      }
      updateReadMode = 1;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
        delay(initialReadFailureDelay);  // Defined At The Top
      }
      updateReadMode = 0;  // If Unsuccessful
    }
  }
  //------------------------------------------------- Read WiFi Reset Counter
  while (updateReadMode == 1) {
    if (serialDebugLogging) {
      Serial.print("Default WiFi Reset Cycles Read ");
    }
    if (Firebase.getInt(fbdo, "/Counters/" + pbNumber + "/11-WiFiResetCycles")) {
      wifiResetCounter = fbdo.intData();
      delay(initialReadSuccessDelay);  // Defined At The Top
      if (serialDebugLogging) {
        Serial.print("Successful, ");
        Serial.println(wifiResetCounter);
      }
      updateReadMode = 2;  // If Successful
    }
    else {
      if (serialDebugLogging) {
        Serial.print("Failed, ");
        Serial.println(fbdo.errorReason());
        delay(initialReadFailureDelay);  // Defined At The Top
      }
      updateReadMode = 1;  // If Unsuccessful
    }
  }
  //------------------------------------------------- End Update Read
  if (updateReadMode >= 2) {
    firebaseState = 0;
    updateReadMode = 2;
  }
}
//--------------------------------------------------------------------------------------------------
//                      Void Write Firebase Count
//--------------------------------------------------------------------------------------------------
void writeFirebaseCount() {    // This Void Is The Current Auto Updare Write Void
  digitalWrite(ledPin, LOW);
  if (serialDebugLogging) {
    Serial.print("Write Count ");
  }
  firebaseState = 1;
  if (Firebase.setInt(fbdo, "/Counters/" + pbNumber + "/01-PointlessCount", count)) {
    delay(1000);
    if (serialDebugLogging) {
      Serial.print("Success, ");
      Serial.println(count);
    }
    firebaseState = 0;
  }
  else {
    if (serialDebugLogging) {
      Serial.print("Error, ");
      Serial.println(fbdo.errorReason());
    }
    delay(5000);
    firebaseState = 1;
    writeFirebaseCount();
  }
}
//--------------------------------------------------------------------------------------------------
//                      Void Setup
//--------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  if (!Serial) {
    serialDebugLogging = false;
  }
  if (serialDebugLogging) {
    Serial.println(" - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - ");
    Serial.println(String("- Starting - ") + versionNumber + String(" - ") + pbNumber + String(" - - - - - - - - - - ") + String("New Node?: ") + newPB);
  }
  delay(2000);
  pinMode(ledPin, OUTPUT);  // This Sets The Pin Mode For The Physical Button
  digitalWrite(ledPin, HIGH);  // This Sets The Pin Mode For THe LED
  delay(1000);
  digitalWrite(ledPin, LOW);  // This Is Turning The LED Off At Boot
  if (serialDebugLogging) {
    Serial.println("--------------------------------------------- First WiFi");
  }
  firstWiFiConnect();  // This Calls The First WiFi Connection Void Above
  if (newPB == true) {  // This Checks To See If It Needs To Load As A New Node And Changes The Boot Order Accordingly
    firstWriteMode = 0;  // If newPB Is Set To true, It Will Overwrite Any Existing Data Under Its pbNumber
  }
  else if (newPB == false) {  // Change If Adding / Removing Steps To newPB In firstWriteFirebase();
    firstWriteMode = 4;
  }
  if (serialDebugLogging) {
    Serial.println("--------------------------------------------- First Write");
  }
  firstWriteFirebase();  // This Calls The First Write To Firebase Void Above, Also Changed Based On newPB Variable
  if (serialDebugLogging) {
    Serial.println("--------------------------------------------- First Read");
  }
  firstReadFirebase();  // This Calls The First Read Of Firebase Values At Boot
  firebaseState = 0;  // When All That Is Done, Set Client Web Server To Live Again
  server.begin(portNumber);
  if (serialDebugLogging) {
    Serial.println("- SETUP COMPLETE - - - - - - - - - -");
  }
  delay(500);
}
//--------------------------------------------------------------------------------------------------
//                      Void Loop
//--------------------------------------------------------------------------------------------------
void loop() {
  if (WiFi.status() == 2) {  // Checks Each WiFi Status From The Wireless Card And Does Things Accordingly
    if (serialDebugLogging) {
      Serial.println("Scan Complete");
    }
  }
  else if (WiFi.status() == 0) {
    if (serialDebugLogging) {
      Serial.println("WiFi Idle");
    }
    reconnectWiFi();
  }
  else if (WiFi.status() == 1) {
    if (serialDebugLogging) {
      Serial.println("No SSID");
    }
    reconnectWiFi();
  }
  else if (WiFi.status() == 4) {
    if (serialDebugLogging) {
      Serial.println("Connection Failed");
    }
    delay(1000);
    reconnectWiFi();
  }
  else if (WiFi.status() == 5) {
    if (serialDebugLogging) {
      Serial.println("WiFi Connection Lost");
    }
    reconnectWiFi();
  }
  else if (WiFi.status() == 6) {
    if (serialDebugLogging) {
      Serial.println("WiFi Disconnected");
    }
    reconnectWiFi();
  }
  else if (WiFi.status() == 3) {
    //------------------------------------------------------------------ This Is The Main Loop That Runs While Its Connected
    sendTime = millis();  // This Sets The Send Time Variable
    // This if Statement Is Saying, If The Send Time Is Greater Or Equal To Previous Time Plus Detault Write Delay Multiplied By 1000
    // The Multiplied By 1000 Is How We Are Able To Use Seconds In Firebase As A Global Readable Value
    if (sendTime >= prevSendTime + (defaultWriteDelay * 1000)) {
      prevSendTime = sendTime;
      digitalWrite(ledPin, LOW);
      writeFirebaseCount();
      updateReadMode = 0;
      readFirebaseGlobals();
    }
    buttonState = digitalRead(buttonPin);
    if (buttonState == LOW) {
      if (pressState == 0) {
        count++;
        pressState = 1;
        if (serialDebugLogging) {
          Serial.println(String("Count: ") + String(count));
        }
        delay(1);
      }
      else if (pressState == 1) {
        digitalWrite(ledPin, LOW);
      }
    }
    else if (buttonState == HIGH) {
      pressState = 0;
      digitalWrite(ledPin, HIGH);
    }
    if (firebaseState == 0) {  // This Is THe Live VS Standby Test, If 0, Run This, If 1, Run Next else if Statement
      WiFiClient client = server.available();   // listen for incoming clients
      if (client) {                             // if you get a client,
        if (serialDebugLogging) {
          Serial.println("New Client.");           // print a message out the serial port
        }
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected()) {            // loop while the client's connected
          if (client.available()) {             // if there's bytes to read from the client,
            char c = client.read();             // read a byte, then
            Serial.write(c);                    // print it out the serial monitor
            if (c == '\n') {                    // if the byte is a newline character
              // if the current line is blank, you got two newline characters in a row.
              // that's the end of the client HTTP request, so send a response:
              if (currentLine.length() == 0) {
                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                // and a content-type so the client knows what's coming, then a blank line:
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");  // This Does Not Appear To Work On Mobile Browsers For Some Reason
                client.println();
                // the content of the HTTP response follows the header:
                client.print("<h1>");  // Start Mass Formatting
                client.print("PB#: ");  // Display PB Number
                client.print(pbNumber);
                client.print("<br>");
                client.print("<br>");
                client.print("Count: ");  // Display Count Number
                client.print(count);
                client.print("<br>");
                client.print("<br>");
                client.print("Local IP: ");  // Display Local IP
                client.print(lip);
                client.print("<br>");
                client.print("<br>");
                client.print("Port: ");  // Display Port Number
                client.print(portNumber);
                client.print("<br>");
                client.print("<br>");
                client.print("External IP: ");  // Display External IP
                client.print(ep);
                client.print("<br>");
                client.print("<br>");
                client.print("Version Number: ");  // Display Version Number
                client.print(versionNumber);
                client.print("<br>");
                client.print("<br>");
                client.print("</h1>");  // End Mass Formatting
                // The HTTP response ends with another blank line:
                client.println();
                // break out of the while loop:
                break;
              } else {    // if you got a newline, then clear currentLine:
                currentLine = "";
              }
            } else if (c != '\r') {  // if you got anything else but a carriage return character,
              currentLine += c;      // add it to the end of the currentLine
            }
            if (currentLine.endsWith("GET /favicon.ico HTTP/1.1")) {  // If The Client Asks For A Favorite Icon FIle, It Closes The Connection
              //client.print("404 ERROR");  // Testing Only, Does Not Help Much
              client.stop();
            }
            if (currentLine.endsWith("GET /resetdevice")) {  // If The Client Sends A /resetdevice It Will Call A Software Reset
              if (serialDebugLogging) {
                Serial.println("- - - - - - - - - - Browser Reset Called - - - - - - - - - -");
              }
              client.stop();  // Close The Client Connection
              delay(3000);  // Wait A Bit
              resetFunc();  // Call Software Reset
            }
            if (currentLine.endsWith("GET '\n'")) {  // If The Client Sends Another Blank End, It Closes The Connection
              delay(1000);
              client.stop();
            }
          }
          //client.stop();  // Testing Only, Does Not Help Much
        }
        // Close The Connection For Good
        client.stop();
        if (serialDebugLogging) {
          Serial.println("  Client Disconnected.");
        }
      }
    }
    else if (firebaseState == 1) {  // This Is The Standby State For The Client Web Server.
      if (serialDebugLogging) {
        Serial.println("Writing To Firebase, Please wait...");
      }
    }
  }
  else {
    if (serialDebugLogging) {  // When All Else Fails On The Wireless Card, Print The Error And Move On
      Serial.println(String("Unknown Status: ") + String(WiFi.status()));
    }
  }
  delay(defaultDelay);  // Default Whole System Delay Defined At The Top (Default Is 10, Anyting Higher Than 100 Has Button Read Errors)
}

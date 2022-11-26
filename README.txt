Pointless_Button_V2 - Next Planned Release Version Date: February 2023

HARDWARE SUBJECT TO CHANGE!! - Current Hardware: ESP32_WROOM32_DEV_MODULE

Just a heads up, this is my master notes doc and will change quite frequently while I am working on the project.


# ----------------------------------------------------------------------------------------------------------
0                        Table Of Contents:
# ----------------------------------------------------------------------------------------------------------

0 - Table Of Contents
1 - Version History and *Currently Planned for Implimentation
2 - Notes
3 - Current Order Of Operations (How it works / runs)
4 - Possible Future Plans
5 - How To Guides I Need To Create




# ----------------------------------------------------------------------------------------------------------
1                        Version History and *Currently Planned for Implimentation
# ----------------------------------------------------------------------------------------------------------

      V2.0.0  - Connect Buttons (Current Build Uses 4 Buttons and 4 LEDs)
              - Base WiFi and WiFi Reset Functions
              - Multi SSID Failover Functions
              - Basic Firebase Read Functions
              - Basic Count Functions
              - Basic Firebase Write and Update Functions
              - Release and Beta Version Checker
              - *Add Firebase Initial Setup Guide
              - *Add Firebase Default Node Guide
              - *
              
              
     V2.0.1   - *Add LED Success / Fail Indicators for Boot and Updates
              - *Add Button Combo to Trigger a Reboot
              - *Add Current SSID String and Firebase Entry
              - *Move Boot Cycle to its own String and FIrebase Entry
              - *Local Web GUI
              - *Add Remote LED Test Trigger
              - *
              
              
     V2.1.0   - *Create Python Intermediate Server For Time Keeping
              - *Last Boot Time Recording
              - *Last Update Time Recording
              - *




# ----------------------------------------------------------------------------------------------------------
2                        Notes
# ----------------------------------------------------------------------------------------------------------
     
     V2.1.0   - Things I want for next major version.
     
     V2.0.1   - This is going to be a lot of extra functions and indicators for things normally moditored over serial.
     
     V2.0.0   - The 1.0.0 to 1.10.10 versions are basicallt all in version 2.0.0.




# ----------------------------------------------------------------------------------------------------------
3                        Current Order Of Operations (How it works / runs)
# ----------------------------------------------------------------------------------------------------------

      - Power On
      - Green Button 1 will begin to flash as it attempts to connect to WiFi.
      - Connect to 1 of 2 SSIDs, tries one, if it fails, it tries the second one, if they both fail, it reboots the whole ESP32 and starts from the begining.
      - Once its connected to the SSID, Green Button 1 will stay illuminated.
      - Green Button 2 will begin to flash as it checks its internal and external IP addresses. Once they are found, Green Button 2 stays illuminated.
      - Red Button 1 will begin to flash as it attempts to authenticate with Firebase, once successful Red Button 1 stays illuminated.
      - Red Button 2 will begin to flash as it reads its boot variables from Firebase and writes its after boot variables if any have changed.
      - Once all that is done, all 4 buttons will illuminate for a second, turn off, then flash back on in the order or G1, G2, R1, R2.
      - Once all 4 buttons are not illuminated, the buttons are ready to count.
      - It will update the count based on the "Default_Update_Delay_In_Seconds" variable in Firebase, by default I set them to 600 secons (10 minutes)
      so it does not overload Firebase with a bunch of requests each time the buttons are pressed. This also stops button lag from occuring while pressing
      them. Future plan is to run the Firebase stuff as a sub process in Python so it does not interupt the counting cycle, this update is dependant on
      the choice of hardware in the fugure for the next major version release.
      - While updating, it will stop counting the buttons and stop illuminating them while pressing a button to perform the Firebase update.
      - Once it is finished, it goes back to counting and illuminating on button presses.




# ----------------------------------------------------------------------------------------------------------
4                        Possible Future Plans
# ----------------------------------------------------------------------------------------------------------
     - I may switch out the ESP32 boards for Raspberry Pi Pico W boards to hopefully make the battery pack perform more efficiently.
     
     - I eventually want to have it running in Python again like hardware version 1 originally had, this way I can do sub processing of the Firebase
     tasks and not interupt the counting tasks while it does it. Right now it basically switches between two loops, the main counting loop and the
     Firebase update loop.
     
     - I want this eventually to have the ability to be directly connected to over the wireless, possibly Bluetooth if I go with a Raspberry Pi Zero W
     again to go back to running it on Python, so you can change the primary SSID it connects to each boot. That way you wont have to open it up, plug
     it into your computer, and upload a new sketch to change it. Might also need SD storage to keep those values unless I can figure out how to write
     values to EPROM or some other non-volatile means of storage.
     
     - *





# ----------------------------------------------------------------------------------------------------------
5                        How To Guides I Need To Create
# ----------------------------------------------------------------------------------------------------------

      - Create Firebase Realtime Database and Link to ESP32 (Other board instructions will vary and I will only be covering the hardware I use.
      









#END

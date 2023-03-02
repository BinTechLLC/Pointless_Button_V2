Pointless_Button_V2 - Next Planned Release Version Date: September 2023

Current Release Version       : V2.0.1                 Release Date: 3-2-2023

Current Beta Release Version  : V2.0.2-Beta            Release Date: TBA

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
              
              
     V2.0.1   - Add LED Success / Fail Indicators for Boot and Updates
              - (Not going to add, have had a lot of people mash multiple / all buttons at a time, would cause an unwanted reboot.
              ## Add Button Combo to Trigger a Reboot
              - Add Current SSID String and Firebase Entry
              - Move Boot Cycle to its own String and Firebase Entry
              - Add Firebase Read Entry for Serial Debug Output and Serial Count Output
              - Added NETBIOS name configuration to pull from pbName string so it no longer shows "esp32-arduino" on the network.
              - Add NTP Read and strftime Variables
                        - Now that NTP and strftime are working, rework code to run based on that for time like the original Python version did, rather than system delays. This will give the updates more rounded update times and would ignore the time needed to perform update operations. Right now it has a default system delay of 50 ms, every 50 ms it increases a count cycle by 1, which reads the default update delay, and for every amount of cycles that is equal to 1 second, it adds to the delay second count. When the delay second count hits the default update count number, it runs the update, the update also takes some time and while it is running, it stops counting system ticks. So a 600 second or 10 minute update delay is actually something like 660 seconds on the high end if the WiFi signal is not that great. The time.h library is still keeping track of time in the background with millis and will update at the next 600 second interval but does not wait for the 60 ish seconds the update takes to keep counting for the net update. After a few days of it running, the updates will slowly start to loose time frame due to the read write delay of the update process, so your boot time might be 01:00:00 but the last update after a few days can be at 01:23:12 or something, I just pulled those numbers and did not really do the math. But with time.h running it, the updates would come in as 01:00:00, 01:10:00, 01:20:00, and so on. I assume the seconds will be that accurate with it checking time every 50 ms, or 20 times a second. It should also be noted that it only pulls time once during the First WiFi Connect function and then uses the onboard memory to keep track of it from ther on out, it will only update again if it has to reconnect to the WiFi. I assume it can be configured to pull it every time it needs it but that seems excessive and would also add more to network usage.
              - Add Last Boot Time and Last Update Time Firebase Entries
              - Add Remote LED Test Trigger
              
              
    V2.0.2    - Added USB port extension faceplate to conduit box so it doesn't need to be opened to be connected to.
              - Found issue with USB port, still have to open to push download button on chip.
                        - Added jumper wires to download button on chip and wired to isolated USB faceplate screws on outside of box.
                        - Now make contact with both screws with a jumper wire to trigger download mode.
              
              
     V2.0.3   - *Add Create New Node Function for New Boxes
              - *Firebase Directory Restructure to group like things together. (Previous to this it was in order of as added.)
              - *Change out delay clock for NTP clock to control update times. **V2.0.2-NTP
              
     V2.0.4   - *Local Web GUI
              - *Maybe App functionality
              - *
              
              
     V2.1.0   - *


# ----------------------------------------------------------------------------------------------------------
2                        Notes
# ----------------------------------------------------------------------------------------------------------
     
     V2.1.0   - Things I want for next major version.
     
     V2.0.3   - Web GUI and possibly App integration.
     
     V2.0.2-NTP  - Now that NTP and strftime are working, rework code to run based on that for time like the original Python version did, rather than system delays. This will give the updates more rounded update times and would ignore the time needed to perform update operations. Right now it has a default system delay of 50 ms, every 50 ms it increases a count cycle by 1, which reads the default update delay, and for every amount of cycles that is equal to 1 second, it adds to the delay second count. When the delay second count hits the default update count number, it runs the update, the update also takes some time and while it is running, it stops counting system ticks. So a 600 second or 10 minute update delay is actually something like 660 seconds on the high end if the WiFi signal is not that great. The time.h library is still keeping track of time in the background with millis and will update at the next 600 second interval but does not wait for the 60 ish seconds the update takes to keep counting for the net update. After a few days of it running, the updates will slowly start to loose time frame due to the read write delay of the update process, so your boot time might be 01:00:00 but the last update after a few days can be at 01:23:12 or something, I just pulled those numbers and did not really do the math. But with time.h running it, the updates would come in as 01:00:00, 01:10:00, 01:20:00, and so on. I assume the seconds will be that accurate with it checking time every 50 ms, or 20 times a second. It should also be noted that it only pulls time once during the First WiFi Connect function and then uses the onboard memory to keep track of it from ther on out, it will only update again if it has to reconnect to the WiFi. I assume it can be configured to pull it every time it needs it but that seems excessive and would also add more to network usage.
     
     V2.0.1   - This is going to be a lot of extra functions and indicators for things normally moditored over serial.
     
     V2.0.0   - The 1.0.0 through 1.10.10 versions are basically all in version 2.0.0 as the base version. I do plan on updating
                  the code on hardware version 1 to match the new implimentations of the hardware version 2 stuff.




# ----------------------------------------------------------------------------------------------------------
3                        Current Order Of Operations (How it works / runs)
# ----------------------------------------------------------------------------------------------------------
      
      Current Process (>= V1.0.1-Beta)
      - Power On
      - Green Button 1 will begin to flash as it attempts to connect to WiFi
      - Once it connects to one of two SSIDs, Green Button 1 will stay illuminated for one second.
      - The green buttons will alternate as it steps through the Firebase read / write functions.
      - When all boot up functions have been successful, it will illuminate both green buttons saying it is done.
      - If there were any errors in the boot process, red button 1 will also illuminate while both green buttons are indicating a done state.
      - If there were any errors in the update process, red button 2 will also illuminate while both green buttons are indicating a done state.
      - After that, each button will turn on in order once then turn off to test the LEDs.
      - It will update the count based on the "Default_Update_Delay_In_Seconds" variable in Firebase, by default I set them to 600 secons (10 minutes)
      so it does not overload Firebase with a bunch of requests each time the buttons are pressed. This also stops button lag from occuring while pressing
      them. Future plan is to run the Firebase stuff as a sub process in Python so it does not interupt the counting cycle, this update is dependant on
      the choice of hardware in the fugure for the next major version release. This is planned to be changed to follow strftime for update times and not system delays.
      - During an update cycle, both green buttons will flash back and forth as it runs through the read  write process and show any errors as described above.
      - Once it is finished, it goes back to counting and illuminating on button presses.


      Old Process (V1.0.0)
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
     
     - ****





# ----------------------------------------------------------------------------------------------------------
5                        How To Guides I Need To Create
# ----------------------------------------------------------------------------------------------------------

      - *Create Firebase Realtime Database and Link to ESP32 (Other board instructions will vary and I will only be covering the hardware I use.
      - *Add Firebase Initial Setup Guide
      - *Add Firebase Default Node Guide









#END

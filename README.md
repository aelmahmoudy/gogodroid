#GogoDroid - Android Gogoc client#

GogoDroid is a graphical front end of gogoc for Android. It intends to provide 
some user-friendly and powerful features such as config file editing, one-key 
start and stop, etc.

TSP is a control protocol used to establish and maintain static tunnels. The 
Gateway6 client (gogoc) is used on the host computer to connect to a tunnel 
broker using the TSP protocol and to get the information for its tunnel. When 
it receives the information for the tunnel, the Gateway6 client creates the 
static tunnel on its operating system.

This application requires a **ROOTED** device.

## Download ##

Binaries are available on [F-Droid] (https://f-droid.org/repository/browse/?fdid=com.googlecode.gogodroid)

## Building ##

Only 5 simple step to build this project.

### Requirements: ###

* Android NDK
* Android SDK

### How to build: ###

1. Change working directory to gogoc-android
2. Run "ndk-build"
3. run: android update project -p .
4. Build project (ant debug or ant release), Android package is in bin/
   directory

## License ##

This project is licensed under GNU Gernel Public License (V2).
Use it at your own risk!


## Credits ##
* [gogoc-android] (https://github.com/liudongmiao/gogoc-android/), core application
* GUI written by Mariotaku Lee <mariotaku.lee@gmail.com>, currently maintained by أحمد المحمودي
* [rsyncdroid] (http://code.google.com/p/rsyncdroid), GUI modded from this project
* [Droid VNC Server] (https://github.com/oNaiPs/droid-VNC-server/), Install binary file
* [DroidSSHd] (https://github.com/mestre/droidsshd/), Some references

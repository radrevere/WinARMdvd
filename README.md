# WinARMdvd
 Automated DVD ripping using makemkvcon for Windows
 
 I had a hard time getting ARM to work on ubuntu and there were complexities that I thought weren't needed.  Also it is a script specifically for Unix/Linux distros.  I wanted something for Windows since that is what I typically run at home (more because I like games than anything).  So this is a C++/Win32 application that automatically detects your DVD/CD drives on start up, will auto-launch makemkv when a disk is inserted and eject the disk when done...hence the autmation part.  I wrote the original code over one weekend centering specifically on DVD's so I could backup my own personal collection.  I would like to eventually expand it for music as well and enhance it to better handle tv and multiple movies on one disk.
 
 ## The App
 
 ![Untitled](https://user-images.githubusercontent.com/47608301/127780405-957022af-6ad3-4b93-b15d-b0890e594859.jpg)

So, no, it is not a background service.  That would be a nice to have but, especially at this stage, it is nicer to have the settings handy and watch what is going on.  You will notice that it lists all of the CD/DVD drives on the system.  This is automatically obtained through the Win32 API.  Currently, if you add a USB DVD drive you may need to re-start the application in order for it to be picked it up (I automatically add a Worker for the drive when a disc is inserted but I haven't tested whether it gets added to the UI properly).  Each of those sections is an output window for its respective drive. I capture stdout from makemkv, filter it for what I want and display it here.  I have a TODO for adding more information from stdout but will need more time to see what I can decipher that is useful.  I also have a TODO for logging this infomration.  For now it is progress and completion status.  The only other thing of note is the File->Options menu which opens the Setitngs Dialog.
 
 ## Settings
![image](https://user-images.githubusercontent.com/47608301/127779308-ddf513f3-f396-4003-8253-7f18936cbf46.png)

Be warned that I do not validate input at this time.  The only setting worth concern is the Min Length, please ensure it is a number.

**MakeMKV Root Path**
The root path to MakeMKV installation directory.  Only the folder is specified for security purposes as I don't want injection of other executables.

**Output Root Path**
The root path for the MKV output.  Each DVD will be RIPed into its own folder so it will be [rootpath]\\[DVD_TITLE] where DVD_TITLE is the title that is displayed for the drive when a disc is inserted.  This is done to allow for downloading the poster from OMDb.

**OMDb Key**
If you wish to automatically grab posters, set your OMDb key value here.  Please note that I am currently using the DVD_TITLE as the search title so many DVD's will not be found.(*any advice on how to get a better title would be much appreciated.  I am no stranger to reading binary formats so I am open to reading it from the disc*)

**Min length (sec)**
MakeMKV uses this to know which titles to exclude.  It is the smallest length to be included in seconds.  3600 is what they recommend for starters but TV shows may require a smaller value.

**Cache**
How much cache memory MakeMKV can use in MB.  I have a machine with a lot of RAM so I set it to 1024 and easily can RIP 4 DVD's at a time.  Although the drive write speed with 4 simultaneous RIPs can be a bottleneck.

**Use MakeMkvCon64**
Check if you want to use MakeMkvCon64.exe.  I had odd issues with this which is why I gave the option.  I currently prefer the non 64 bit version.

**Robot Mode**
MakeMKV highly recommends this be used when using the command line utility.  It is for outputing more information that is easier to parse.

**Decrypt**
Decrypt the output.  The default is to make a backup copy with all the DVD encryption in place.  This may be faster for RIPing but I am not sure.

**Direct Disc IO**
Forces direct disc access.  Uncheck will use the program preferences.

**Treat Multiples as Series**
If multiple MKV files are output, by default a "- part[number]" will be appended to the files.  If this is checked a "- s01e[number]" will be appended instead. The s01 or season part is currently hard coded to always be s01 as I currently am not reading the disc for information so it is a placeholder for you to edit.  [number] in both cases will be the number in the order they were RIPed. from the disc. (*again, if anyone knows where this information can be found on the disc I would love to know*)

**Eject When Done**
When checked, I attempt to eject the disc.  As noted above it works most of the time.  I have tried sending an eject flag to MakeMKVcon to no avail.  Any advice on improving this would be appreciated.

## Known Issues
For some reason Windows seems to take its time to release its hold on the drive.  Because of this, the eject command seems to be sporatic; meaning it works most of the time.  I setup a loop where I check each second for 10 seconds to see if I can get a hold of the drive to eject it.  I had it setup for 30 seconds and it didn't seem to make any difference.  The MakeMKVcon process has exited and it's thread and process handles are closed yet sometimes something just doesn't let go of the drive.  If anyone has any ideas on what I can do to fix this I would love to hear it.

MakeMKVcon doesn't seem to have the same options as the UI because it will fail to RIP some DVD's yet when I open the same disc in the UI it RIPs just fine.  I would appreciate any experts with MakeMKV console app to shed some light on this.  Perhaps I need more/differnt options?

The process should hover around 2-3 MB when no discs are RIPPING.  I RIPed a whole afternoon with it and it maintained this size.  If anyone notices any potential memory leaks, please let me know so I can resolve them.

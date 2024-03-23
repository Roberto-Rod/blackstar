* * VersaLogic VersaAPI Sample Code - 20211018
*
* This code is given with no guarantees and is intended to be used as an
* example of how to use the VersaAPI software package and to check the
* operation post install. This code will work with various VersaLogic
* single board computers, the VersaLogic VL-MPEe-A1/A2 and VL-MPEe-U2
* miniPCIe boards.
*
* Package:
*    README                    => This file.
*    VL_OSALib.dll             => VersaLogic VersaAPI dll library file, version 1.7.2  -Note 1.
*    VL_OSALib.lib             => VersaLogic VersaAPI lib library file, version 1.7.2  -Note 1.
*    versaAPI_U2_only_sample   => Compiled and linked example application              -Note 2.
*    versaAPI_U2_only_sample.c => Source code for the compiled application             -Note 2.
* Note 1. For this example, keep the library file in the same directory as the sample application.
* Note 2. If you need to recompile the application to get it to run, you can:
*             a. Save the current source file
*             b. Copy versaAPI_U2_only_sample.c to the original source file name
*             c. Compile as before
*
* Default Program Description: 
* This application will:
*    Report the VersaAPI Library version number
*    Report if the application is running on a VersaAPI or non-VersaAPI SBC
*    Report if there is an MPEe-U2E connected to the SBC
*    Try and toggle DIO_U2_CHANNEL_1 from LOW to HIGH, checking for the 
*        appropriate DIO level and reporting SUCCESS or FAILED as appropriate.
*
* This example application was compiled with:
*     Visual Studio 2017 (v141)
*     Windows SDK Version: 10.0.177763.0
*     Target Platform:     Windows 10
*     Configuration:       Debug
*     Platform:            x64
*
* This application was run on:
*   Edition: Windows 10 Pro
*   Version: 20H2
*   OS Build: 19042.1288
*   Experience: Windows Feature Experience Pack 120.2212.3920.0
* 
* Drivers Loaded:
*  Only default Windows 10 drivers were used (I believe, still checking).  See previous email for 
*  example of what drivers were loaded.
*  In Device Manager, for 'Multi-port serial adapters':
*      ExarMPIO-Access Device              - Properties -> Details -> Inf name = oem2.inf
*      Exar's 4-Port UART PCI-Express Card - Properties -> Details -> Inf name = oem10.inf
*  
*  In Device Manager, for 'Ports (COM & LPT)':
*      Should see four "Exar's Communications Port (COM10-13)"
* 
* Command Line Options: 
* Run versaAPI_U2_only_sample -h for a list of command line options.
* 
* Please run from a cmd window with non-admin priviledge and send me the output. If 
* there is still issues, use the -v option when running for additional debug output.
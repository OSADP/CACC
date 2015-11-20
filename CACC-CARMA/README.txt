Open Source Overview
============================
Connected Vehicle Research Platform (CVRP)
Version 1.0

Description
-----------
The CVRP software provides an API to each of the TFHRC's CARMA vehicles (modified 2013 Cadillac SRX).
Researchers can build project-specific control code on top of this API to study, for example, advanced
ACC and driver assist systems. The software in this repository comprises four components that execute on two processors.
The primary processor is a MicroAutobox running dSpace, that houses:
  * CARMA platform Real-time
  * PATH Cooperative Adaptive Cruise Control (CACC) control code
The CACC code is not strictly part of the CVRP, but was included in this project as a practical
application to exercise and demonstrate the function of the reusable CARMA platform. Therefore, it is
included in this repository.  
The secondary processor is a generic Ubuntu Linux PC that houses:
  * Data communication and extended driver interface
  * Log processor

The primary functions of these components are as follows.

CARMA real-time:  this code parses input data from the CANbus and secondary processor and places the
individual data elements into the data interface block, which can be read by the project-specific
application (e.g. the PATH CACC code). It also reads project-specific outputs from the data interface
block and packages them into properly formed messages and forwards them to the CANbus or secondary
processor, as necessary.

Data communication and extended driver interface:  is a web application that provides an extended
driver interface (beyond the stock vehicle dashboard interface). This DVI is a web page that can be
transmitted over wifi to a tablet mounted on the dashboard to provide experimental data feedback and
get commands from the driver. It also communicates with the non-CANbus devices on the vehicle: 
two-way messaging with the DSRC on-board unit to pass BSMs; TORC controller to receive vehicle speed
and to pass speed commands; and TORC Pinpoint location device to receive current vehicle position. 
Data from these devices is then reformatted as necessary and passed to the CARMA real-time component
via Ethernet messages.  Data from the CARMA real-time component (e.g. speed commands and BSMs) are
passed to these devices as appropriate.

Log processor:  is a Python program running on the vehicle’s secondary processor responsible for formatting logs from the CACC control software on the MAB. The CACC control software sends various control parameters to the Log Processor every timestep via a UDP socket. The Log Processor parses this binary-formatted message into a human-readable CSV format more suited for post-processing and analysis. These files are stored in a subfolder of the Log Processer program’s working directory and are stored in files with timestamped names.


Installation instructions
-------------------------
Microautobox:  this is a complicated build & install process.  It is described in the document "CACC Software Installation Instructions.docx" in the Carma-Mab/docs directory.

Secondary processor:  the data communication app runs on Java 1.7, which needs to be installed first.
All of its functionality and resources are packaged in a single jar file, which needs to be installed
in a directory named /opt/v2i.  It will also need a directory named /opt/v2i/logs.

log processor:  this is described in the same doc as the Microautobox installation.



License information
-------------------
See accompanying LICENSE file.


System Requirements
-------------------------
Microautobox: dSpace Microautobox II computer

Secondary processor:  
Minimum memory:  2 GB
Processing power:  Intel Core I3 @ 1.6 GHz or equivalent
Connectivity:  ethernet
Operating systems supported:  Ubuntu 14.04


Documentation
-------------
The user guide for the research platform is included with the software distribution.


Web sites
---------
This software is distributed through the USDOT's JPO Open Source Application Development Portal (OSADP) under the CACC-CARMA project.
http://itsforge.net/


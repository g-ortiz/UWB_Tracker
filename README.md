# UWB_Tracker

The growing number of applications in automated robots and vehicles has increased the demand for positioning, locating, and tracking systems. The majority of the current methods are based on machine vision systems and require a direct line of sight (LOS) between the tracking device and the target at all times for carrying out the desired functionalities. This limits the possible applications and makes them vulnerable to disturbances. The method presented in this thesis work aims to remove the continuous LOS requirement and allow for an omnidirectional and accurate tracking method using ultra-wideband (UWB) technology. This is achieved by using a flipped UWB positioning topology where a set of anchors keeps track of the position of a target and maintains a specific distance from it; this is in contrast to regular indoor positioning systems where a target monitors its own position in relation to a set of fixed references. The feasibility of this solution is shown by a tracking device prototype which demonstrates the capabilities of the proposed system and the UWB technology. The results show that the proposed topology is suitable for positioning, tracking and following applications that require a high degree of accuracy at short distances with the possibility of removing the continuous direct LOS requirement.

http://publications.lib.chalmers.se/records/fulltext/249921/249921.pdf 
https://ieeexplore.ieee.org/document/8250067

The code is written for Arduino M0 Pro on the tracker (Anchor) side and an Arduino Pro Mini 3.3V on the target side.

The communication with the DWM1000 from the arduino is based on the library created by thotro, with some modifications: https://github.com/thotro/arduino-dw1000

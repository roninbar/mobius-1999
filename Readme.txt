-------------------------------------------------------------------------------
Moebius Clock Screen Saver v 1.7 README.TXT                     November 1999
         Copyright (c) 1999-2000 by Uriel Cohen & Ron Inbar.
-------------------------------------------------------------------------------

I. INTRODUCTION
  Thank you for trying the Moebius Clock Screen Saver! This file contains 
  installation/removal instructions, requirements and a brief help section. 


II. CONTENTS
  1. INTRODUCTION
     A. THE CLOCK CONCEPT
	 B. THE SCREEN SAVER
  2. REQUIREMENTS
  3. INSTALLATION INSTRUCTIONS
  4. REMOVING INSTRUCTIONS
  5. CREDITS


1. INTRODUCTION

  A. THE CLOCK CONCEPT
    The Moebius Clock is an original clock concept designed to show the 24 hours
	with regular clock hands and without addins showing AM or PM time. Instead of
	a regular flat ring of numbers (the 12 numbers) it has a vertical Moebius
	strip with the 24 numbers on it.
	The Moebius strip is a circular strip which was twisted before being closed
	(before one extreme was connected with the other). If you follow one side of
	it with your finger along the strip you will finish where you started. Thus,
	we say that the Moebius strip has no inside or outside.
	The Moebius Clock has the 24 hours numbers arranged on a Moebius strip on such
	a way that numbers with 12 hours difference fall back to back (for example 2:00
	and 14:00; 2:00 AM and PM).
	The hours hand will turn normaly, and will show the hour where we expect it to
	point. The Moebius strip at that point will have the correct number (in the 24
	hours interval) facing the hours hand. How is this possible? Simple...the
	Moebius strip turns on itself while the hands are turning, so that it turns 180
	degrees in 12 hours. When the hours hand returns to this place, the strip will
	have the other number facing it (the number with 12 hours difference).

  B. THE SCREEN SAVER
    The Moebius Clock Screen Saver shows a fully functional Moebius Clock in 3D 
	in the middle of the screen, which shows the correct hour taken from your
	system settings.
	The screen saver has several options to control how the Moebius Clock will be
	rendered (quality and performance options), and other options that control the
	behavior of the clock. It even implements some other options compatible with
	Microsoft's Plus! screen savers (like Hot Spots and mouse sensitivity).
	The Moebius Clock Screen Saver was implemented in OpenGL and depends highly on
	the current OpenGL implementation installed in your computer (see Requirements
	and Installation instructions).

2. REQUIREMENTS
  1) Operating System. The Moebius Clock Screen Saver was fully tested in Windows
     NT 4, Windows 95 and Windows 98. It requires the OpenGL drivers/dlls installed
	 on your computer. Luckily, Windows NT 4 and Windows 98 come with a software 
	 implementation of those drivers. If you own Windows 95 you will have to
	 download and install the drivers/dlls since they are not included in this
	 package.
  2) Hardware requirements. The hardware requirements are simple: the faster your
     processor and the better your 3D acceleration card the best performance you
	 will get. It was fully tested with a Pentium II 350 Mhz and a Creative Riva TNT
	 3D acceleration card and found to run smoothly. It was executed also with
	 several other graphic cards and processor speeds. The thumb rule: you'll need
	 a reasonable good 3D accelerator with OpenGL implementation and support for
	 texture mapping in hardware.
	 Some settings have been added to improve the performance in non 3D accelerated
	 computers. With those options it was tested on a Pentium 266 Mhz with a regular
	 VGA card. Check every option with the question mark cursor in the settings 
	 dialog for an explanation.
	 We recommend a resolution of at least 800x600 with 16 bits color depth.

3. INSTALLATION INSTRUCTIONS
  The installation instructions are a bit more complicated than the usual since we
  didn't use an automatic installer with this version (we plan to use it in the
  next one). We apologize in advance for the inconvinience.
  1) To install the screen saver from this ZIP distribution, you must first 
     decompress it. Decompression requires a ZIP extraction program like PKUNZIP or
     WinZip.
     You should extract all files in the archive to a temporary directory (see
     the PKZIP or WinZip documentation for instructions). If the files were
     extracted succesfully you should see the following files in your temporary
     directory:
       Readme.txt          - this readme
       Moebius Clock.scr   - the screen saver executable
       Moebius Clock.hlp   - the screen saver context help

  2) After extracting all the files, you should copy the "Moebius Clock.scr" and the
     "Moebius Clock.hlp" files to your Windows System directory (usually
     Windows\System for Windows 95/98 and WinNT\System32 for NT 4).
  3) Then open the Display properties by double clicking on it's icon from the Control
     Panel. Press on the Screen Saver tab at the top of the dialog. On the screen
     saver section you should see a combo box with the name of the current screen
     saver displayed on it. Press on the combo box and scroll until you see "Moebius
     Clock". Select it. Close the dialog by pressing OK or Preview the screen saver
     before. It is recommended to configure the screen saver by pressing on Settings
     before you run it for the first time.

4. REMOVING INTRUCTIONS
  1) Select another screen saver by following the 3rd step of the previous section.
  2) Delete the "Moebius Clock.scr" and "Moebius Clock.hlp" files from your Windows
     System directory (see 2nd step of the previous section).
  3) If you like to clean up the registry entries that the screen saver added (for
     advanced Windows users only) open the Registry Editor (regedit). Delete all the
     subtree from the following registry path: 
     HKEY_CURRENT_USER\Software\Moebius Clock

5. CREDITS
  The original Moebius Clock concept was created by Ron Inbar. He also did the
  OpenGL implementation. The Win32 porting and documentation was done by
  Uriel Cohen. Thank you to all who supported us and gave us constructive criticism.

  Contact us at:
      Uriel Cohen  : urielc@actcom.co.il
      Ron Inbar    : rinbar@netvision.net.il

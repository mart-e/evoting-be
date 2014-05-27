                        +----------+
                        | SHOWLOGO |
                        +----------+

Purpose :
*********
To Visualize a bitmap and check the BMP-file according
the restrictions concerning the logo's to be used in the
Belgian Election software.
If the bitmap is too large - more than 128x26 pixels, 
but within the screen resolution of 640x480 pixels,
the bitmap is displayed and the area which is too large
is shown with a gray instead of a white background.

Syntax :
********
SHOWLOGO.EXE <filename>

<filename> should be provided WITHOUT EXTENTION
<filename> is limited to MAXIMUM 8 CHARACTERS

Remember SHOWLOGO.EXE is a MS-DOS program and 
that it is restricted to the 8.3 filename convention !
(8 characters for the filename and 3 characters for the extention)

Users guide :
*************
* Program and logo's should be in the same directory

* Do not double click the program, but open a dos window 
  (command for win9x, cmd for winnt/xp) in the directory
  where the program and logo's are located.

* Start the test using above explained syntax.

Workflow :
**********
* First the 'Steria Company logo' appears for three
  seconds (hit a key to speed up the process)

* A screen with some technical information is shown next.
  On the first line of this screen, the filename of your logo is shown.
  (notice the added extention .BMP)
  If a requirement is out of norm, it will be shown and commented.
  If the dimensions (WxH) exceeds its limits, an error 'BITMAP IS TOO LARGE'
  is given.
  Hit the C key to advance to the next screen.

* On the next screen your logo will appear.
  If the dimensions (WxH) exceeds its limits, a grey array is shown,
  indicating the part of the logo that is out of boundary indications.
  Hit the T key to toggle the WHITE and BLACK color of the logo.
  (The background color will not be toggle by hitting the T key !)
  Hit the Q key to quit the program.

Prerequisites of bitmap :
*************************
* Dimensions (WxH in pixels) : 128x26
* Bits per pixel............ : 1
* Compression............... : no compression (BI_RGB)
* Max colors used........... : 2


Assessment :
************
If no error is displayed and the logo appears
without a grey zone, the concerned logo is within
the requirements and valid for the electronic election
purposes.

_______________________________________
Copyright (C) Steria Benelux sa/nv 2003

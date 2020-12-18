
/*********************************************************************************************

    This is public domain software that was developed by or for the U.S. Naval Oceanographic
    Office and/or the U.S. Army Corps of Engineers.

    This is a work of the U.S. Government. In accordance with 17 USC 105, copyright protection
    is not available for any work of the U.S. Government.

    Neither the United States Government, nor any employees of the United States Government,
    nor the author, makes any warranty, express or implied, without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or assumes any liability or
    responsibility for the accuracy, completeness, or usefulness of any information,
    apparatus, product, or process disclosed, or represents that its use would not infringe
    privately-owned rights. Reference herein to any specific commercial products, process,
    or service by trade name, trademark, manufacturer, or otherwise, does not necessarily
    constitute or imply its endorsement, recommendation, or favoring by the United States
    Government. The views and opinions of authors expressed herein do not necessarily state
    or reflect those of the United States Government, and shall not be used for advertising
    or product endorsement purposes.
*********************************************************************************************/


/****************************************  IMPORTANT NOTE  **********************************

    Comments in this file that start with / * ! or / / ! are being used by Doxygen to
    document the software.  Dashes in these comment blocks are used to create bullet lists.
    The lack of blank lines after a block of dash preceeded comments means that the next
    block of dash preceeded comments is a new, indented bullet list.  I've tried to keep the
    Doxygen formatting to a minimum but there are some other items (like <br> and <pre>)
    that need to be left alone.  If you see a comment that starts with / * ! or / / ! and
    there is something that looks a bit weird it is probably due to some arcane Doxygen
    syntax.  Be very careful modifying blocks of Doxygen comments.

*****************************************  IMPORTANT NOTE  **********************************/




#ifndef VERSION

#define     VERSION     "PFM Software - chartsPic V5.12 - 10/23/17"

#endif

/*

    Version 1.0
    Jan C. Depner
    09/09/04

    First working version.


    Version 1.1
    Jan C. Depner
    10/13/04

    Add system 2 roll, pitch, heading parameters.  Check for sbet file
    prior to checking for pos file.  Fix resize problem.


    Version 1.2
    Jan C. Depner
    10/29/04

    Added Linux only display of rotated image (separate window, no cursor
    tracking, requires ImageMagick);


    Version 1.3
    Jan C. Depner
    04/01/05

    Added --actionkey00 and --actionkey01 options (from pfmEdit).  --actionkey01
    will generate a rotated picture when the associated action key is pressed
    in pfmEdit.  Also, now shows NULL value photos.


    Version 1.31
    Jan C. Depner
    04/19/05

    Changed all of the QVBox's to QVBoxLayout's.  This allows the dialogs to auto
    size correctly.  I'm learning ;-)


    Version 1.32
    Jan C. Depner
    05/05/05

    Finally got all of the utility library functions working in C++.  Happy Cinco De Mayo!


    Version 1.4
    Jan C. Depner
    05/18/05

    Modified so that it could be started geoSwath (--geoSwath PPID argument) as well as PFM_ABE.


    Version 1.5
    Jan C. Depner
    05/23/05

    Added Brighten and Darken action keys.  Added Freeze toggle button.


    Version 2.0
    Jan C. Depner
    12/29/05

    Now able to display pictures rotated to North up.


    Version 2.1
    Jan C. Depner
    01/03/06

    No longer making temporary jpg file to store image.  Using Qt::Pixmap.loadFromData.  Happy 28 freaking
    years at NAVO.  Sheesh, I'm old ;-)


    Version 2.2
    Jan C. Depner
    03/07/06

    Added option for saving the snippet using file name sent in ABE_SHARE from pfmEdit.


    Version 3.0
    Jan C. Depner
    04/06/06

    Massive Qt changes - new cursors, left click to finish operations, right click menu in some modes, fixed
    stretch, replaced QVBox, QHBox and others with layouts.


    Version 3.01
    Jan C. Depner
    06/14/06

    Saves heading to ABE_SHARE when dumping a target snippet.


    Version 3.02
    Jan C. Depner
    09/08/06

    Switched to overrideCursor for wait state.


    Version 3.03
    Jan C. Depner
    09/12/06

    Check zoom_level in pic.cpp so it doesn't try to grab tons of memory.


    Version 3.04
    Jan C. Depner
    09/29/06

    Make default filename for saving scaled picture the dtg and position of picture.


    Version 3.05
    Jan C. Depner
    01/25/07

    Try to open upper case SBET file if lower case isn't found.  GCS stupidly changes
    case on the SBET file when it mods it.  DOH!


    Version 4.0
    Jan C. Depner
    04/11/07

    Qt 4 port.


    Version 4.01
    Jan C. Depner
    08/24/07

    Switched from setGeometry to resize and move.  See Qt4 X11 window geometry documentation.


    Version 4.02
    Jan C. Depner
    09/05/07

    Make sure picture is redrawn each time the picture changes (heading problem).


    Version 4.03
    Jan C. Depner
    10/05/07

    Change the timer to check every 250 milliseconds.  This really takes the load off of the processor
    and it's still just as responsive.


    Version 4.04
    Jan C. Depner
    10/15/07

    Set the shared memory key to 0 after each redraw so that the caller will know it has been started.


    Version 4.05
    Jan C. Depner
    11/01/07

    Added "stickpins".


    Version 4.06
    Jan C. Depner
    12/07/07

    Got rid of annoying QMessageBox error messages and moved them to the status bar.  Tora, tora, tora!


    Version 4.07
    Jan C. Depner
    01/04/08

    Now uses the parent process ID of the bin viewer plus _pfm or _abe for the shared memory ID's.  This removes the single instance
    per user restriction from ABE.


    Version 4.08
    Jan C. Depner
    04/01/08

    Added acknowledgments to the Help pulldown menu.


    Version 4.09
    Jan C. Depner
    04/07/08

    Replaced single .h files from utility library with include of nvutility.h


    Version 4.10
    Jan C. Depner
    07/15/08

    Removed pfmShare shared memory usage and replaced with abeShare.


    Version 4.11
    Jan C. Depner
    10/30/08

    Mods to support new waveMonitor changes to ABE.h.


    Version 4.12
    Jan C. Depner
    11/28/08

    Added kill_switch option.


    Version 4.20
    Jan C. Depner
    03/13/09

    Added ability to handle WLF data.  Friday the 13th - Oh No!


    Version 4.21
    Jan C. Depner
    04/13/09

    Use NINT instead of typecasting to NV_INT32 when saving Qt window state.  Integer truncation was inconsistent on Windows.


    Version 4.22
    Jan C. Depner
    05/21/09

    Set all QFileDialogs to use List mode instead of Detail mode.


    Version 4.23
    Jan C. Depner
    06/11/09

    Added support for PFM_CHARTS_HOF_DATA.


    Version 4.24
    Jan C. Depner
    06/15/09

    Use WLF sensor position and attitude data instead of POS or SBET if it is present in the WLF record.


    Version 4.25
    Jan C. Depner
    09/16/09

    Set killed flag in abe_share when program exits other than from kill switch from parent.


    Version 4.26
    Jan C. Depner
    04/29/10

    Fixed posfix and fixpos calls to use numeric constants instead of strings for type.


    Version 4.27
    Jan C. Depner
    01/06/11

    Correct problem with the way I was instantiating the main widget.  This caused an intermittent error on Windows7/XP.


    Version 4.28
    Jan C. Depner
    11/30/11

    Converted .xpm icons to .png icons.


    Version 5.00
    Jan C. Depner (PFM Software)
    08/20/13

    Added suport for CZMIL images.


    Version 5.01
    Jan C. Depner (PFM Software)
    12/09/13

    Switched to using .ini file in $HOME (Linux) or $USERPROFILE (Windows) in the ABE.config directory.  Now
    the applications qsettings will not end up in unknown places like ~/.config/navo.navy.mil/blah_blah_blah on
    Linux or, in the registry (shudder) on Windows.


    Version 5.02
    Jan C. Depner (PFM Software)
    03/17/14

    Removed WLF support.  Top o' the mornin' to ye!


    Version 5.03
    Jan C. Depner (PFM Software)
    03/19/14

    - Straightened up the Open Source acknowledgments.


    Version 5.04
    Jan C. Depner (PFM Software)
    05/27/14

    - Added the new LGPL licensed GSF library to the acknowledgments.


    Version 5.05
    Jan C. Depner (PFM Software)
    07/05/14

    - Had to change the argument order in pj_init_plus for the UTM projection.  Newer versions of
      proj4 appear to be very sensitive to this.


    Version 5.06
    Jan C. Depner (PFM Software)
    07/23/14

    - Switched from using the old NV_INT64 and NV_U_INT32 type definitions to the C99 standard stdint.h and
      inttypes.h sized data types (e.g. int64_t and uint32_t).


    Version 5.07
    Jan C. Depner (PFM Software)
    02/16/15

    - To give better feedback to shelling programs in the case of errors I've added the program name to all
      output to stderr.


    Version 5.08
    Jan C. Depner (PFM Software)
    06/27/15

    - Fixed PROJ4 init problem.


    Version 5.09
    Jan C. Depner (PFM Software)
    08/08/16

    - Now gets font from globalABE.ini as set in Preferences by pfmView.
    - To avoid possible confusion, removed translation setup from QSettings in env_in_out.


    Version 5.10
    J. Heath Harwood (USACE)
    10/31/16

    - Added ability to read RCD30 Camera file while retaining CM4800 (CZMIL) camera readability/functionality
    - I have made fire!...I...have...made...fire...


    Version 5.11
    Jan Depner (PFM Software)
    12/18/16

    - Added hot key to open orthorectified images for CZMIL data.
    - Added zoom capability for orthorectified images.


    Version 5.12
    Jan C. Depner (PFM Software)
    10/23/17

    - A bunch of changes to support doing translations in the future.  There is a generic
      chartsPic_xx.ts file that can be run through Qt's "linguist" to translate to another language.

*/

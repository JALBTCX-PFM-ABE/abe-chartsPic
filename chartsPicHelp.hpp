
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



QString rotateText = 
  chartsPic::tr ("<img source=\":/icons/rotate.png\"> Click this button to rotate the pictures to north up.  In most cases this "
                 "button is only used as an indicator of the <b>rotation</b> status.  Rotating the display is usually "
                 "done from within <b>pfmEdit3D</b> or <b>fileEdit3D</b> by pressing the <b>rotate</b> action key.  By "
                 "default this is the <b><i>r</i></b> key.  Look in <b>pfmEdit3D</b> or <b>fileEdit3D</b> under "
                 "<b>Preferences->Ancillary Programs</b> to see what the key is actually set to.  The default "
                 "list of keys for chartsPic is as follows:"
                 "<ul>"
                 "<li>r - display the picture in north-up rotated view</li>"
                 "<li>t - save the current picture to a file indicated in ABE_SHARE memory (only used in pfmEdit3D)</li>"
                 "</ul>");

QString brightenText = 
  chartsPic::tr ("Click this button to brighten the picture.");

QString darkenText = 
  chartsPic::tr ("Click this button to darken the picture.");

QString orthoText = 
  chartsPic::tr ("<img source=\":/icons/ortho.png\"> Click this button to display the high-resolution, orthorectified image from which current "
                 "image was derived.  When you display the orthorectified image the cursor will become the zoom cursor <img source=\":/icons/zoom_cursor.png\"> "
                 "and you can zoom in to an area of the image.  Click the left mouse button to define a starting point for a "
                 "rectangle.  Move the mouse to define the zoom in bounds.  Left click again to finish the operation.  To "
                 "abort the zoom in operation click the middle mouse button.<br><br>"
                 "<b>IMPORTANT NOTE: This button will only work if you are viewing CZMIL data, the <i>camera_single_frames_orthomosaics</i> folder is "
                 "present in the <i>DC_</i> image folder, and the <i>C...._ortho_mosaic_geo.tif</i> file associated with the current image is "
                 "present in the folder.</b>");

QString zoomOutText = 
  chartsPic::tr ("<img source=\":/icons/icon_zoomout.png\"> Click this button to zoom out one level.<br><br>"
                 "<b>IMPORTANT NOTE: This button is only enabled if you are displaying an orthorectified image.</b>");

QString quitText = 
  chartsPic::tr ("<img source=\":/icons/quit.png\"> Click this button to <b><em>exit</em></b> from the program.  "
                 "You can also use the <b>Quit</b> entry in the <b>File</b> menu or the optional quit key "
                 "that may have been passed from <b>pfmEdit3D</b> or <b>fileEdit3D</b>.");

QString saveText = 
  chartsPic::tr ("<img source=\":/icons/fileopen.png\"> Click this button to save the picture at it's original size "
                 "to a file.");

QString scaledText = 
  chartsPic::tr ("<img source=\":/icons/scaled.png\"> Click this button to save the picture at it's scaled/rotated "
                 "(WYSIWIG) size to a file.");

QString prefsText = 
  chartsPic::tr ("<img source=\":/icons/prefs.png\"> Click this button to change program preferences.  Right now "
                 "that's just the position format.  Maybe more later...");

QString picText = 
  chartsPic::tr ("This is the ChartsPic program, a companion to the <b>pfmEdit3D</b> and/or <b>fileEdit3D</b> program.  "
                 "It is used for viewing CHARTS and/or CZMIL down-looking digital camera pictures.  This program is never run as "
                 "a stand-alone program but is <i><b>shelled</b></i> from other programs such as <b>pfmEdit3D</b> or "
                 "<b>fileEdit3D</b>.<br><br>"
                 "You may place <b><i>stickpins</i></b> in the picture to mark locations by left clicking in the "
                 "picture.  These will be thrown away whenever you change images.  You can save the "
                 "<b><i>stickpins</i></b> in the picture by saving the scaled picture or using the shared memory "
                 "hotkey described below.<br><br>"
                 "The box cursor will track the movement of the cursor in the <b>pfmEdit3D</b> or <b>fileEdit3D</b> window "
                 "as well as possible.  The error in position will generally be on the order of 5 meters at the center "
                 "of the picture and somewhat more toward the outer edges.  A couple of the functions/buttons have remote "
                 "action keys defined in <b>fileEdit3D</b> and/or <b>pfmEdit3D</b>.  By default these are:"
                 "<ul>"
                 "<li>r - display the picture in north-up rotated view</li>"
                 "<li>t - save the current picture to a file indicated in ABE_SHARE memory</li>"
                 "<li>h - try to display the associated orthorectified image for the current picture (CZMIL only)</li>"
                 "</ul>"
                 "The options need to be listed in the above order in the <b>Preferences-Ancillary Programs</b> "
                 "dialog of <b>fileEdit3D</b> or <b>pfmEdit3D</b>.  For instance - r,t,h.");

QString bGrpText = 
  chartsPic::tr ("Select the format in which you want all geographic positions to be displayed.");

QString closePrefsText = 
  chartsPic::tr ("Click this button to close the preferences dialog.");

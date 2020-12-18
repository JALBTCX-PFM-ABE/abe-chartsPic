
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



/*  ChartPic class definitions.  */

#ifndef CHARTSPIC_H
#define CHARTSPIC_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <getopt.h>
#include <cerrno>
#include <cmath>

#include "nvutility.h"
#include "nvutility.hpp"

#include <gdal.h>
#include <gdal_priv.h>
#include <cpl_string.h>
#include <ogr_spatialref.h>
#include <proj_api.h>

#include "pfm.h"
#include "pfm_extras.h"
#include "FileHydroOutput.h"
#include "FileTopoOutput.h"
#include "FilePOSOutput.h"
#include "FileImage.h"
#include "czmil.h"
#include "version.hpp"


#include <QtCore>
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <QSharedMemory>

#include "basename.h"


#define MAX_ZOOM 100


typedef struct
{
  double         x;
  double         y;
  double         lat;
  double         lon;
  double         altitude;
  int32_t        zone;
  char           hemi[2];
  double         roll;
  double         pitch;
  double         heading;
} AIRCRAFT;

typedef struct
{
  double         x_step;
  double         y_step;
  int32_t        width;
  int32_t        height;
  double         min_x;
  double         max_y;
  int32_t        zone;
  char           hemi[20];
  int32_t        x_pix;
  int32_t        y_pix;
  double         center_x;
  double         center_y;
  double         center_lat;
  double         center_lon;
  double         point_x;
  double         point_y;
  AIRCRAFT       aircraft;
} LOCAL_JPW;


typedef struct
{
  double      y_step;
  double      x_step;
  NV_F64_XYMBR geotiff_area[MAX_ZOOM];    //  Bounds of the GeoTIFF file
  NV_I32_COORD2 geotiff_size[MAX_ZOOM];   //  Width (x) and height (y) of scaled pixmap
  int32_t     zoom_level;
  int32_t     mosaic_width;
  int32_t     mosaic_height;
  projPJ      pj_utm;
  projPJ      pj_latlon;
  QImage      *full_res_image;
  QString     projRef;
  QSharedMemory *abeShare;                //  ABE's shared memory pointer.
  ABE_SHARE   *abe_share;                 //  Pointer to the ABE_SHARE structure in shared memory.
  char        progname[256];              /*  This is the program name.  It will be used in all output to stderr so that shelling programs
                                              will know what program printed out the error message.  */
  FILE        *dfp;                       //  Debug file pointer (may not be used).
} MISC;


class chartsPic:public QMainWindow
{
  Q_OBJECT


public:

  chartsPic (int32_t *argc = 0, char **argv = 0, QWidget *parent = 0);
  ~chartsPic ();

  void redrawPic ();

protected:

  int32_t         menu_cursor_x, menu_cursor_y, key, stickpin_count, active_window_id, window_x, window_y, width, height, pos_format;

  uint32_t        kill_switch, ac[6];

  int64_t         image_time;

  int8_t          dummy;

  uint8_t         rotation, save_rotation, startup_message, ortho, ortho_displayed, zooming;

  char            prev_file[512], tmp_file[512], pf[10], pos_file[512], jpw_file[1024], rot_file[512];

  double          x_fov[5], y_fov[5], pitch_bias[5], roll_bias[5], a0, b0, rad_to_deg, heading, center_lat, center_lon;

  MISC            misc;

  QFont           font;

  NV_I32_COORD2   stickpin[100];

  ABE_SHARE       l_share;

  QPainterPath    marker;

  QMessageBox     *filError;

  QCheckBox       *sMessage;

  QMouseEvent     *menu_mouse_event;

  nvPic           *pic;

  QScrollBar      *vBar, *hBar;

  QString         save_directory, snippet_file_name, parentName, save_file_name, imgDir;

  NVPIC_DEF       picDef;

  QButtonGroup    *bGrp;

  QDialog         *prefsD;

  QToolButton     *bQuit, *bSave, *bScaled, *bBrighten, *bDarken, *bPrefs, *bOrtho, *bZoomOut, *bRotate;

  QCursor         zoomCursor, stickpinCursor;

  QPixmap         *stickPin;

  QTimer          *trackCursor;

  LOCAL_JPW       czmil_jpw;


  void redraw_ortho ();
  void picSave (uint8_t scaled);
  void envin ();
  void envout ();


protected slots:

  void slotMousePress (QMouseEvent *e, int32_t x, int32_t y);
  void slotMouseRelease (QMouseEvent *e, int32_t x, int32_t y);
  void slotMouseMove (QMouseEvent *e, int32_t x, int32_t y);
  void slotPostRedraw (NVPIC_DEF picdef);
  void slotResize (QResizeEvent *e);
  void slotKeyPress (QKeyEvent *e);
  void closeEvent (QCloseEvent *event);

  void slotTrackCursor ();

  void midMouse (QMouseEvent *e, int32_t x, int32_t y);
  void leftMouse (QMouseEvent *e, int32_t x, int32_t y);
  void rightMouse (QMouseEvent *e, int32_t x, int32_t y);

  void slotQuit ();
  void slotSave ();
  void slotScaled ();
  void slotBrighten ();
  void slotDarken ();
  void slotOrtho ();
  void slotZoomOut ();

  void slotPrefs ();
  void slotPosClicked (int id);
  void slotClosePrefs ();
  void slotHelp ();

  void slotRotate ();

  void about ();
  void slotAcknowledgments ();
  void aboutQt ();


 private:
};

#endif

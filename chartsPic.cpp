
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



#include "chartsPic.hpp"
#include "chartsPicHelp.hpp"
#include "acknowledgments.hpp"


static double settings_version = 2.0;


chartsPic::chartsPic (int32_t *argc, char **argv, QWidget * parent):
  QMainWindow (parent, 0)
{
  extern char     *optarg;

  //misc.dfp = fopen ("fred.txt", "w");

  strcpy (misc.progname, argv[0]);
  filError = NULL;
  stickpin_count = 0;
  active_window_id = getpid ();


  QResource::registerResource ("/icons.rcc");


  stickPin = new QPixmap (":/icons/stickpin.png");


  //  Have to set the focus policy or keypress events don't work properly at first in Focus Follows Mouse mode

  setFocusPolicy (Qt::WheelFocus);


  //  Make the "marker" cursor painter path.

  marker = QPainterPath ();

  marker.moveTo (0, 0);
  marker.lineTo (30, 0);
  marker.lineTo (30, 20);
  marker.lineTo (0, 20);
  marker.lineTo (0, 0);

  marker.moveTo (0, 10);
  marker.lineTo (12, 10);

  marker.moveTo (30, 10);
  marker.lineTo (18, 10);

  marker.moveTo (15, 0);
  marker.lineTo (15, 6);

  marker.moveTo (15, 20);
  marker.lineTo (15, 14);


  //  Set the main icon

  setWindowIcon (QIcon (":/icons/charts_pic.png"));


  zoomCursor = QCursor (QPixmap (":/icons/zoom_cursor.png"), 11, 11);
  stickpinCursor = QCursor (QPixmap (":/icons/stickpin_cursor.png"), 9, 31);


  int32_t option_index = 0;
  misc.abe_share = NULL;
  kill_switch = ANCILLARY_FORCE_EXIT;

  while (NVTrue)
    {
      static struct option long_options[] = {{"actionkey00", required_argument, 0, 0},
                                             {"actionkey01", required_argument, 0, 0},
                                             {"actionkey02", required_argument, 0, 0},
                                             {"shared_memory_key", required_argument, 0, 0},
                                             {"kill_switch", required_argument, 0, 0},
                                             {0, no_argument, 0, 0}};

      char c = (char) getopt_long (*argc, argv, "o", long_options, &option_index);
      if (c == -1) break;


      QString skey;


      switch (c)
        {
        case 0:

          switch (option_index)
            {
            case 3:

              /******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY **************************************** \

                  This is a little note about the use of shared memory within the Area-Based Editor (ABE) programs.  If you read
                  the Qt documentation (or anyone else's documentation) about the use of shared memory they will say "Dear [insert
                  name of omnipotent being of your choice here], whatever you do, always lock shared memory when you use it!".
                  The reason they say this is that access to shared memory is not atomic.  That is, reading shared memory and then
                  writing to it is not a single operation.  An example of why this might be important - two programs are running,
                  the first checks a value in shared memory, sees that it is a zero.  The second program checks the same location
                  and sees that it is a zero.  These two programs have different actions they must perform depending on the value
                  of that particular location in shared memory.  Now the first program writes a one to that location which was
                  supposed to tell the second program to do something but the second program thinks it's a zero.  The second program
                  doesn't do what it's supposed to do and it writes a two to that location.  The two will tell the first program 
                  to do something.  Obviously this could be a problem.  In real life, this almost never occurs.  Also, if you write
                  your program properly you can make sure this doesn't happen.  In ABE we almost never lock shared memory because
                  something much worse than two programs getting out of sync can occur.  If we start a program and it locks shared
                  memory and then dies, all the other programs will be locked up.  When you look through the ABE code you'll see
                  that we very rarely lock shared memory, and then only for very short periods of time.  This is by design.

              \******************************************* IMPORTANT NOTE ABOUT SHARED MEMORY ****************************************/


              //  Get the ABE shared memory area.

              sscanf (optarg, "%d", &key);

              skey.sprintf ("%d_abe", key);

              misc.abeShare = new QSharedMemory (skey);

              if (misc.abeShare->attach (QSharedMemory::ReadWrite)) misc.abe_share = (ABE_SHARE *) misc.abeShare->data ();
              break;

            case 4:

              //  Get the kill switch.

              sscanf (optarg, "%d", &kill_switch);
              break;

            default:
              char tmp;
              sscanf (optarg, "%1c", &tmp);
              ac[option_index] = (uint32_t) tmp;
              break;
            }
          break;
        }
    }


  ortho = NVFalse;
  ortho_displayed = NVFalse;
  zooming = NVFalse;
  misc.zoom_level = 0;
  dummy = 0;


  //  Make sure we got the shared memory (this program can only run as a shell from a parent).

  if (misc.abe_share == NULL)
    {
      fprintf (stderr, "%s %s %s %d - abe_share - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, strerror (errno));
      exit (-1);
    }


  //  This is the "tools" toolbar.  We have to do this here so that we can restore the toolbar location(s).

  QToolBar *tools = addToolBar (tr ("Tools"));
  tools->setObjectName (tr ("chartsPic main toolbar"));


  envin ();


  save_rotation = rotation;


  // Set the application font

  QApplication::setFont (font);


  setWindowTitle (QString (VERSION));


  //  Set the window size and location from the defaults

  this->resize (width, height);
  this->move (window_x, window_y);


  //  Camera system biases at present (we only know systems 2 and 3 and they change ;-).  I've stuffed the CZMIL camera
  //  data into [3] for now.

  x_fov[0] = 42.0 / NV_RAD_TO_DEG;
  x_fov[1] = 42.0 / NV_RAD_TO_DEG;
  x_fov[2] = 42.0 / NV_RAD_TO_DEG;
  x_fov[3] = 40.0 / NV_RAD_TO_DEG;
  x_fov[4] = 56.5 / NV_RAD_TO_DEG;
  y_fov[0] = 28.0 / NV_RAD_TO_DEG;
  y_fov[1] = 28.0 / NV_RAD_TO_DEG;
  y_fov[2] = 28.0 / NV_RAD_TO_DEG;
  y_fov[3] = 27.0 / NV_RAD_TO_DEG;
  y_fov[4] = 43.8 / NV_RAD_TO_DEG;
  roll_bias[0] = -0.2 / NV_RAD_TO_DEG;
  roll_bias[1] = -0.7 / NV_RAD_TO_DEG;
  roll_bias[2] = -0.2 / NV_RAD_TO_DEG;
  roll_bias[3] = 0.0 / NV_RAD_TO_DEG;
  roll_bias[4] = 0.0 / NV_RAD_TO_DEG;
  pitch_bias[0] = 11.3 / NV_RAD_TO_DEG;
  pitch_bias[1] = 10.72 / NV_RAD_TO_DEG;
  pitch_bias[2] = 11.3 / NV_RAD_TO_DEG;
  pitch_bias[3] = 0.0 / NV_RAD_TO_DEG;
  pitch_bias[4] = 8.5 / NV_RAD_TO_DEG;

  a0 = 6378137.0;
  b0 = 6356752.314245;


  //  Set the tracking timer to every 100 milliseconds.

  trackCursor = new QTimer (this);
  connect (trackCursor, SIGNAL (timeout ()), this, SLOT (slotTrackCursor ()));
  trackCursor->start (100);


  statusBar ()->setSizeGripEnabled (false);
  statusBar ()->showMessage (VERSION);


  QFrame *frame = new QFrame (this, 0);

  setCentralWidget (frame);


  //  Make the pic.

  picDef.transMode = Qt::FastTransformation;
  pic = new nvPic (this, &picDef);
  pic->setWhatsThis (picText);


  strcpy (prev_file, "NEW");


  //  Connect to the signals from the pic class.

  connect (pic, SIGNAL (mousePressSignal (QMouseEvent *, int32_t, int32_t)), this, SLOT (slotMousePress (QMouseEvent *, int32_t, int32_t)));
  connect (pic, SIGNAL (mouseReleaseSignal (QMouseEvent *, int32_t, int32_t)), this, SLOT (slotMouseRelease (QMouseEvent *, int32_t, int32_t)));
  connect (pic, SIGNAL (mouseMoveSignal (QMouseEvent *, int32_t, int32_t)), this, SLOT (slotMouseMove (QMouseEvent *, int32_t, int32_t)));
  connect (pic, SIGNAL (postRedrawSignal (NVPIC_DEF)), this, SLOT (slotPostRedraw (NVPIC_DEF)));
  connect (pic, SIGNAL (resizeSignal (QResizeEvent *)), this, SLOT (slotResize (QResizeEvent *)));
  connect (pic, SIGNAL (keyPressSignal (QKeyEvent *)), this, SLOT (slotKeyPress (QKeyEvent *)));


  //  Layouts, what fun!

  QVBoxLayout *vBox = new QVBoxLayout (frame);


  vBox->addWidget (pic);


  //  Button, button, who's got the buttons?

  bQuit = new QToolButton (this);
  bQuit->setIcon (QIcon (":/icons/quit.png"));
  bQuit->setToolTip (tr ("Quit"));
  bQuit->setWhatsThis (quitText);
  connect (bQuit, SIGNAL (clicked ()), this, SLOT (slotQuit ()));
  tools->addWidget (bQuit);


  bSave = new QToolButton (this);
  bSave->setIcon (QIcon (":/icons/fileopen.png"));
  bSave->setToolTip (tr ("Save original size picture"));
  bSave->setWhatsThis (saveText);
  connect (bSave, SIGNAL (clicked ()), this, SLOT (slotSave ()));
  tools->addWidget (bSave);


  bScaled = new QToolButton (this);
  bScaled->setIcon (QIcon (":/icons/scaled.png"));
  bScaled->setShortcut ((char) ac[1]);
  bScaled->setToolTip (tr ("Save scaled picture [%1]").arg ((char) ac[1]));
  bScaled->setWhatsThis (scaledText);
  connect (bScaled, SIGNAL (clicked ()), this, SLOT (slotScaled ()));
  tools->addWidget (bScaled);


  bRotate = new QToolButton (this);
  bRotate->setIcon (QIcon (":/icons/rotate.png"));
  bRotate->setShortcut ((char) ac[0]);
  bRotate->setToolTip (tr ("Rotate picture to <b>north up</b> [%1]").arg ((char) ac[0]));
  bRotate->setWhatsThis (rotateText);
  bRotate->setCheckable (true);
  bRotate->setChecked (rotation);
  connect (bRotate, SIGNAL (clicked ()), this, SLOT (slotRotate ()));
  tools->addWidget (bRotate);


  tools->addSeparator ();
  tools->addSeparator ();


  bBrighten = new QToolButton (this);
  bBrighten->setIcon (QIcon (":/icons/brighten.png"));
  bBrighten->setToolTip (tr ("Brighten the picture"));
  bBrighten->setWhatsThis (brightenText);
  connect (bBrighten, SIGNAL (clicked ()), this, SLOT (slotBrighten ()));
  tools->addWidget (bBrighten);


  bDarken = new QToolButton (this);
  bDarken->setIcon (QIcon (":/icons/darken.png"));
  bDarken->setToolTip (tr ("Darken the picture"));
  bDarken->setWhatsThis (darkenText);
  connect (bDarken, SIGNAL (clicked ()), this, SLOT (slotDarken ()));
  tools->addWidget (bDarken);


  tools->addSeparator ();
  tools->addSeparator ();


  bOrtho = new QToolButton (this);
  bOrtho->setIcon (QIcon (":/icons/ortho.png"));
  bOrtho->setShortcut ((char) ac[2]);
  bOrtho->setToolTip (tr ("Display orthorectified image [%1]").arg ((char) ac[2]));
  bOrtho->setWhatsThis (orthoText);
  connect (bOrtho, SIGNAL (clicked ()), this, SLOT (slotOrtho ()));
  tools->addWidget (bOrtho);


  bZoomOut = new QToolButton (this);
  bZoomOut->setIcon (QIcon (":/icons/icon_zoomout.png"));
  bZoomOut->setToolTip (tr ("Zoom out"));
  bZoomOut->setWhatsThis (zoomOutText);
  connect (bZoomOut, SIGNAL (clicked ()), this, SLOT (slotZoomOut ()));
  tools->addWidget (bZoomOut);

  bZoomOut->setEnabled (false);


  tools->addSeparator ();
  tools->addSeparator ();


  bPrefs = new QToolButton (this);
  bPrefs->setIcon (QIcon (":/icons/prefs.png"));
  bPrefs->setToolTip (tr ("Change application preferences"));
  bPrefs->setWhatsThis (prefsText);
  connect (bPrefs, SIGNAL (clicked ()), this, SLOT (slotPrefs ()));
  tools->addWidget (bPrefs);


  QAction *bHelp = QWhatsThis::createAction (this);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  tools->addAction (bHelp);


  //  Setup the file menu.

  QAction *fileQuitAction = new QAction (tr ("&Quit"), this);
  fileQuitAction->setShortcut (tr ("Ctrl+Q"));
  fileQuitAction->setStatusTip (tr ("Exit from application"));
  connect (fileQuitAction, SIGNAL (triggered ()), this, SLOT (slotQuit ()));

  QAction *fileSaveAction = new QAction (tr ("&Save As"), this);
  fileSaveAction->setShortcut (tr ("Ctrl+S"));
  fileSaveAction->setStatusTip (tr ("Save file"));
  connect (fileSaveAction, SIGNAL (triggered ()), this, SLOT (slotSave ()));

  QMenu *fileMenu = menuBar ()->addMenu (tr ("&File"));
  fileMenu->addAction (fileQuitAction);
  fileMenu->addSeparator ();
  fileMenu->addAction (fileSaveAction);


  //  Setup the help menu.

  QAction *aboutAct = new QAction (tr ("&About"), this);
  aboutAct->setShortcut (tr ("Ctrl+A"));
  aboutAct->setStatusTip (tr ("Information about chartsPic"));
  connect (aboutAct, SIGNAL (triggered ()), this, SLOT (about ()));

  QAction *acknowledgments = new QAction (tr ("A&cknowledgments"), this);
  acknowledgments->setShortcut (tr ("Ctrl+c"));
  acknowledgments->setStatusTip (tr ("Information about supporting libraries"));
  connect (acknowledgments, SIGNAL (triggered ()), this, SLOT (slotAcknowledgments ()));

  QAction *aboutQtAct = new QAction (tr ("About Qt"), this);
  aboutQtAct->setStatusTip (tr ("Information about Qt"));
  connect (aboutQtAct, SIGNAL (triggered ()), this, SLOT (aboutQt ()));

  QMenu *helpMenu = menuBar ()->addMenu (tr ("&Help"));
  helpMenu->addAction (aboutAct);
  helpMenu->addSeparator ();
  helpMenu->addAction (acknowledgments);
  helpMenu->addAction (aboutQtAct);


  pic->setCursor (stickpinCursor);

  pic->enableSignals ();
}



chartsPic::~chartsPic ()
{
}



void 
chartsPic::closeEvent (QCloseEvent *event __attribute__ ((unused)))
{
  slotQuit ();
}



void
chartsPic::leftMouse (QMouseEvent *e, int32_t x __attribute__ ((unused)), int32_t y __attribute__ ((unused)))
{
  if (zooming)
    {
      if (pic->rubberbandRectangleIsActive ())
        {
          int32_t *px, *py, *mx, *my;


          //  Important note: Adding the "dummy" argument causes nvPic to work in screen space instead of mapped picture space.

          pic->closeRubberbandRectangle (e->x (), e->y (), &px, &py, &mx, &my, dummy);
          pic->discardRubberbandRectangle ();


          double ux[4], uy[4];

          for (int32_t i = 0 ; i < 4 ; i++)
            {
              //  Convert px and py to orthorectified image coordinates.

              ux[i] = misc.geotiff_area[misc.zoom_level].min_x + ((double) px[i] / (double) misc.geotiff_size[misc.zoom_level].x) *
                (misc.geotiff_area[misc.zoom_level].max_x - misc.geotiff_area[misc.zoom_level].min_x);
              if (ux[i] < misc.geotiff_area[misc.zoom_level].min_x) ux[i] = misc.geotiff_area[misc.zoom_level].min_x;
              if (ux[i] > misc.geotiff_area[misc.zoom_level].max_x) ux[i] = misc.geotiff_area[misc.zoom_level].max_x;


              //  Screen space Y is inverted...

              uy[i] = misc.geotiff_area[misc.zoom_level].min_y +
                ((double) (misc.geotiff_size[misc.zoom_level].y - py[i]) / (double) misc.geotiff_size[misc.zoom_level].y) *
                (misc.geotiff_area[misc.zoom_level].max_y - misc.geotiff_area[misc.zoom_level].min_y);
              if (uy[i] < misc.geotiff_area[misc.zoom_level].min_y) uy[i] = misc.geotiff_area[misc.zoom_level].min_y;
              if (uy[i] > misc.geotiff_area[misc.zoom_level].max_y) uy[i] = misc.geotiff_area[misc.zoom_level].max_y;

              //fprintf (misc.dfp, "%s %s %d %d %d %d %d %d %f %f\n", NVFFL, i, px[i], py[i], misc.geotiff_size[misc.zoom_level].x, misc.geotiff_size[misc.zoom_level].y, ux[i], uy[i]);fflush (misc.dfp);
            }

          misc.zoom_level++;

          misc.geotiff_area[misc.zoom_level].min_x = 99999999999.0;
          misc.geotiff_area[misc.zoom_level].min_y = 99999999999.0;
          misc.geotiff_area[misc.zoom_level].max_x = -99999999999.0;
          misc.geotiff_area[misc.zoom_level].max_y = -99999999999.0;

          for (int32_t i = 0 ; i < 4 ; i++)
            {
              if (ux[i] < misc.geotiff_area[misc.zoom_level].min_x) misc.geotiff_area[misc.zoom_level].min_x = ux[i];
              if (uy[i] < misc.geotiff_area[misc.zoom_level].min_y) misc.geotiff_area[misc.zoom_level].min_y = uy[i];
              if (ux[i] > misc.geotiff_area[misc.zoom_level].max_x) misc.geotiff_area[misc.zoom_level].max_x = ux[i];
              if (uy[i] > misc.geotiff_area[misc.zoom_level].max_y) misc.geotiff_area[misc.zoom_level].max_y = uy[i];
            }
          //fprintf (misc.dfp, "%s %s %d %f %f %f %f\n", NVFFL, misc.geotiff_area[misc.zoom_level].min_x, misc.geotiff_area[misc.zoom_level].min_y, misc.geotiff_area[misc.zoom_level].max_x, misc.geotiff_area[misc.zoom_level].max_y);fflush (misc.dfp);

          redraw_ortho ();

          bZoomOut->setEnabled (true);
        }
      else
        {
          //  Important note: Adding the "dummy" argument causes nvPic to work in screen space instead of mapped picture space.

          pic->anchorRubberbandRectangle (e->x (), e->y (), Qt::white, 2, dummy);
        }
    }
  else
    {
      stickpin[stickpin_count].x = e->x ();
      stickpin[stickpin_count].y = e->y ();

      stickpin_count++;

      pic->redrawPic ();
    }
}



void
chartsPic::midMouse (QMouseEvent * e __attribute__ ((unused)), int32_t x __attribute__ ((unused)), int32_t y __attribute__ ((unused)))
{
  if (zooming) pic->discardRubberbandRectangle ();
}



void
chartsPic::rightMouse (QMouseEvent * e __attribute__ ((unused)), int32_t x __attribute__ ((unused)), int32_t y __attribute__ ((unused)))
{
  //  Placeholder
}



//  Signal from the pic class.

void
chartsPic::slotMousePress (QMouseEvent * e, int32_t x, int32_t y)
{
  if (e->button () == Qt::LeftButton) leftMouse (e, x, y);
  if (e->button () == Qt::MidButton) midMouse (e, x, y);
  if (e->button () == Qt::RightButton) rightMouse (e, x, y);
}



//  Signal from the pic class.

void
chartsPic::slotMouseRelease (QMouseEvent * e, int32_t x __attribute__ ((unused)), int32_t y __attribute__ ((unused)))
{
  if (e->button () == Qt::LeftButton) {};
  if (e->button () == Qt::MidButton) {};
  if (e->button () == Qt::RightButton) {};
}



//  Signal from the pic class.

void
chartsPic::slotMouseMove (QMouseEvent *e, int32_t x, int32_t y)
{
  //  Let other ABE programs know which window we're in.

  misc.abe_share->active_window_id = active_window_id;


  char ltstring[25], lnstring[25];
  QString string;


  if (ortho_displayed)
    {
      if (zooming)
        {
          //  Important note: Adding the "dummy" argument causes nvPic to work in screen space instead of mapped picture space.

          if (pic->rubberbandRectangleIsActive ()) pic->dragRubberbandRectangle (e->x (), e->y (), dummy);
        }


      //  Convert e->x () and e->y () to orthorectified image coordinates.

      double ux = misc.geotiff_area[misc.zoom_level].min_x + ((double) e->x () / (double) misc.geotiff_size[misc.zoom_level].x) *
        (misc.geotiff_area[misc.zoom_level].max_x - misc.geotiff_area[misc.zoom_level].min_x);


      //  Screen space Y is inverted...

      double uy = misc.geotiff_area[misc.zoom_level].min_y + ((double) (misc.geotiff_size[misc.zoom_level].y - e->y ()) / (double) misc.geotiff_size[misc.zoom_level].y) *
        (misc.geotiff_area[misc.zoom_level].max_y - misc.geotiff_area[misc.zoom_level].min_y);


      double nx = ux;
      double ny = uy;
      pj_transform (misc.pj_utm, misc.pj_latlon, 1, 1, &nx, &ny, NULL);
      double point_lon = nx * NV_RAD_TO_DEG;
      double point_lat = ny * NV_RAD_TO_DEG;

      double deg, min, sec;
      char hem;

      strcpy (ltstring, fixpos (point_lat, &deg, &min, &sec, &hem, POS_LAT, pos_format));
      strcpy (lnstring, fixpos (point_lon, &deg, &min, &sec, &hem, POS_LON, pos_format));

      string = tr ("Northing: %L1  Easting: %L2 , Lat: %3  Lon: %4    cursor position from orthorectified image").arg (ux, 0, 'f', 1).arg (uy, 0, 'f', 1).arg
        (ltstring).arg (lnstring);
    }
  else
    {
      string = tr ("X: %1  Y: %2   pixel of the original image").arg (y).arg (x);
    }

  statusBar ()->showMessage (string);
}



//  Timer - timeout signal.  Very much like an X workproc.  This tracks the cursor in the associated
//  programs (pfmEdit3D and geoSwath) not in this window.

void
chartsPic::slotTrackCursor ()
{
  int32_t            type = 0, cpf = 0;
  char               ltstring[25], lnstring[25], img_file[1024], ortho_file[1024];
  int64_t            timestamp = 0;
  FILE               *lfp = NULL, *ifp, *pfp;
  CZMIL_CPF_Header   cpf_header;
  CZMIL_CPF_Data     cpf_record;
  HOF_HEADER_T       hof_header;
  TOF_HEADER_T       tof_header;
  HYDRO_OUTPUT_T     hof_record;
  TOPO_OUTPUT_T      tof_record;
  POS_OUTPUT_T       pos_record;

  static QString     string;
  static uint8_t     no_nav = NVTrue;
  static int32_t     prev_pix_x = -1, prev_pix_y = -1, system_num = 2;
  static uint32_t    prev_rec = UINT32_MAX;
  static uint8_t     first = NVTrue, startup = NVTrue;
  static double      lat, lon, alt, new_lat, new_lon, dist, az, x_pix_size, y_pix_size, point_lat, point_lon;
  static int64_t     prev_time = -1;
  static uint8_t     *image = NULL;
  static uint32_t    size = 0;


  uint8_t get_dist (QDir up1, QString baseName, double *dist, double lat, double lon, LOCAL_JPW *jpw);
  uint8_t get_est_dist (QString baseName, double *dist, double lat, double lon, LOCAL_JPW *jpw);
  uint8_t get_geotiff (char *mosaic_file, MISC *misc);


  //  Since this is always a child process of something we want to exit if we see the CHILD_PROCESS_FORCE_EXIT key.
  //  We also want to exit on the ANCILLARY_FORCE_EXIT key (from pfmEdit3D) or if our own personal kill signal
  //  has been placed in misc.abe_share->key.

  if (misc.abe_share->key == CHILD_PROCESS_FORCE_EXIT || misc.abe_share->key == ANCILLARY_FORCE_EXIT || misc.abe_share->key == kill_switch) slotQuit ();


  QString sep = QString (SEPARATOR);


  //  Locking makes sure another process does not have memory locked.  It will block until it can lock it.
  //  At that point we copy the contents and then unlock it so other processes can continue.

  misc.abeShare->lock ();


  //  Check for change of record and correct record type.

  uint8_t hit = NVFalse;
  if (prev_rec != misc.abe_share->mwShare.multiRecord[0] &&
      (misc.abe_share->mwShare.multiType[0] == PFM_SHOALS_TOF_DATA || misc.abe_share->mwShare.multiType[0] == PFM_SHOALS_1K_DATA ||
       misc.abe_share->mwShare.multiType[0] == PFM_CHARTS_HOF_DATA || misc.abe_share->mwShare.multiType[0] == PFM_CZMIL_DATA))
    {
      l_share = *misc.abe_share;
      prev_rec = l_share.mwShare.multiRecord[0];
      hit = NVTrue;
    }

  misc.abeShare->unlock ();


  //  Save snippet file

  if (misc.abe_share->key == ac[1])
    {
      misc.abeShare->lock ();
      misc.abe_share->key = 0;

      if (!strstr (misc.abe_share->snippet_file_name, "NONE"))
        {
          misc.abe_share->heading = heading;

          snippet_file_name = QString (misc.abe_share->snippet_file_name);

          pic->SaveFile (snippet_file_name, NVTrue);


          string = tr ("Saved: %1").arg (misc.abe_share->snippet_file_name);

          statusBar ()->showMessage (string);
        }


      //  pfmEdit3D (actionKey.cpp) is looking for this modcode.

      misc.abe_share->modcode = CHARTSPIC_SAVED_PICTURE;
      misc.abeShare->unlock ();
    }


  //  Look for the full resolution file

  if (misc.abe_share->key == ac[2])
    {
      misc.abeShare->lock ();
      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED;
      misc.abeShare->unlock ();

      misc.zoom_level = 0;


      if (ortho)
        {
          ortho = ortho_displayed = NVFalse;
          no_nav = NVTrue;

          bZoomOut->setEnabled (false);
          bBrighten->setEnabled (true);
          bDarken->setEnabled (true);

          rotation = save_rotation;

          pic->RotatePixmap (rotation);

          hit = NVTrue;
          prev_time = -1;
        }
      else
        {
          if (l_share.mwShare.multiType[0] == PFM_CZMIL_DATA)
            {
              ortho = NVTrue;
              no_nav = NVFalse;

              save_rotation = rotation;
              rotation = NVFalse;

              pic->RotatePixmap (0.0);

              hit = NVTrue;
              prev_time = -1;

              zooming = NVTrue;

              pic->setCursor (zoomCursor);

              bBrighten->setEnabled (false);
              bDarken->setEnabled (false);
            }


          //  If you're trying to see orthorectified images and you're not looking at CZMIL data, throw it away.

          else
            {
              ortho = ortho_displayed = NVFalse;
              no_nav = NVTrue;

              bZoomOut->setEnabled (false);
              bBrighten->setEnabled (true);
              bDarken->setEnabled (true);
            }
        }
    }


  //  Toggle the picture rotation.

  if (misc.abe_share->key == ac[0])
    {
      misc.abeShare->lock ();
      misc.abe_share->key = 0;
      misc.abe_share->modcode = NO_ACTION_REQUIRED;
      misc.abeShare->unlock ();


      //  We don't want to mess with the rotation if an orthorectified image is displayed.

      if (!ortho_displayed)
        {
          if (rotation)
            {
              rotation = save_rotation = NVFalse;

              strcpy (prev_file, "NEW");

              pic->RotatePixmap (0.0);
            }
          else
            {
              rotation = save_rotation = NVTrue;

              pic->RotatePixmap ((double) heading);
            }
          bRotate->setChecked (rotation);

          pic->redrawPic ();
        }
    }


  //  Don't do anything unless we're CZMIL, HOF, or TOF and the record has changed.

  if (hit)
    {
      if (l_share.mwShare.multiType[0] == PFM_CHARTS_HOF_DATA || l_share.mwShare.multiType[0] == PFM_SHOALS_1K_DATA)
        {
          lfp = open_hof_file (l_share.nearest_filename);

          if (lfp == NULL)
            {
              string = tr ("Error opening %1\n%2").arg (QDir::toNativeSeparators (QString (l_share.nearest_filename))).arg (strerror (errno));
              statusBar ()->showMessage (string);

              return;
            }
          type = PFM_CHARTS_HOF_DATA;
        }
      else if (l_share.mwShare.multiType[0] == PFM_SHOALS_TOF_DATA)
        {
          lfp = open_tof_file (l_share.nearest_filename);


          if (lfp == NULL)
            {
              string = tr ("Error opening %1\n%2").arg (QDir::toNativeSeparators (QString (l_share.nearest_filename))).arg (strerror (errno));
              statusBar ()->showMessage (string);

              return;
            }
          type = PFM_SHOALS_TOF_DATA;
        }
      else if (l_share.mwShare.multiType[0] == PFM_CZMIL_DATA)
        {
          cpf = czmil_open_cpf_file (l_share.nearest_filename, &cpf_header, CZMIL_READONLY);


          if (cpf < 0)
            {
              string = tr ("Error opening %1\n%2").arg (QDir::toNativeSeparators (QString (l_share.nearest_filename))).arg (czmil_strerror ());
              statusBar ()->showMessage (string);

              return;
            }
          type = PFM_CZMIL_DATA;
        }


      switch (type)
        {
        case PFM_CHARTS_HOF_DATA:
          hof_read_header (lfp, &hof_header);

          system_num = hof_header.text.ab_system_number - 1;

          hof_read_record (lfp, l_share.mwShare.multiRecord[0], &hof_record);


          timestamp = hof_record.timestamp;
          alt = hof_record.altitude;
          point_lat = hof_record.latitude;
          point_lon = hof_record.longitude;
          fclose (lfp);
          break;

        case PFM_SHOALS_TOF_DATA:
          tof_read_header (lfp, &tof_header);

          system_num = tof_header.text.ab_system_number - 1;

          tof_read_record (lfp, l_share.mwShare.multiRecord[0], &tof_record);
          timestamp = tof_record.timestamp;
          point_lat = tof_record.latitude_last;
          point_lon = tof_record.longitude_last;
          fclose (lfp);
          break;

        case PFM_CZMIL_DATA:

          //  Here we are reading the camera sync header file to find the system number

          char cam_name[1024];
          QString simple;
          FILE *cfp;

          QFileInfo cpfFileInfo = QFileInfo (l_share.nearest_filename);
          QDir cpfDir = cpfFileInfo.absoluteDir ();

          char name[1024], img_dir_name[1024];

          strcpy (name, cpfDir.absolutePath ().toLatin1 ());
          strcpy (img_dir_name, gen_basename (name));


          //  Replace the "LD" with "DC".

          imgDir = QString (img_dir_name).replace ("LD_DS_P_", "DC_DS_P_");


          //  Go to the DC camera directory and define dataset name.

          QDir up1 = cpfDir;
          up1.cdUp ();
          up1.cd (imgDir);
          QString dataset = up1.dirName ().section ('_', 3);


          //  Read the timestamped CameraSync file to find the system number.

          QString camName = up1.absolutePath () + sep + "CameraSync_" + dataset + "_T.dat";

          strcpy (cam_name, camName.toLatin1 ());


          //  If we can't read the file just use system number 3 as the default.

          if (access (cam_name, F_OK) != -1)
            {
              system_num = 3;
            }
          else
            {
              //  Read the timestamped CameraSync file to find the system number.

              QString camName = up1.absolutePath () + sep + "CameraSync_" + dataset + "_R.dat";

              strcpy (cam_name, camName.toLatin1 ());

              if ((cfp = fopen (cam_name, "r")) == NULL)
                {
                  fprintf (stderr, "%s %s %s %d - %s - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, cam_name, strerror (errno));
                  perror (cam_name);
                  return;
                }


              //  Figure out the record size

              ngets (name, sizeof (name), cfp);


              // Get the first line header for the system number

              simple = QString (name).simplified ();
              system_num = simple.section (' ', 1, 1).toInt();
              printf ("System Number: %i \n", system_num);

              fclose (cfp);
            }


          czmil_read_cpf_record (cpf, l_share.mwShare.multiRecord[0], &cpf_record);

          czmil_close_cpf_file (cpf);


          timestamp = cpf_record.timestamp;


          //  The subrecord number for CZMIL consists of the channel (0-8) times 100 plus the return number (0-31).

          int32_t channel = l_share.mwShare.multiSubrecord[0] / 100;
          int32_t retnum =  l_share.mwShare.multiSubrecord[0] % 100;

          point_lat = cpf_record.channel[channel][retnum].latitude;
          point_lon = cpf_record.channel[channel][retnum].longitude;

          break;
        }


      if (!ortho_displayed && (first || strcmp (prev_file, l_share.nearest_filename) || timestamp / 1000000 != prev_time))
        {
          first = NVFalse;
          strcpy (prev_file, l_share.nearest_filename);


          //  HOF or TOF data.

          if (type != PFM_CZMIL_DATA)
            {
              strcpy (img_file, l_share.nearest_filename);
              strcpy (&img_file[strlen (img_file) - 4], ".img");

              ifp = open_image_file (img_file);
              if (ifp == NULL)
                {
                  string = tr ("Error opening %1\n%2").arg (QDir::toNativeSeparators (QString (img_file))).arg (strerror (errno));
                  statusBar ()->showMessage (string);

                  return;
                }


              image = image_read_record (ifp, timestamp, &size, &image_time);
              fclose (ifp);

              if (image == NULL)
                {
                  string = tr ("Error reading image");
                  statusBar ()->showMessage (string);

                  return;
                }


              //  Load the picture.

              if (!pic->OpenData (image, size))
                {
                  string = tr ("Error converting image");
                  statusBar ()->showMessage (string);
                  free (image);

                  return;
                }

              free (image);


              //  Zero the stickpin count since we've changed pictures.

              stickpin_count = 0;


              prev_time = timestamp / 1000000;


              //  Try to find the associated POS or SBET file.


              if (get_pos_file (l_share.nearest_filename, pos_file))
                {
                  if ((pfp = open_pos_file (pos_file)) != NULL)
                    {
                      pos_find_record (pfp, &pos_record, image_time);

                      lat = pos_record.latitude * NV_RAD_TO_DEG;
                      lon = pos_record.longitude * NV_RAD_TO_DEG;


                      //  Having to use height above ellipsoid for .tof data.

                      if (type == PFM_SHOALS_TOF_DATA) alt = pos_record.altitude;

                      fclose (pfp);


                      //  Find the center of the image.

                      heading = (pos_record.platform_heading - pos_record.wander_angle) * NV_RAD_TO_DEG;

                      dist = tan (pos_record.pitch + pitch_bias[system_num]) * alt;

                      newgp (lat, lon, heading, dist, &new_lat, &new_lon);


                      //  Add 270 (same as -90).

                      az = fmod (heading + 270.0, 360.0);

                      dist = tan (pos_record.roll + roll_bias[system_num]) * alt;

                      newgp (new_lat, new_lon, az, dist, &lat, &lon);

                      no_nav = NVFalse;
                    }
                  else
                    {
                      no_nav = NVTrue;
                    }
                }
              else
                {
                  no_nav = NVTrue;
                }


              if (!no_nav)
                {
                  //  Save the center lat/lon in case we want to save the picture.

                  center_lat = lat;
                  center_lon = lon;


                  //  This just sets the rotation.

                  if (rotation)
                    {
                      pic->RotatePixmap (heading);
                      pic->redrawPic ();
                    }
                  else
                    {
                      pic->RotatePixmap (0.0);
                    }


                  //  Estimating pixel size.  This isn't quite right since the picture isn't
                  //  taken straight down.  The difference per pixel in X can be as much as
                  //  ~2 cm at an elevation of 400 meters (37 meters difference between the
                  //  width at the forward edge and directly under the plane).  I think we can
                  //  live with this for the time being since we only really want the box so
                  //  we can see what the features are.

                  y_pix_size = (2.0 * tan (y_fov[system_num] / 2.0) * alt) / (double) picDef.height;
                  x_pix_size = (2.0 * tan (x_fov[system_num] / 2.0) * alt) / (double) picDef.width;

                  prev_pix_x = -1;
                  prev_pix_y = -1;
                }
            }


          //  PFM_CZMIL_DATA

          else
            {
              QFileInfo cpfFileInfo = QFileInfo (l_share.nearest_filename);
              QDir cpfDir = cpfFileInfo.absoluteDir ();

              char name[1024];
              int32_t winner = -1;
              LOCAL_JPW jpw[5];
              QString baseName[5];
              char cam_name[1024];
              uint8_t found = NVFalse;

              FILE *cfp;
              int64_t ts, pts = -1;
              double dist[5] = {0.0, 0.0, 0.0, 0.0, 0.0};
              double min_dist = 999999999999.0;
              uint8_t dist_comp[5];


              strcpy (name, cpfDir.absolutePath ().toLatin1 ());


              //  Read the timestamped CameraSync file to find the nearest timestamp to the data point timestamp.

              QDir up1 = cpfDir;
              up1.cdUp ();
              up1.cd (imgDir);

              QString dataset = up1.dirName ().section ('_', 3);


              //  Old camera system...

              if (system_num < 4)
                {
                  QString camName = up1.absolutePath () + sep + "CameraSync_" + dataset + "_T.dat";

                  QString simple;


                  strcpy (cam_name, camName.toLatin1 ());

                  if ((cfp = fopen (cam_name, "r")) == NULL)
                    {
                      fprintf (stderr, "%s %s %s %d - %s - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, cam_name, strerror (errno));
                      perror (cam_name);
                      return;
                    }


                  //  Figure out the record size.

                  ngets (name, sizeof (name), cfp);
                  int32_t rec_size = ftell (cfp);
                  fseek (cfp, 0, SEEK_SET);


                  //  Find the nearest picture (by time).

                  int32_t recnum = 1;
                  while (ngets (name, sizeof (name), cfp) != NULL)
                    {
                      // Define elements and list;

                      simple = QString (name).simplified ();

                      ts = simple.section (' ', 16, 16).toLongLong ();

                      if (ts > timestamp)
                        {
                          if (pts >= 0)
                            {
                              if (abs (ts - timestamp) <= abs (pts - timestamp))
                                {
                                  if (abs (ts - timestamp) > 3000000) return;

                                  found = NVTrue;
                                  break;
                                }
                              else
                                {
                                  if (abs (pts - timestamp) > 3000000) return;

                                  recnum--;
                                  found = NVTrue;
                                  break;
                                }
                            }
                        }

                      pts = ts;

                      recnum++;
                    }


                  //  In case we didn't find a nearby picture.

                  if (!found)
                    {
                      fprintf (stderr, "No picture found\n");
                      fclose (cfp);
                      return;
                    }


                  //  Try to read the previous records.

                  if (recnum > 1)
                    {
                      fseek (cfp, (recnum - 2) * rec_size, SEEK_SET);
                      ngets (name, sizeof (name), cfp);
                      simple = QString (name).simplified ();
                      baseName[0] = simple.section (' ', 1, 1);
                      jpw[0].aircraft.x = simple.section (' ', 2, 2).toDouble ();
                      jpw[0].aircraft.y = simple.section (' ', 3, 3).toDouble ();
                      jpw[0].aircraft.altitude = simple.section (' ', 4, 4).toDouble ();
                      jpw[0].aircraft.zone = simple.section (' ', 8, 8).toInt ();
                      strcpy (jpw[0].aircraft.hemi, simple.section (' ', 9, 9).toLatin1 ());
                      jpw[0].aircraft.lat = simple.section (' ', 10, 10).toDouble ();
                      jpw[0].aircraft.lon = simple.section (' ', 11, 11).toDouble ();
                      jpw[0].aircraft.roll = simple.section (' ', 13, 13).toDouble ();
                      jpw[0].aircraft.pitch = simple.section (' ', 14, 14).toDouble ();
                      jpw[0].aircraft.heading = simple.section (' ', 15, 15).toDouble ();
                    }

                  if (recnum > 0)
                    {
                      fseek (cfp, (recnum - 1) * rec_size, SEEK_SET);
                      ngets (name, sizeof (name), cfp);
                      simple = QString (name).simplified ();
                      baseName[1] = simple.section (' ', 1, 1);
                      jpw[1].aircraft.x = simple.section (' ', 2, 2).toDouble ();
                      jpw[1].aircraft.y = simple.section (' ', 3, 3).toDouble ();
                      jpw[1].aircraft.altitude = simple.section (' ', 4, 4).toDouble ();
                      jpw[1].aircraft.zone = simple.section (' ', 8, 8).toInt ();
                      strcpy (jpw[1].aircraft.hemi, simple.section (' ', 9, 9).toLatin1 ());
                      jpw[1].aircraft.lat = simple.section (' ', 10, 10).toDouble ();
                      jpw[1].aircraft.lon = simple.section (' ', 11, 11).toDouble ();
                      jpw[1].aircraft.roll = simple.section (' ', 13, 13).toDouble ();
                      jpw[1].aircraft.pitch = simple.section (' ', 14, 14).toDouble ();
                      jpw[1].aircraft.heading = simple.section (' ', 15, 15).toDouble ();
                    }


                  //  Read the selected record.

                  ngets (name, sizeof (name), cfp);
                  simple = QString (name).simplified ();
                  baseName[2] = simple.section (' ', 1, 1);
                  jpw[2].aircraft.x = simple.section (' ', 2, 2).toDouble ();
                  jpw[2].aircraft.y = simple.section (' ', 3, 3).toDouble ();
                  jpw[2].aircraft.altitude = simple.section (' ', 4, 4).toDouble ();
                  jpw[2].aircraft.zone = simple.section (' ', 8, 8).toInt ();
                  strcpy (jpw[2].aircraft.hemi, simple.section (' ', 9, 9).toLatin1 ());
                  jpw[2].aircraft.lat = simple.section (' ', 10, 10).toDouble ();
                  jpw[2].aircraft.lon = simple.section (' ', 11, 11).toDouble ();
                  jpw[2].aircraft.roll = simple.section (' ', 13, 13).toDouble ();
                  jpw[2].aircraft.pitch = simple.section (' ', 14, 14).toDouble ();
                  jpw[2].aircraft.heading = simple.section (' ', 15, 15).toDouble ();


                  //  Try to read the next records.

                  if (ngets (name, sizeof (name), cfp) != NULL)
                    {
                      simple = QString (name).simplified ();
                      baseName[3] = simple.section (' ', 1, 1);
                      jpw[3].aircraft.x = simple.section (' ', 2, 2).toDouble ();
                      jpw[3].aircraft.y = simple.section (' ', 3, 3).toDouble ();
                      jpw[3].aircraft.altitude = simple.section (' ', 4, 4).toDouble ();
                      jpw[3].aircraft.zone = simple.section (' ', 8, 8).toInt ();
                      strcpy (jpw[3].aircraft.hemi, simple.section (' ', 9, 9).toLatin1 ());
                      jpw[3].aircraft.lat = simple.section (' ', 10, 10).toDouble ();
                      jpw[3].aircraft.lon = simple.section (' ', 11, 11).toDouble ();
                      jpw[3].aircraft.roll = simple.section (' ', 13, 13).toDouble ();
                      jpw[3].aircraft.pitch = simple.section (' ', 14, 14).toDouble ();
                      jpw[3].aircraft.heading = simple.section (' ', 15, 15).toDouble ();
                    }

                  if (ngets (name, sizeof (name), cfp) != NULL)
                    {
                      simple = QString (name).simplified ();
                      baseName[4] = simple.section (' ', 1, 1);
                      jpw[4].aircraft.x = simple.section (' ', 2, 2).toDouble ();
                      jpw[4].aircraft.y = simple.section (' ', 3, 3).toDouble ();
                      jpw[4].aircraft.altitude = simple.section (' ', 4, 4).toDouble ();
                      jpw[4].aircraft.zone = simple.section (' ', 8, 8).toInt ();
                      strcpy (jpw[4].aircraft.hemi, simple.section (' ', 9, 9).toLatin1 ());
                      jpw[4].aircraft.lat = simple.section (' ', 10, 10).toDouble ();
                      jpw[4].aircraft.lon = simple.section (' ', 11, 11).toDouble ();
                      jpw[4].aircraft.roll = simple.section (' ', 13, 13).toDouble ();
                      jpw[4].aircraft.pitch = simple.section (' ', 14, 14).toDouble ();
                      jpw[4].aircraft.heading = simple.section (' ', 15, 15).toDouble ();
                    }

                  fclose (cfp);


                  //  Compute the distance from the center of the three images to the data point.

                  for (int32_t edc = 0 ; edc < 5 ; edc++) dist_comp[edc] = get_est_dist (baseName[edc], &dist[edc], point_lat, point_lon, &jpw[edc]);


                  //  Now that we have the estimated distances let's figure out which is closer.

                  for (int32_t edc = 0 ; edc < 5 ; edc++)
                    {
                      if (dist_comp[edc])
                        {
                          if (dist[edc] < min_dist)
                            {
                              min_dist = dist[edc];
                              winner = edc;
                            }
                        }
                    }

                  QString imgFile = up1.absolutePath () + sep + baseName[winner].replace (".jpg", "_scaled.jpeg");
                  strcpy (img_file, imgFile.toLatin1 ());

                  QString orthoFile = up1.absolutePath () + sep + "camera_single_frames_orthomosaics" + sep + baseName[winner].remove ("_scaled.jpeg") +
                    "single_ortho_mosaic_geo.tif";
                  strcpy (ortho_file, orthoFile.toLatin1 ());
                }


              //  RCD30 camera system...

              else
                {

                  QString camName = up1.absolutePath () + sep + "CameraSync_" + dataset + "_R.dat";

                  QString simple;

                  strcpy (cam_name, camName.toLatin1 ());

                  if ((cfp = fopen (cam_name, "r")) == NULL)
                    {
                      fprintf (stderr, "%s %s %s %d - %s - %s\n", misc.progname, __FILE__, __FUNCTION__, __LINE__, cam_name, strerror (errno));
                      perror (cam_name);
                      return;
                    }

                  //  Figure out the record size.

                  ngets (name, sizeof (name), cfp);
                  fseek (cfp, 0, SEEK_SET);


                  //  Find the nearest picture (by time).

                  uint8_t found = NVFalse;
                  int32_t recnum = 0;
                  while (ngets (name, sizeof (name), cfp) != NULL)
                    {
                      // Define elements and list;

                      simple = QString (name).simplified ();

                      ts = simple.section (' ', 16, 16).toLongLong ();

                      if (ts > timestamp)
                        {
                           if (pts >= 0)
                            {
                              if (abs (ts - timestamp) <= abs (pts - timestamp))
                                {
                                  if (abs (ts - timestamp) > 1000000) return;

                                  found = NVTrue;
                                  break;
                                }
                              else
                                {
                                  if (abs (pts - timestamp) > 1000000) return;
 
                                  recnum--;
                                  found = NVTrue;
                                  break;
                                }
                            }
                        }

                      pts = ts;

                      recnum++;
                    }

                  //  In case we didn't find a nearby picture.

                  if (!found)
                    {
                      fprintf (stderr, "No picture found\n");
                      fclose (cfp);
                      return;
                    }

                  //  Read the selected record.

                  ngets (name, sizeof (name), cfp);
                  simple = QString (name).simplified ();
                  baseName[0] = simple.section (' ', 1, 1);
                  jpw[0].aircraft.x = simple.section (' ', 2, 2).toDouble ();
                  jpw[0].aircraft.y = simple.section (' ', 3, 3).toDouble ();
                  jpw[0].aircraft.altitude = simple.section (' ', 4, 4).toDouble ();
                  jpw[0].aircraft.zone = simple.section (' ', 8, 8).toInt ();
                  strcpy (jpw[0].aircraft.hemi, simple.section (' ', 9, 9).toLatin1 ());
                  jpw[0].aircraft.lat = simple.section (' ', 10, 10).toDouble ();
                  jpw[0].aircraft.lon = simple.section (' ', 11, 11).toDouble ();
                  jpw[0].aircraft.roll = simple.section (' ', 13, 13).toDouble ();
                  jpw[0].aircraft.pitch = simple.section (' ', 14, 14).toDouble ();
                  jpw[0].aircraft.heading = simple.section (' ', 15, 15).toDouble ();


                  for (int32_t edc = 0 ; edc < 1 ; edc++) dist_comp[edc] = get_est_dist (baseName[edc], &dist[edc], point_lat, point_lon, &jpw[edc]);


                  //  Now that we have the estimated distances let's figure out which is closer.

                  for (int32_t edc = 0 ; edc < 1 ; edc++)
                    {
                      if (dist_comp[edc])
                        {
                          if (dist[edc] < min_dist)
                            {
                              min_dist = dist[edc];
                              winner = edc;
                            }
                        }
                    }

                  QString imgFile = up1.absolutePath () + sep + baseName[winner].replace (".jpg", "_scaled.jpeg");
                  strcpy (img_file, imgFile.toLatin1 ());

                  QString orthoFile = up1.absolutePath () + sep + "camera_single_frames_orthomosaics" + sep + baseName[winner].remove ("_scaled.jpeg") +
                    "single_ortho_mosaic_geo.tif";
                  strcpy (ortho_file, orthoFile.toLatin1 ());
                }


              //  If we're viewing a CZMIL image and the user requested the orthorectified original image...

              if (ortho)
                {
                  if (get_geotiff (ortho_file, &misc))
                    {
                      char proj_ref[1024];
                      strcpy (proj_ref, misc.projRef.toLatin1 ());
                      //fprintf (misc.dfp,"%lf %lf %lf %lf %lf %lf %d %d\n%s\n",misc.y_step, misc.x_step, misc.geotiff_area[0].min_y, misc.geotiff_area[0].min_x, misc.geotiff_area[0].max_y, misc.geotiff_area[0].max_x, misc.mosaic_width, misc.mosaic_height, proj_ref);fflush (misc.dfp);

                      QPixmap sub_image = QPixmap::fromImage (misc.full_res_image->copy (0, 0, misc.mosaic_width, misc.mosaic_height));
                      QPixmap scaled_image = sub_image.scaled (picDef.draw_width, picDef.draw_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                      misc.geotiff_size[misc.zoom_level].x = scaled_image.width ();
                      misc.geotiff_size[misc.zoom_level].y = scaled_image.height ();

                      pic->fillRectangle (0, 0, picDef.draw_width, picDef.draw_height, Qt::white, NVTrue, dummy);
                      pic->pixmapPaint (0, 0, &scaled_image, 0, 0, picDef.draw_width, picDef.draw_height);

                      ortho_displayed = NVTrue;
                    }
                  else
                    {
                      ortho = NVFalse;
                      no_nav = NVTrue;

                      string = tr ("Can't access %1").arg (ortho_file);
                      statusBar ()->showMessage (string);

                      return;
                    }
                }
              else
                {
                  //  Load the picture.

                  if (!pic->OpenFile (img_file))
                    {
                      string = tr ("Error loading image file");
                      statusBar ()->showMessage (string);

                      return;
                    }


                  //  Zero the stickpin count since we've changed pictures.

                  stickpin_count = 0;


                  prev_time = timestamp / 1000000;

                  if (winner >= 0)
                    {
                      LOCAL_JPW *wjpw = &jpw[winner];


                      //  Saving the camera sync file to the pos_file name so that it will show up in the status line as the nav source.

                      strcpy (pos_file, cam_name);


                      //  Save the center lat/lon in case we want to save the picture.

                      center_lat = lat = wjpw->center_lat;
                      center_lon = lon = wjpw->center_lon;


                      //  Save the heading.

                      heading = wjpw->aircraft.heading;


                      //  This just sets the rotation.

                      if (rotation)
                        {
                          pic->RotatePixmap (heading);
                          pic->redrawPic ();
                        }
                      else
                        {
                          pic->RotatePixmap (0.0);
                        }


                      //  Estimating pixel size.

                      alt = wjpw->aircraft.altitude;
                      y_pix_size = (2.0 * tan (y_fov[system_num] / 2.0) * alt) / (double) picDef.height;
                      x_pix_size = (2.0 * tan (x_fov[system_num] / 2.0) * alt) / (double) picDef.width;

                      prev_pix_x = -1;
                      prev_pix_y = -1;

                      no_nav = NVFalse;
                    }
                }
            }
        }
    }
  else
    {
      point_lat = misc.abe_share->cursor_position.y;
      point_lon = misc.abe_share->cursor_position.x;
    }


  //  If the cursor is in this window we don't want to try to keep track of the cursor elsewhere.

  if (misc.abe_share->active_window_id == active_window_id) return;


  //  Did we get a pos or sbet file, or a CZMIL CameraSync file?

  if (no_nav)
    {
      heading = 0.0;
      pic->RotatePixmap (0.0);
      string = tr ("Cannot find nav file for this picture!");
    }
  else
    {
      int32_t pix_x;
      int32_t pix_y;


      //  If we are displaying an orthorectified image we compute the position differently.

      if (ortho_displayed)
        {
          double ux = point_lon * NV_DEG_TO_RAD;
          double uy = point_lat * NV_DEG_TO_RAD;
          pj_transform (misc.pj_latlon, misc.pj_utm, 1, 1, &ux, &uy, NULL);


          pix_x = NINT (((ux - misc.geotiff_area[misc.zoom_level].min_x) / (misc.geotiff_area[misc.zoom_level].max_x - misc.geotiff_area[misc.zoom_level].min_x)) *
                        (double) misc.geotiff_size[misc.zoom_level].x);

          pix_y = NINT ((double) (misc.geotiff_size[misc.zoom_level].y) -
                        ((uy - misc.geotiff_area[misc.zoom_level].min_y) / (misc.geotiff_area[misc.zoom_level].max_y - misc.geotiff_area[misc.zoom_level].min_y)) *
                        (double) misc.geotiff_size[misc.zoom_level].y);

          //if (prev_pix_x != pix_x || prev_pix_y != pix_y)fprintf (misc.dfp,"%s %s %d %d %d %d %d %f %f %f %f %f %f\n",NVFFL,pix_x,pix_y,prev_pix_x,prev_pix_y,lat,lon,uy,ux,point_lat,point_lon);fflush (misc.dfp);


          hit = NVTrue;
        }
      else
        {
          //  Convert lat and lon into a pixel position in the picture.

          invgp (a0, b0, lat, lon, point_lat, point_lon, &dist, &az);


          double diff = az - heading;

          if (diff < 0.0) diff += 360.0;


          //  Convert to radians

          diff /= NV_RAD_TO_DEG;

          double x_dist = sin (diff) * dist;
          double y_dist = cos (diff) * dist;


          pix_x = picDef.width / 2 + NINT (x_dist / x_pix_size);
          pix_y = picDef.height / 2 - NINT (y_dist / y_pix_size);
        }


      if (prev_pix_x != pix_x || prev_pix_y != pix_y)
        {
          double deg, min, sec;
          char hem;

          strcpy (ltstring, fixpos (point_lat, &deg, &min, &sec, &hem, POS_LAT, pos_format));
          strcpy (lnstring, fixpos (point_lon, &deg, &min, &sec, &hem, POS_LON, pos_format));


          //  If we don't have a frozen cursor in pfmEdit3D...

          if (hit)
            {
              if (ortho_displayed)
                {
                  string = tr ("Lat: %1   Lon: %2   Alt: %L3   cursor position from parent ABE application").arg (ltstring).arg (lnstring).arg (alt, 0, 'f', 2);
                }
              else
                {
                  string = tr ("Lat: %1   Lon: %2   Alt: %L3   Nav source: %4").arg (ltstring).arg (lnstring).arg (alt, 0, 'f', 2).arg (gen_basename (pos_file));
                }
            }
          else
            {
              string = tr ("Lat: %1   Lon: %2   approximate cursor position from parent ABE application").arg (ltstring).arg (lnstring);
            }


          QBrush b1;
          if (rotation && !ortho_displayed)
            {
              pic->setMovingPath (marker, pix_x, pix_y, 2, Qt::red, b1, NVFalse, heading);
            }
          else
            {
              pic->setMovingPath (marker, pix_x, pix_y, 2, Qt::red, b1, NVFalse, 0.0);
            }

          prev_pix_x = pix_x;
          prev_pix_y = pix_y;
        }
    }


  statusBar ()->showMessage (string);


  //  Display the startup info message the first time through.

  if (startup)
    {
      QString key1;
      if (parentName == "geoSwath")
        {
          key1 = "\n\n";
        }
      else
        {
          key1 = tr ("%1 = Save the picture \"as is\" to a snippet file\n\n").arg (QString (ac[1]));
        }

      QString startupMessageText =
        tr ("The following action keys are available in %1\n\n"
            "%2 = Rotate the picture to \"north up\"\n"
            "%3"
            "You can change these key values in the %1\n"
            "Preferences->Ancillary Programs window.\n\n\n"
            "You can turn off this startup message in the\n"
            "chartsPic Preferences window.").arg (parentName).arg (QString (ac[0])).arg (key1);

      if (startup_message) QMessageBox::information (this, tr ("chartsPic Startup Message"), startupMessageText);

      startup = NVFalse;
    }
}



//  Signal from the pic class.

void
chartsPic::slotPostRedraw (NVPIC_DEF pic_picdef)
{
  picDef = pic_picdef;


  if (!ortho_displayed)
    {
      //  Draw the stickpin(s).

      int32_t x, y;

      for (int32_t i = 0 ; i < stickpin_count ; i++)
        {
          x = stickpin[i].x - 11;
          y = stickpin[i].y - 28;

          pic->pixmapPaint (x, y, stickPin, 0, 0, 32, 32);
        }


      zooming = NVFalse;
      pic->setCursor (stickpinCursor);
    }


  //  Let the caller know that we're up and running.

  misc.abeShare->lock ();

  if (misc.abe_share->key == WAIT_FOR_START) misc.abe_share->key = 0;

  misc.abeShare->unlock ();
}



void
chartsPic::slotResize (QResizeEvent *e __attribute__ ((unused)))
{
  if (ortho_displayed) redraw_ortho ();
}



void
chartsPic::slotKeyPress (QKeyEvent *e)
{
  char key[20];
  strcpy (key, e->text ().toLatin1 ());

  if (key[0] == (char) ac[0])
    {
      if (!ortho_displayed)
        {
          if (rotation)
            {
              rotation = save_rotation = NVFalse;
              strcpy (prev_file, "NEW");

              pic->RotatePixmap (0.0);
            }
          else
            {
              rotation = save_rotation = NVTrue;

              pic->RotatePixmap ((double) heading);
            }
          bRotate->setChecked (rotation);

          pic->redrawPic ();
        }
    }


  //  Put the key into abe_share->key

  if (key[0] == (char) ac[1] || key[0] == (char) ac[2]) misc.abe_share->key = key[0];


  e->accept ();
}



void
chartsPic::redrawPic ()
{
  pic->redrawPic ();
}



//  A bunch of slots.

void
chartsPic::slotQuit ()
{
  //  Let the parent program know that we have died from something other than the kill switch from the parent.

  if (misc.abe_share->key != kill_switch) misc.abe_share->killed = kill_switch;


  //  Make sure we haven't locked memory.

  misc.abeShare->unlock ();


  //  restore the rotation in case we were looking at an orthorectified image

  rotation = save_rotation;


  envout ();


  //  Let go of the shared memory.

  misc.abeShare->detach ();


  //fclose (misc.dfp);


  exit (0);
}



void
chartsPic::slotBrighten ()
{
  pic->brighten (10);
}



void
chartsPic::slotDarken ()
{
  pic->brighten (-10);
}



void
chartsPic::slotZoomOut ()
{
  if (misc.zoom_level > 0)
    {
      misc.zoom_level--;

      redraw_ortho ();
    }

  if (!misc.zoom_level) bZoomOut->setEnabled (false);
}



void
chartsPic::slotOrtho ()
{
  misc.abe_share->key = (char) ac[2];
}



void
chartsPic::redraw_ortho ()
{
  int32_t start_x = NINT ((misc.geotiff_area[misc.zoom_level].min_x - misc.geotiff_area[0].min_x) / misc.x_step);
  int32_t start_y = NINT ((misc.geotiff_area[0].max_y - misc.geotiff_area[misc.zoom_level].max_y) / misc.y_step);
  int32_t new_width = NINT ((misc.geotiff_area[misc.zoom_level].max_x - misc.geotiff_area[misc.zoom_level].min_x) / misc.x_step);
  int32_t new_height = NINT ((misc.geotiff_area[misc.zoom_level].max_y - misc.geotiff_area[misc.zoom_level].min_y) / misc.y_step);

  QPixmap sub_image = QPixmap::fromImage (misc.full_res_image->copy (start_x, start_y, new_width, new_height));
  QPixmap scaled_image = sub_image.scaled (picDef.draw_width, picDef.draw_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  misc.geotiff_size[misc.zoom_level].x = scaled_image.width ();
  misc.geotiff_size[misc.zoom_level].y = scaled_image.height ();

  pic->fillRectangle (0, 0, picDef.draw_width, picDef.draw_height, Qt::white, NVTrue, dummy);
  pic->pixmapPaint (0, 0, &scaled_image, 0, 0, picDef.draw_width, picDef.draw_height);
}



void
chartsPic::picSave (uint8_t scaled)
{
  //  Convert the image time to something reasonable

  int32_t year, jday, hour, minute;
  float second;

  charts_cvtime (image_time, &year, &jday, &hour, &minute, &second);


  //  Convert the lat and lon to something reasonable

  char ltstring[50], lnstring[50];
  double deg, min, sec;
  char       hem;

  strcpy (ltstring, fixpos (center_lat, &deg, &min, &sec, &hem, POS_LAT, pos_format));
  for (int32_t i = 0 ; i < (int32_t) strlen (ltstring) ; i++) if (ltstring[i] == ' ') ltstring[i] = '_';
  strcpy (lnstring, fixpos (center_lon, &deg, &min, &sec, &hem, POS_LON, pos_format));
  for (int32_t i = 0 ; i < (int32_t) strlen (lnstring) ; i++) if (lnstring[i] == ' ') lnstring[i] = '_';


  //  Set the default file name.

  QString name;
  name.sprintf ("P%04d%03d%02d%02d%05.2f__%s__%s.jpg", year + 1900, jday, hour, minute, second, ltstring, lnstring);


  QDir dir = QDir (save_directory);

  QFileDialog *fd = new QFileDialog (this, tr ("chartsPic Save As"));
  fd->setViewMode (QFileDialog::List);
  fd->setDirectory (dir);

  QStringList filters;
  filters << tr ("JPEG (*.jpg)");

  fd->setNameFilters (filters);
  fd->setFileMode (QFileDialog::AnyFile);
  fd->selectNameFilter (tr ("JPEG (*.jpg)"));
  fd->selectFile (name);

  QStringList files;
  if (fd->exec () == QDialog::Accepted)
    {
      files = fd->selectedFiles ();

      save_file_name = files.at (0);

      if (!save_file_name.endsWith (".jpg")) save_file_name.append (".jpg");

      save_directory = fd->directory ().absolutePath ();

      pic->SaveFile (save_file_name, scaled);
    }
}



void
chartsPic::slotSave ()
{
  picSave (NVFalse);
}



void
chartsPic::slotScaled ()
{
  picSave (NVTrue);
}



void
chartsPic::slotPrefs ()
{
  prefsD = new QDialog (this, (Qt::WindowFlags) Qt::WA_DeleteOnClose);
  prefsD->setWindowTitle (tr ("chartsPic Preferences"));
  prefsD->setModal (true);

  QVBoxLayout *vbox = new QVBoxLayout (prefsD);
  vbox->setMargin (5);
  vbox->setSpacing (5);

  QGroupBox *fbox = new QGroupBox (tr ("Position Format"), prefsD);
  fbox->setWhatsThis (bGrpText);

  QRadioButton *hdms = new QRadioButton (tr ("Hemisphere Degrees Minutes Seconds.decimal"));
  QRadioButton *hdm_ = new QRadioButton (tr ("Hemisphere Degrees Minutes.decimal"));
  QRadioButton *hd__ = new QRadioButton (tr ("Hemisphere Degrees.decimal"));
  QRadioButton *sdms = new QRadioButton (tr ("+/-Degrees Minutes Seconds.decimal"));
  QRadioButton *sdm_ = new QRadioButton (tr ("+/-Degrees Minutes.decimal"));
  QRadioButton *sd__ = new QRadioButton (tr ("+/-Degrees.decimal"));

  bGrp = new QButtonGroup (prefsD);
  bGrp->setExclusive (true);
  connect (bGrp, SIGNAL (buttonClicked (int)), this, SLOT (slotPosClicked (int)));

  bGrp->addButton (hdms, 0);
  bGrp->addButton (hdm_, 1);
  bGrp->addButton (hd__, 2);
  bGrp->addButton (sdms, 3);
  bGrp->addButton (sdm_, 4);
  bGrp->addButton (sd__, 5);

  QHBoxLayout *fboxSplit = new QHBoxLayout;
  QVBoxLayout *fboxLeft = new QVBoxLayout;
  QVBoxLayout *fboxRight = new QVBoxLayout;
  fboxSplit->addLayout (fboxLeft);
  fboxSplit->addLayout (fboxRight);
  fboxLeft->addWidget (hdms);
  fboxLeft->addWidget (hdm_);
  fboxLeft->addWidget (hd__);
  fboxRight->addWidget (sdms);
  fboxRight->addWidget (sdm_);
  fboxRight->addWidget (sd__);
  fbox->setLayout (fboxSplit);

  vbox->addWidget (fbox, 1);

  bGrp->button (pos_format)->setChecked (true);


  QGroupBox *mBox = new QGroupBox (tr ("Display startup message"), this);
  QHBoxLayout *mBoxLayout = new QHBoxLayout;
  mBox->setLayout (mBoxLayout);
  sMessage = new QCheckBox (mBox);
  sMessage->setToolTip (tr ("Toggle display of startup message when program starts"));
  mBoxLayout->addWidget (sMessage);
  if (startup_message)
    {
      sMessage->setCheckState (Qt::Checked);
    }
  else
    {
      sMessage->setCheckState (Qt::Unchecked);
    }


  vbox->addWidget (mBox, 1);


  QHBoxLayout *actions = new QHBoxLayout (0);
  vbox->addLayout (actions);

  QPushButton *bHelp = new QPushButton (prefsD);
  bHelp->setIcon (QIcon (":/icons/contextHelp.png"));
  bHelp->setToolTip (tr ("Enter What's This mode for help"));
  connect (bHelp, SIGNAL (clicked ()), this, SLOT (slotHelp ()));
  actions->addWidget (bHelp);

  actions->addStretch (10);

  QPushButton *closeButton = new QPushButton (tr ("Close"), prefsD);
  closeButton->setToolTip (tr ("Close the preferences dialog"));
  connect (closeButton, SIGNAL (clicked ()), this, SLOT (slotClosePrefs ()));
  actions->addWidget (closeButton);

  prefsD->show ();
}


void
chartsPic::slotHelp ()
{
  QWhatsThis::enterWhatsThisMode ();
}



void
chartsPic::slotPosClicked (int id)
{
  pos_format = id;
}



void
chartsPic::slotClosePrefs ()
{
  if (sMessage->checkState () == Qt::Checked)
    {
      startup_message = NVTrue;
    }
  else
    {
      startup_message = NVFalse;
    }

  prefsD->close ();
}


void
chartsPic::slotRotate ()
{
  if (bRotate->isChecked ())
    {
      rotation = NVTrue;

      pic->RotatePixmap ((double) heading);
    }
  else
    {
      rotation = NVFalse;
      strcpy (prev_file, "NEW");

      pic->RotatePixmap (0.0);
    }

  pic->redrawPic ();
}



void
chartsPic::about ()
{
  QMessageBox::about (this, VERSION, tr ("chartsPic - CHARTS down-looking picture viewer.\n\nAuthor : Jan C. Depner (jan@pfmabe.software)"));
}


void
chartsPic::slotAcknowledgments ()
{
  QMessageBox::about (this, VERSION, acknowledgmentsText);
}


void
chartsPic::aboutQt ()
{
  QMessageBox::aboutQt (this, VERSION);
}


//  Get the users defaults.

void
chartsPic::envin ()
{
  double saved_version = 1.0;


  // Set defaults so that if keys don't exist the parameters are defined

  pos_format = POS_HDMS;

  save_directory = ".";


  window_x = 0;
  window_y = 0;
  width = PIC_X_SIZE;
  height = PIC_Y_SIZE;
  rotation = NVFalse;
  startup_message = NVTrue;
  font = QApplication::font ();


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/chartsPic.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/chartsPic.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("chartsPic");

  saved_version = settings.value (QString ("settings version"), saved_version).toDouble ();


  //  If the settings version has changed we need to leave the values at the new defaults since they may have changed.

  if (settings_version != saved_version) return;


  pos_format = settings.value (QString ("position form"), pos_format).toInt ();

  rotation = settings.value (QString ("rotate image"), rotation).toBool ();

  save_directory = settings.value (QString ("save directory"), save_directory).toString ();

  width = settings.value (QString ("width"), width).toInt ();

  height = settings.value (QString ("height"), height).toInt ();

  window_x = settings.value (QString ("window x"), window_x).toInt ();

  window_y = settings.value (QString ("window y"), window_y).toInt ();


  startup_message = settings.value (QString ("Display Startup Message"), startup_message).toBool ();


  this->restoreState (settings.value (QString ("main window state")).toByteArray (), NINT (settings_version * 100.0));

  settings.endGroup ();


  //  We need to get the font from the global settings since it will be used in (hopefully) all of the ABE map GUI applications.

#ifdef NVWIN3X
  QString ini_file2 = QString (getenv ("USERPROFILE")) + "/ABE.config/" + "globalABE.ini";
#else
  QString ini_file2 = QString (getenv ("HOME")) + "/ABE.config/" + "globalABE.ini";
#endif

  QSettings settings2 (ini_file2, QSettings::IniFormat);
  settings2.beginGroup ("globalABE");


  QString defaultFont = font.toString ();
  QString fontString = settings2.value (QString ("ABE map GUI font"), defaultFont).toString ();
  font.fromString (fontString);


  settings2.endGroup ();
}




//  Save the users defaults.

void
chartsPic::envout ()
{
  //  Use frame geometry to get the absolute x and y.

  QRect tmp = this->frameGeometry ();
  window_x = tmp.x ();
  window_y = tmp.y ();


  //  Use geometry to get the width and height.

  tmp = this->geometry ();
  width = tmp.width ();
  height = tmp.height ();


  //  Get the INI file name

#ifdef NVWIN3X
  QString ini_file = QString (getenv ("USERPROFILE")) + "/ABE.config/chartsPic.ini";
#else
  QString ini_file = QString (getenv ("HOME")) + "/ABE.config/chartsPic.ini";
#endif

  QSettings settings (ini_file, QSettings::IniFormat);
  settings.beginGroup ("chartsPic");


  settings.setValue (QString ("settings version"), settings_version);

  settings.setValue (QString ("position form"), pos_format);

  settings.setValue (QString ("rotate image"), rotation);

  settings.setValue (QString ("save directory"), save_directory);

  settings.setValue (QString ("width"), width);

  settings.setValue (QString ("height"), height);

  settings.setValue (QString ("window x"), window_x);

  settings.setValue (QString ("window y"), window_y);


  settings.setValue (QString ("Display Startup Message"), startup_message);


  settings.setValue (QString ("main window state"), this->saveState (NINT (settings_version * 100.0)));

  settings.endGroup ();
}

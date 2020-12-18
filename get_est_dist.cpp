
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

#include "chartsPic.hpp"

uint8_t get_est_dist (QString baseName, double *dist, double lat, double lon, LOCAL_JPW *jpw)
{
  projPJ pj_utm, pj_latlon;


  if (baseName.isNull ()) return (NVFalse);


  QString sep = QString (SEPARATOR);


  //  Find the center of the image.

  double mv_dist, new_lat[2], new_lon[2];

  mv_dist = tan (jpw->aircraft.pitch * NV_DEG_TO_RAD) * jpw->aircraft.altitude;

  newgp (jpw->aircraft.lat, jpw->aircraft.lon, jpw->aircraft.heading, mv_dist, &new_lat[0], &new_lon[0]);


  //  Add 270 (same as -90).

  double az = fmod (jpw->aircraft.heading + 270.0, 360.0);

  mv_dist = tan (jpw->aircraft.roll * NV_DEG_TO_RAD) * jpw->aircraft.altitude;

  newgp (new_lat[0], new_lon[0], az, mv_dist, &new_lat[1], &new_lon[1]);


  //  Now compute the distance based on the lat and lon of the nearest data point.

  char prj[60];

  if (strstr (jpw->aircraft.hemi, "S"))
    {
      sprintf (prj, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +south", jpw->aircraft.zone);
    }
  else
    {
      sprintf (prj, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84", jpw->aircraft.zone);
    }

  if (!(pj_utm = pj_init_plus (prj))) return (NVFalse);

  if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84"))) return (NVFalse);

  jpw->point_x = lon * NV_DEG_TO_RAD;
  jpw->point_y = lat * NV_DEG_TO_RAD;
  pj_transform (pj_latlon, pj_utm, 1, 1, &jpw->point_x, &jpw->point_y, NULL);
  //fprintf (stderr,"%s %s %d %f %f\n",NVFFL,jpw->point_x,jpw->point_y);

  jpw->center_lat = new_lat[1];
  jpw->center_lon = new_lon[1];

  jpw->center_x = new_lon[1] * NV_DEG_TO_RAD;
  jpw->center_y = new_lat[1] * NV_DEG_TO_RAD;
  pj_transform (pj_latlon, pj_utm, 1, 1, &jpw->center_x, &jpw->center_y, NULL);

  //fprintf (stderr,"%s %s %d %f %f %f %f %f %f %f\n",NVFFL,jpw->aircraft.x,jpw->aircraft.y,jpw->center_x,jpw->center_y, jpw->aircraft.roll,jpw->aircraft.pitch,jpw->aircraft.heading);


  *dist = sqrt (((jpw->point_x - jpw->center_x) * (jpw->point_x - jpw->center_x)) + ((jpw->point_y - jpw->center_y) * (jpw->point_y - jpw->center_y)));


  return (NVTrue);
}

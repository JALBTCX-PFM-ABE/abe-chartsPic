
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

uint8_t get_dist (QDir up1, QString baseName, double *dist, double lat, double lon, LOCAL_JPW *jpw)
{
  projPJ pj_utm, pj_latlon;


  if (baseName.isNull ()) return (NVFalse);


  QString sep = QString (SEPARATOR);


  //  Create the .jpw file name so we can read the center location.

  char jpw_data[200], jpw_file[1024];
  FILE *jfp;

  QString single_mosaic = baseName.remove (".jpg") + "single_mosaic";
  QString jpwFile = up1.absolutePath () + sep + single_mosaic + ".jpw";

  strcpy (jpw_file, jpwFile.toLatin1 ());

  if ((jfp = fopen (jpw_file, "r")) == NULL) return (NVFalse);

  while (ngets (jpw_data, sizeof (jpw_data), jfp) != NULL)
    {
      if (strstr (jpw_data, "samples = "))
        {
          sscanf (jpw_data, "samples = %d", &jpw->width);
        }
      else if (strstr (jpw_data, "lines = "))
        {
          sscanf (jpw_data, "lines = %d", &jpw->height);
        }
      else if (strstr (jpw_data, "map info = "))
        {
          sscanf (jpw_data, "map info = {UTM, 1, 1, %lf, %lf, %lf, %lf, %d, %s", &jpw->min_x, &jpw->max_y, &jpw->x_step, &jpw->y_step,
                  &jpw->zone, jpw->hemi);
        }
      else if (strstr (jpw_data, "center = "))
        {
          sscanf (jpw_data, "center = %lf, %lf, %lf, %lf", &jpw->center_lat, &jpw->center_lon, &jpw->center_x, &jpw->center_y);
        }
    }


  fclose (jfp);


  //  Now compute the distance based on the lat and lon of the nearest data point.

  char prj[60];

  if (strstr (jpw->hemi, "South"))
    {
      sprintf (prj, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84 +south", jpw->zone);
    }
  else
    {
      sprintf (prj, "+proj=utm +zone=%d +ellps=WGS84 +datum=WGS84", jpw->zone);
    }

  if (!(pj_utm = pj_init_plus (prj))) return (NVFalse);

  if (!(pj_latlon = pj_init_plus ("+proj=latlon +ellps=WGS84 +datum=WGS84"))) return (NVFalse);

  jpw->point_x = lon * NV_DEG_TO_RAD;
  jpw->point_y = lat * NV_DEG_TO_RAD;
  pj_transform (pj_latlon, pj_utm, 1, 1, &jpw->point_x, &jpw->point_y, NULL);


  *dist = sqrt (((jpw->point_x - jpw->center_x) * (jpw->point_x - jpw->center_x)) + ((jpw->point_y - jpw->center_y) * (jpw->point_y - jpw->center_y)));


  return (NVTrue);
}


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


uint8_t get_geotiff (char *mosaic_file, MISC *misc)
{
  if (strstr (mosaic_file, ".tif") || strstr (mosaic_file, ".TIF"))
    {
      GDALDataset     *poDataset;
      double          adfGeoTransform[6];


      GDALAllRegister ();


      poDataset = (GDALDataset *) GDALOpen (mosaic_file, GA_ReadOnly);
      if (poDataset != NULL)
        {
          if (poDataset->GetProjectionRef ()  != NULL)
            {
              misc->projRef = QString (poDataset->GetProjectionRef ());

              if (misc->projRef.contains ("GEOGCS"))
                {
                  if (poDataset->GetGeoTransform (adfGeoTransform) == CE_None)
                    {
                      misc->x_step = adfGeoTransform[1];
                      misc->y_step = -adfGeoTransform[5];


                      misc->mosaic_width = poDataset->GetRasterXSize ();
                      misc->mosaic_height = poDataset->GetRasterYSize ();


                      misc->geotiff_area[0].min_x = adfGeoTransform[0];
                      misc->geotiff_area[0].max_y = adfGeoTransform[3];


                      misc->geotiff_area[0].min_y = misc->geotiff_area[0].max_y - misc->mosaic_height * misc->y_step;
                      misc->geotiff_area[0].max_x = misc->geotiff_area[0].min_x + misc->mosaic_width * misc->x_step;
                    }
                  else
                    {
                      delete poDataset;
                      return (NVFalse);
                    }
                }
              else
                {
                  delete poDataset;
                  return (NVFalse);
                }
            }
          else
            {
              delete poDataset;
              return (NVFalse);
            }

          if (misc->full_res_image != NULL) delete misc->full_res_image;


          misc->full_res_image = new QImage (mosaic_file);

          if (misc->full_res_image == NULL || misc->full_res_image->width () == 0 || misc->full_res_image->height () == 0)
            {
              delete poDataset;
              return (NVFalse);
            }

          delete poDataset;
        }
      else
        {
          return (NVFalse);
        }
    }
  else
    {
      return (NVFalse);
    }


  if (misc->projRef.contains ("PROJCS"))
    {
      char proj_ref[1024], string[1024];
      strcpy (proj_ref, misc->projRef.toLatin1 ());

      OGRSpatialReference SRS;
      char *ppszProj4, *ptr_wkt = proj_ref;

      SRS.importFromWkt (&ptr_wkt);

      SRS.exportToProj4 (&ppszProj4);

      strcpy (string, ppszProj4);
      OGRFree (ppszProj4);


      if (!(misc->pj_utm = pj_init_plus (string))) return (NVFalse);


      if (QString (misc->abe_share->open_args[0].head.proj_data.wkt).contains ("GEOGCS"))
        {
          OGRSpatialReference pfmSRS;
          ptr_wkt = misc->abe_share->open_args[0].head.proj_data.wkt;

          SRS.importFromWkt (&ptr_wkt);

          SRS.exportToProj4 (&ppszProj4);

          strcpy (string, ppszProj4);
          OGRFree (ppszProj4);
        }
      else
        {
          strcpy (string, "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs");
        }


      if (!(misc->pj_latlon = pj_init_plus (string))) return (NVFalse);
    }
  //fprintf (misc->dfp,"%s %s %d %f %f %f %f\n",NVFFL,misc->geotiff_area[0].min_x, misc->geotiff_area[0].min_y, misc->geotiff_area[0].max_x, misc->geotiff_area[0].max_y);fflush (misc->dfp);
 
  return (NVTrue);
}

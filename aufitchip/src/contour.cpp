/*-------------------------------------------------------------------------
    contour.cpp
    Реализация класса TContourImage.
-------------------------------------------------------------------------*/
#include <tc_config.h>
#include <c_lib/c_types.hpp>
#include <contour.hpp>

TContourImage :: TContourImage()
{
}

TContourImage :: ~TContourImage()
{
}

/****************************************************************************************************************************************************************************************
    Выделение контурных пикселов береговой линии для бинарного изображения (в частности маска суша/море).
    Если mark_sea = sea или mark_land = mark_land, тогда контурные пикселы пренадлежат суше или морю.

    Параметры:
    b_image                             Бинарное изображение, результат - выделенные контурные пикселы, помечаются в нём
    sea, land                           Значения для моря и суши в b_image
    cloudy_mask                         Маска облачности для b_image (0 - пиксел безоблачный, 1 - пиксел покрыт облачностью)
    width, height                       Количество строк и количество пикселов в строке для b_image и cloudy_mask
    mark_sea, mark_land                 Значения, которыми помечаются контурные пикселы (открытые от облачности), как для моря так и для суши в b_image
    mark_cloudy_sea, mark_cloudy_land   Значения, которыми помечаются контурные пикселы (покрытые облачностью), как для моря так и для суши в b_image
*****************************************************************************************************************************************************************************************/

void TContourImage :: coastal_lineBinaryImage( const char * b_image, char sea, char land, const char * cloudy_mask,
                                               long width, long height, char mark_sea, char mark_land,
                                               char mark_cloudy_sea, char mark_cloudy_land )
{
    fBinaryImage = (char *)b_image;
    fCloudyMask = (char *)cloudy_mask;

    fWidth = width;
    fHeight = height;

    bool flagland = false;
    for( long i = 0; i < fHeight; i++ ){
         char * bimage_p = fBinaryImage + i * fWidth;
         char * mcloudy_p = fCloudyMask + i * fWidth;
         flagland = false;
         for( long j = 0; j < fWidth; j++ ){
              if( flagland == false && j == 0 && bimage_p[j] == land ) flagland = true;
              if( flagland == false && bimage_p[j] == land ){
                  if( bimage_p[j] == land ){
                      if( mcloudy_p[j] ) bimage_p[j] = mark_cloudy_land;
                      else bimage_p[j] = mark_land;
                  }
                  if( j - 1 >= 0 && bimage_p[j - 1] == sea ){
                      if( mcloudy_p[j - 1] )  bimage_p[j - 1] = mark_cloudy_sea;
                      else bimage_p[j - 1] = mark_sea;
                  }
                  if( j + 1 >= fWidth ) break;
                  if( bimage_p[j + 1] == land ) flagland = true;
                  else  flagland = false;
                  continue;
             }
             if( flagland == true && bimage_p[j] == sea ){
                 if( bimage_p[j] == sea ){
                     if( mcloudy_p[j] ) bimage_p[j] = mark_cloudy_sea;
                     else bimage_p[j] = mark_sea;
                 }
                 if( j - 1 >= 0 && bimage_p[j - 1] == land ){
                     if( mcloudy_p[j - 1] ) bimage_p[j - 1] = mark_cloudy_land;
                     else bimage_p[j - 1] = mark_land;
                 }
                 if( j + 1 >= fWidth ) break;
                 if( bimage_p[j + 1] == land ) flagland = true;
                 else  flagland = false;
                 continue;
             }

         }
    }

    for( long j = 0; j < fWidth; j++ ){
         char * bimage_p = fBinaryImage + j;
         char * mcloudy_p = fCloudyMask + j;
         flagland = false;
         for( long i = 0; i < fHeight; i++ ){
              if( flagland == false && i == 0 && (bimage_p[i * fWidth] == land || bimage_p[i * fWidth] == mark_land) ) flagland = true;
              if( flagland == false && (bimage_p[i * fWidth] == land || bimage_p[i * fWidth] == mark_land) ){
                  if( bimage_p[i * fWidth] == land ){
                      if( mcloudy_p[i * fWidth] ) bimage_p[i * fWidth] = mark_cloudy_land;
                      else bimage_p[i * fWidth] = mark_land;
                  }
                  if( i - 1 >= 0 && bimage_p[(i - 1) * fWidth] == sea ){
                      if( mcloudy_p[(i - 1) * fWidth] ) bimage_p[(i - 1) * fWidth] = mark_cloudy_sea;
                      else bimage_p[(i - 1) * fWidth] = mark_sea;
                  }
                  if( i + 1 >= fHeight ) break;
                  if( bimage_p[(i + 1) * fWidth] == land || bimage_p[(i + 1) * fWidth] == mark_land ) flagland = true;
                  else  flagland = false;
                  continue;
              }
              if( flagland == true && (bimage_p[i * fWidth] == sea || bimage_p[i * fWidth] == mark_sea) ){
                  if( i < fHeight && bimage_p[i * fWidth] == sea ){
                      if( mcloudy_p[i * fWidth] ) bimage_p[i * fWidth] = mark_cloudy_sea;
                      else bimage_p[i * fWidth] = mark_sea;
                  }
                  if( i - 1 >= 0 && bimage_p[(i - 1) * fWidth] == land ){
                      if( mcloudy_p[(i - 1) * fWidth] ) bimage_p[(i - 1) * fWidth] = mark_cloudy_land;
                      else bimage_p[(i - 1) * fWidth] = mark_land;
                  }
                  if( i + 1 >= fHeight ) break;
                  if( bimage_p[(i + 1) * fWidth] == land || bimage_p[(i + 1) * fWidth] == mark_land ) flagland = true;
                  else  flagland = false;
                  continue;
              }
         }
    }
/*
    // Собственно запись PCX-файла.
    TPCXHdr hdr;
    pcx_ini_hdr_256( &hdr, fWidth, fHeight );

    char pal_pcx[256 * 3 + 1];
    pal_pcx[0] = 12;
    PCXRGB * pal = (PCXRGB *)(pal_pcx + 1);
    memset( pal, 0, sizeof( PCXRGB ) * 256 );

    pal[land].r = 255; pal[land].g = 255; pal[land].b = 255;   // суша.
    pal[sea].r = 0; pal[sea].g = 0; pal[sea].b = 0;           // море.
    pal[mark_sea].r = 0; pal[mark_sea].g = 0; pal[mark_sea].b = 255;      // контурные пикселы - море.
    pal[mark_land].r = 0; pal[mark_land].g = 255; pal[mark_land].b = 0;  // контурные пикселы - суша.
    pal[mark_cloudy_sea].r = 255; pal[mark_cloudy_sea].g = 0; pal[mark_cloudy_sea].b = 255;     // контурные пикселы (покрытые облачностью) - море.
    pal[mark_cloudy_land].r = 255; pal[mark_cloudy_land].g = 255; pal[mark_cloudy_land].b = 0; // контурные пикселы (покрытые облачностью) - суша.

    char * buf = fBinaryImage;
    char * buffer = new char [fWidth * 2];	// Используется для хранения одной строки PCX-данных.

    // Создание имени PCX-файла.
    char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], file_path[_MAX_PATH];
    _splitpath( fCS.fGPCXDir, drive, dir, 0, 0 );
    _makepath( file_path, drive, dir, "contour", "pcx" );

    FILE *f = fopen( file_path, "wb" );
    fwrite( &hdr, 1, 128, f );

    for( unsigned long i = 0; i < fHeight; i++, buf += fWidth ){
         // Построчное PCX-кодирование данных из буфера картинки и запись их в файл.
        fwrite( buffer, 1, pcx_pack_str( buffer, (char *)buf, fWidth ), f );
    }
    fwrite( pal_pcx, 1, (256 * 3 + 1), f );

    fclose( f );
    delete [] buffer;
*/
}


void TContourImage :: coastal_lineBinaryImage_withThickness( int number_gcp, uint8_t thickness, const char * b_image, char sea, char land, const char * cloudy_mask,
                                                             long width, long height, char mark_sea, char mark_land,
                                                             char mark_cloudy_sea, char mark_cloudy_land, bool flag )
{
    fBinaryImage = (char *)b_image;
    fCloudyMask = (char *)cloudy_mask;

    fWidth = width;
    fHeight = height;

    bool flagland = false;
    for( long i = 0; i < fHeight; i++ ){
         char * bimage_p = fBinaryImage + i * fWidth;
         char * mcloudy_p = fCloudyMask + i * fWidth;
         flagland = false;
         for( long j = 0; j < fWidth; j++ ){
              if( flagland == false && j == 0 && bimage_p[j] == land ) flagland = true;
              if( flagland == false && bimage_p[j] == land ){
                  for( int ig = 0; ig < thickness; ig++ ){
                      if( ( j + ig ) < fWidth && bimage_p[ j + ig ] == land ){
                          if( mcloudy_p[ j + ig ] ) bimage_p[ j + ig ] = mark_cloudy_land;
                          else bimage_p[ j + ig ] = mark_land;
                      }
                      if( j - ig - 1 >= 0 && bimage_p[j - ig - 1] == sea ){
                          if( mcloudy_p[j - ig - 1] )  bimage_p[j - ig - 1] = mark_cloudy_sea;
                          else bimage_p[j - ig - 1] = mark_sea;
                      }
                  }

                  j += thickness - 1;
                  if( j + 1 >= fWidth ) break;
                  if( bimage_p[j + 1] == land ) flagland = true;
                  else  flagland = false;
                  continue;
              }
              if( flagland == true && bimage_p[j] == sea ){
                  for( int ig = 0; ig < thickness; ig++ ){
                      if( ( j + ig ) < fWidth && bimage_p[ j + ig ] == sea ){
                          if( mcloudy_p[ j + ig ] ) bimage_p[ j + ig ] = mark_cloudy_sea;
                          else bimage_p[ j + ig ] = mark_sea;
                      }
                      if( ( j - ig - 1 ) >= 0 && bimage_p[ j - ig - 1 ] == land ){
                          if( mcloudy_p[ j - ig - 1 ] ) bimage_p[ j - ig - 1 ] = mark_cloudy_land;
                          else bimage_p[ j - ig - 1 ] = mark_land;
                      }
                  }

                  j += thickness - 1;
                  if( j + 1 >= fWidth ) break;
                  if( bimage_p[j + 1] == land ) flagland = true;
                  else  flagland = false;
                  continue;
              }

         }
    }

    for( long j = 0; j < fWidth; j++ ){
         char * bimage_p = fBinaryImage + j;
         char * mcloudy_p = fCloudyMask + j;
         flagland = false;
         for( long i = 0; i < fHeight; i++ ){
              if( flagland == false && i == 0 && (bimage_p[i * fWidth] == land || bimage_p[i * fWidth] == mark_land) ) flagland = true;
              if( flagland == false && (bimage_p[i * fWidth] == land || bimage_p[i * fWidth] == mark_land) ){
                  for( int ig = 0; ig < thickness; ig++ ){
                      if( ( i + ig ) < fHeight && bimage_p[( i + ig ) * fWidth] == land ){
                          if( mcloudy_p[( i + ig ) * fWidth] ) bimage_p[( i + ig ) * fWidth] = mark_cloudy_land;
                          else bimage_p[( i + ig ) * fWidth] = mark_land;
                      }
                      if( ( i - ig - 1 ) >= 0 && bimage_p[( i - ig - 1 ) * fWidth] == sea ){
                          if( mcloudy_p[( i - ig - 1 ) * fWidth] ) bimage_p[( i - ig - 1 ) * fWidth] = mark_cloudy_sea;
                          else bimage_p[( i - ig - 1 ) * fWidth] = mark_sea;
                      }
                  }

                  i += thickness - 1;
                  if( i + 1 >= fHeight ) break;
                  if( bimage_p[(i + 1) * fWidth] == land || bimage_p[(i + 1) * fWidth] == mark_land ) flagland = true;
                  else  flagland = false;
                  continue;
              }
              if( flagland == true && (bimage_p[i * fWidth] == sea || bimage_p[i * fWidth] == mark_sea) ){
                  for( int ig = 0; ig < thickness; ig++ ){
                      if( ( i + ig ) < fHeight && bimage_p[( i + ig ) * fWidth] == sea ){
                          if( mcloudy_p[( i + ig ) * fWidth] ) bimage_p[( i + ig ) * fWidth] = mark_cloudy_sea;
                          else bimage_p[( i + ig ) * fWidth] = mark_sea;
                      }
                      if( ( i - ig - 1 ) >= 0 && bimage_p[( i - ig - 1 ) * fWidth] == land ){
                          if( mcloudy_p[( i - ig - 1 ) * fWidth] ) bimage_p[( i - ig - 1 ) * fWidth] = mark_cloudy_land;
                          else bimage_p[( i - ig - 1 ) * fWidth] = mark_land;
                      }
                  }

                  i += thickness - 1;
                  if( i + 1 >= fHeight ) break;
                  if( bimage_p[(i + 1) * fWidth] == land || bimage_p[(i + 1) * fWidth] == mark_land ) flagland = true;
                  else  flagland = false;
                  continue;
              }
         }
    }
/*
    // Собственно запись PCX-файла.
    TPCXHdr hdr;
    pcx_ini_hdr_256( &hdr, fWidth, fHeight );

    char pal_pcx[256 * 3 + 1];
    pal_pcx[0] = 12;
    PCXRGB * pal = (PCXRGB *)(pal_pcx + 1);
    memset( pal, 0, sizeof( PCXRGB ) * 256 );

    pal[land].r = 255; pal[land].g = 255; pal[land].b = 255;   // суша.
    pal[sea].r = 0; pal[sea].g = 0; pal[sea].b = 0;           // море.
    pal[mark_sea].r = 0; pal[mark_sea].g = 0; pal[mark_sea].b = 255;      // контурные пикселы - море.
    pal[mark_land].r = 0; pal[mark_land].g = 255; pal[mark_land].b = 0;  // контурные пикселы - суша.
    pal[mark_cloudy_sea].r = 255; pal[mark_cloudy_sea].g = 0; pal[mark_cloudy_sea].b = 255;     // контурные пикселы (покрытые облачностью) - море.
    pal[mark_cloudy_land].r = 255; pal[mark_cloudy_land].g = 255; pal[mark_cloudy_land].b = 0; // контурные пикселы (покрытые облачностью) - суша.

    char * buf = fBinaryImage;
    char * buffer = new char [fWidth * 2];	// Используется для хранения одной строки PCX-данных.

    // Создание имени PCX-файла.
    char drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME] = "cont", file_path[_MAX_PATH];
    _splitpath( fCS.fGPCXDir, drive, dir, 0, 0 );

    int l = strlen( fname );
    char buf_str[7], *p = fname + l;
    if( flag ) sprintf( buf_str, "our_%d", number_gcp );
    else sprintf( buf_str, "temp_%d", number_gcp );
    strcpy( p, buf_str );

    _makepath( file_path, drive, dir, fname, "pcx" );

    FILE *f = fopen( file_path, "wb" );
    fwrite( &hdr, 1, 128, f );

    for( unsigned long i = 0; i < fHeight; i++, buf += fWidth ){
         // Построчное PCX-кодирование данных из буфера картинки и запись их в файл.
        fwrite( buffer, 1, pcx_pack_str( buffer, (char *)buf, fWidth ), f );
    }
    fwrite( pal_pcx, 1, (256 * 3 + 1), f );

    fclose( f );
    delete [] buffer;
*/
}

/*-------------------------------------------------------------------------
    aufit_mask.cpp
    Реализация класса TAufitChipMask.
-------------------------------------------------------------------------*/

#include <unistd.h>
#include <tc_config.h>
#include <aufitchip.h>
#include <aufit_mask.hpp>
#include <contour.hpp>
#include <astronom.hpp>
#include <tgdb.hpp>
#include <kr_stjudent.hpp>

using namespace std;
void errorMessage( const char * error_message_text );
void print_log( const char * message_text ) ;

TAufitChipMask :: TAufitChipMask( string &fMaskChipBaseFileDir, long scale, long navigation_error_line, long navigation_error_column,
								  TNOAAImageParams * nip, TIniSatParams * isp, TStraightReferencer * sr, TInverseReferencer * ir, const TCorrectionParams & cop,
								  long nrfc, char * mask_cloudy, bool channel1, short * calibr_data1, short max_value1, double Ka1, double Kb1,
								  bool channel2, short * calibr_data2, short max_value2, double Ka2, double Kb2 ) :

maskScale( scale ),
maxShiftLine( navigation_error_line ),
maxShiftColumn( navigation_error_column ),
fNIP( nip ),
fISP( isp ),
fSR( sr ),
fIR( ir ),
fCOP( cop ),
fCloudyMask( mask_cloudy ),
fNumberRegionsforFiltrCloudy( nrfc ),
fChipType( TDChip::GraundChip ),
maskBuf( 0 ),
fragmX1( 0 ),
fragmY1( 0 ),
fragmX2( 0 ),
fragmY2( 0 ),
fragmWidth( 0 ),
fragmHeight( 0 ),
maskWidth( 0 ),
maskHeight( 0 ),
avg_largeChips_dX( .0 ),
avg_largeChips_dY( .0 ),
quantity_largeChips( 0 ),
minCost00( 1e50 ),
clearPixels( .0 ),
flagMaskCloudClear( false )
{
	calibrData[0] = calibr_data1;
	calibrData[1] = calibr_data2;

	fChannels_forCalcGCPs[0] = channel1;
	fChannels_forCalcGCPs[1] = channel2;

	maxPixValue[0] = max_value1;
	maxPixValue[1] = max_value2;

	ka[0] = Ka1;
	ka[1] = Ka2;
	kb[0] = Kb1;
	kb[1] = Kb2;

	try {
		loadDataMask( fMaskChipBaseFileDir );

		fChipList = new TChipList(); // QUANTITY_CHIPS_ON_IMAGE
		iniChipList( fMaskChipBaseFileDir );
	} catch( TAccessExc ae ) {
		errorMessage( ae.what() );
	} catch( TParamExc pe ) {
		errorMessage( pe.what() );
	}
}


TAufitChipMask :: ~TAufitChipMask() {
	if( maskBuf != 0 ) {
		delete [] maskBuf;
		maskBuf = 0;
	}

	delete fPM;
	delete fMFH;
	delete [] fDataMask;

	removeAllChips();
	delete fChipList;
}


void TAufitChipMask :: loadDataMask( string &fMaskFileDir ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка доступа к файлу данных региональной маски суша/море!!!" );

	// Конструирование полного имени файла маски.
	// Проверка существования каталога fMaskandChipBaseDir.
	char drive[MAX_DRIVE], dir[MAX_DIR], /*fname[MAX_FNAME],*/ path[MAX_PATH];
	if( check_dir( fMaskFileDir.c_str() ) == 0 )
		#warning ацкое зло, но мне щас лень думать
		splitpath(const_cast<char *>(fMaskFileDir.c_str()), drive, dir, 0, 0 );
	else {
		getcwd( path, MAX_PATH );
		int t = strlen(path);
		if( path[t-1] != DIRD ) {
			path[t] = DIRD;
			path[t+1] = '\0';
		}
		splitpath( path, drive, dir, 0, 0 );
	}
	makepath( path, drive, dir, "region", "msk" );

	FILE * f;
	if( !(f = fopen( path, "rb" )) ) {
		throw ae1;
	}

	// Чтение заголовка файла региональной маски.
	fMFH = new TMaskFileHdr();
	fread( fMFH, 1, sizeof( TMaskFileHdr ), f );

	fPM = new TProjMapper( TProjMapper::eqd, double(fMFH->lon)*DR, double(fMFH->lat)*DR, double(fMFH->lonSize)*DR, double(fMFH->latSize)*DR, (double(fMFH->lonSize)*DR)/double(fMFH->pixNum), (double(fMFH->latSize)*DR)/double(fMFH->scanNum) );

	// Чтение данных маски региона.
	fDataMask = new char[fMFH->scanNum * fMFH->pixNum];
	fread( fDataMask, 1, (fMFH->scanNum * fMFH->pixNum), f );

	fclose( f );
}


void TAufitChipMask :: iniChipList( std::string &fBaseChipsFileDir ) throw( TAccessExc, TParamExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка доступа к файлу данных базы чипов!!!" );
	TParamExc pe1( 1, "ERROR: Косяки в базе чипов?!!" );

	char drive[MAX_DRIVE], dir[MAX_DIR], /*fname[MAX_FNAME],*/ path[MAX_PATH];
	// Конструирование полного имени файла базы чипов.
	// Проверка существования каталога fMaskandChipBaseDir.
	if( check_dir( fBaseChipsFileDir.c_str() ) == 0 )
		#warning ацкое зло, но мне щас лень думать
		splitpath(const_cast<char *>(fBaseChipsFileDir.c_str()), drive, dir, 0, 0 );
	else {
		getcwd( path, MAX_PATH );
		int t = strlen(path);
		if( path[t-1] != DIRD ) {
			path[t] = DIRD;
			path[t+1] = '\0';
		}
		splitpath( path, drive, dir, 0, 0 );
	}
	makepath( path, drive, dir, "chipbase", "box" );

	FILE * f;
	if( !(f = fopen( path, "r" )) ) {
		throw ae1;
	}

	char tempStr[ 501 ];

	unsigned long n=0;
	char * pointer = fgets( tempStr, 10, f );
	sscanf( tempStr, "%lu", &n );

	if ( pointer == 0 || n == 0 ) {
		fclose( f );
		throw pe1;
	}

	TDChip chip;
	double lon_c, lat_c;
	TAstronom fAstronom;
	for( unsigned long i = 0; i < n; i++ ) {
		pointer = fgets( tempStr, 500, f );

		if ( pointer == 0 ) {
			fclose( f );
			throw ae1;
		}
		unsigned reg;
		/*char * string = chip.fStringQuality;*/
		sscanf( tempStr, "%ld\t %ld\t %ld\t %ld\t %d\t %hhd\t", &(chip.mX1), &(chip.mY1), &(chip.mX2), &(chip.mY2), &reg, &(chip.Ball) );
		if( chip.Ball <= 5 )
			continue;
		chip.fRegionFiltrCloudy = uint8_t(reg);

		if( ( chip.mX1 >= fMFH->pixNum ) || ( chip.mX2 >= fMFH->pixNum ) || ( chip.mY1 >= fMFH->scanNum ) ||
				( chip.mY2 >= fMFH->scanNum ) || ( chip.mX1 >= chip.mX2 ) || ( chip.mY1 >= chip.mY2 ) ||
				( chip.fRegionFiltrCloudy >= fNumberRegionsforFiltrCloudy ) || ( chip.Ball > 10 ) ) {
			throw pe1;
		}

		int l=0, c=0;
		while( c != 6 ) {
			if( tempStr[l] == '\t' )
				c++;
			l++;
		}
		c = 0;
		while( tempStr[l + 1 + c] != '\n' )
			c++;

		strcpy( chip.fStringQuality, &tempStr[l + 1] );
		chip.fStringQuality[c] = '\0';

		double lon[2], lat[2];
		lon[0] = fPM->lon( (unsigned long)chip.mX1 );
		lon[1] = fPM->lon( (unsigned long)chip.mX2 );
		lat[0] = fPM->lat( (unsigned long)chip.mY1 );
		lat[1] = fPM->lat( (unsigned long)chip.mY2 );

		int x[4], y[4];
		if( fIR->ll2xy( lon[0], lat[0], &x[0], &y[0] ) == 0 )
			continue;
		if( fIR->ll2xy( lon[1], lat[1], &x[1], &y[1] ) == 0 )
			continue;
		if( fIR->ll2xy( lon[0], lat[1], &x[2], &y[2] ) == 0 )
			continue;
		if( fIR->ll2xy( lon[1], lat[0], &x[3], &y[3] ) == 0 )
			continue;
		chip.X1 = chip.X2 = x[3];
		chip.Y1 = chip.Y2 = y[3];

		for( int k=0; k<3; k++) {
			if( chip.X1 > x[k] )
				chip.X1 = x[k];
			if( chip.X2 < x[k] )
				chip.X2 = x[k];
			if( chip.Y1 > y[k] )
				chip.Y1 = y[k];
			if( chip.Y2 < y[k] )
				chip.Y2 = y[k];
		}

		if( chip.X1 - maxShiftColumn <= 0 || chip.X2 + maxShiftColumn >= 2047 ||
				chip.Y1 - maxShiftLine <= 0 || chip.Y2 + maxShiftLine >= (fNIP->fScans - 1) )
			continue;

		if( chip.X1 > chip.X2 ) {
			long l = chip.X2;
			chip.X2 = chip.X1;
			chip.X1 = l;
		}
		if( chip.Y1 > chip.Y2 ) {
			long l = chip.Y2;
			chip.Y2 = chip.Y1;
			chip.Y1 = l;
		}

		TDChip * real_chip = new TDChip( chip );
		if( maxPixValue[0] > 0 ) {
			if( !check_GraundChipOnBeatenPixels( 0, real_chip ) ) {
				delete real_chip;
			} else {
				real_chip->fChannels_forCalcShifts[0] = true;
				real_chip->fChipType[0] = TDChip::GraundChip;
				fSR->xy2ll( double(real_chip->X1 + real_chip->ChipSize().x()/2), double(real_chip->Y1 + real_chip->ChipSize().y()/2), &lon_c, &lat_c );
				lon_c = -lon_c;
				real_chip->fAngleCenterSanny = fAstronom.shcrds( fAstronom.julian( fNIP->fYear, fNIP->fYearTime + 1 ), lon_c, lat_c );
				if( fChannels_forCalcGCPs[1] ) {
					if( (real_chip->fAngleCenterSanny > .0) ) {
						real_chip->fChannels_forCalcShifts[1] = true;
						real_chip->fChipType[1] = TDChip::GraundChip;
					}
				}
				fChipList->addAsLast( real_chip );
			}
		} else {
			real_chip->fChannels_forCalcShifts[0] = true;
			real_chip->fChipType[0] = TDChip::CheckChip;
			real_chip->fChannels_forCalcShifts[1] = true;
			real_chip->fChipType[1] = TDChip::CheckChip;
			fChipList->addAsLast( real_chip );
		}
	}

	fclose( f );
}


void TAufitChipMask :: resultMask_to_pcx( char file_path[MAX_PATH], TDChip* chip, unsigned long N, uint8_t pch ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия pcx-файла для записи результата наложения маски суша/море на реперный участок изображения!!!" );

	fragmX1 = chip->X1;
	fragmY1 = chip->Y1;
	fragmX2 = chip->X2;
	fragmY2 = chip->Y2;
	fragmWidth = fragmX2 - fragmX1 + 1;
	fragmHeight = fragmY2 - fragmY1 + 1;

	//    maskWidth = ( fragmWidth + 2 * maxShiftColumn ) * maskScale;
	//    maskHeight = ( fragmHeight + 2 * maxShiftLine ) * maskScale;

	buildMaskChip( chip );

	uint8_t * pic_buf = new uint8_t[maskWidth * maskHeight];

	short min_value=0, max_value=0;
	long shift_x = long( chip->dX[pch] * maskScale ), shift_y = long( chip->dY[pch] * maskScale );

	// вычисление минимального и максимального значений пикселов в фрагменте
	data_find_min_max( &min_value, &max_value, pch,
					   fragmX1 - maxShiftColumn + shift_x/maskScale, fragmY1 - maxShiftLine + shift_y/maskScale,
					   fragmX2 + maxShiftColumn + shift_x/maskScale, fragmY2 + maxShiftLine + shift_y/maskScale );

	unsigned long * hist = new unsigned long [max_value - min_value + 1];

	hist_build_m( hist, pch,
				  fragmX1 - maxShiftColumn + shift_x/maskScale, fragmY1 - maxShiftLine + shift_y/maskScale,
				  fragmX2 + maxShiftColumn + shift_x/maskScale, fragmY2 + maxShiftLine + shift_y/maskScale,
				  min_value, max_value );

	short v_l = min_value, v_u = max_value;

	// оцениваем величины верхнего и нижнего порогов значений пикселов изображения
	hist_calc_q( &v_l, .05, hist, min_value, max_value );
	hist_calc_q( &v_u, .95, hist, min_value, max_value );

	delete [] hist;

	if ( v_l >= v_u ) {
		v_l = min_value;
		v_u = max_value;
	}

	// построение таблицы преобразования значений пикселов изображения для битмэпа
	uint8_t * lookup = new uint8_t[max_value+100];
	uint8_t * lp = lookup + 100;
	memset( lookup, 128, 100 );
	*( lp - 5 ) = 255;
	memset( &lookup[100], 0, v_l );
	memset( &lp[v_u + 1], 254, max_value - v_u );
	double c;
	if ( v_l == v_u )
		c = 254.;
	else
		c = 254. / ( v_u - v_l + 1 );

	for( long i = v_l; i<(v_u + 1); i++ )
		lp[i] = uint8_t( c * ( i - v_l ) + 0.5 );

	// преобразование значений пикселов для битмэпа
	uint8_t * pic_p = pic_buf;
	for( long i = 0; i < maskHeight; i++ ) {
		long data_y = fragmY1 - maxShiftLine + (i + shift_y)/maskScale;
		short * data_p = calibrData[pch] + 2048 * data_y;
		for( long j = 0; j < maskWidth; j++ ) {
			long data_x = fragmX1 - maxShiftColumn + (j + shift_x)/maskScale;
			if( fCloudyMask[2048 * data_y+data_x] == 1 )
				*pic_p++ = 255;
			else
				*pic_p++ = lp[ data_p[data_x] ];
		}
	}

	delete [] lookup;

	// инвертирование пикселов, накрываемых маской суши
	char * pmask = maskBuf;
	pic_p = pic_buf;
	uint8_t * pic_p_end = pic_p + maskWidth * maskHeight;

	do {
		if( *pmask == 1 && *pic_p != 255 )
			*pic_p = 254 - *pic_p;
		pmask++;
	} while( ++pic_p != pic_p_end );

	// Собственно запись PCX-файла.
	TPCXHdr hdr;
	pcx_ini_hdr_256( &hdr, maskWidth, maskHeight );

	char pal_pcx[256 * 3 + 1];
	pal_pcx[0] = 12;
	PCXRGB * pal = (PCXRGB *)(pal_pcx + 1);
	memset( pal, 0, sizeof( PCXRGB ) * 256 );
	for( int i=0; i<255; i++ )
		pal[i].r = pal[i].g = pal[i].b = i+1;
	pal[255].r = 255;
	pal[255].g = 0;
	pal[255].b = 0;   // облачность.

	uint8_t * buf;

	if( fNIP->fAscendFlag ) {
		buf = new uint8_t [maskWidth * maskHeight];
		uint8_t * dst_base = buf + maskWidth * (maskHeight - 1);
		uint8_t * dst = dst_base;
		uint8_t * dst_end = buf;
		uint8_t * src_base = pic_buf;
		uint8_t * src = src_base + maskWidth;

		while( dst != dst_end ) {
			*dst++ = *--src;
			if( src == src_base ) {
				src_base += maskWidth;
				src = src_base + maskWidth;
				dst_base -= maskWidth;
				dst = dst_base;
			}
		}
		pic_p = buf;
	} else {
		pic_p = pic_buf;
	}

	char * buffer = new char [maskWidth * 2];	// Используется для хранения одной строки PCX-данных.

	FILE * f;
	if( !(f = fopen( file_path, "wb" )) ) {
		throw ae1;
	}

	fwrite( &hdr, 1, 128, f );

	for( long i = 0; i < maskHeight; i++, pic_p += maskWidth ) {
		// Построчное PCX-кодирование данных из буфера картинки и запись их в файл.
		fwrite( buffer, 1, pcx_pack_str( buffer, (char *)pic_p, maskWidth ), f );
	}
	fwrite( pal_pcx, 1, (256 * 3 + 1), f );
	fclose( f );

	delete [] buffer;
	delete [] pic_buf;
	if( fNIP->fAscendFlag )
		delete [] buf;
}


void TAufitChipMask :: resultContour_to_pcx(  char file_path[MAX_PATH], TDChip* chip, unsigned long N, uint8_t pch ) throw( TAccessExc ) {
	TAccessExc ae1( 1, "ERROR: Ошибка открытия pcx-файла для записи результата наложения контура на реперный участок изображения!!!" );

	fragmX1 = chip->X1;
	fragmY1 = chip->Y1;
	fragmX2 = chip->X2;
	fragmY2 = chip->Y2;
	fragmWidth = fragmX2 - fragmX1 + 1;
	fragmHeight = fragmY2 - fragmY1 + 1;

	maskWidth = ( fragmWidth + 2 * maxShiftColumn ) * maskScale;
	maskHeight = ( fragmHeight + 2 * maxShiftLine ) * maskScale;

	uint8_t * pic_buf = new uint8_t[maskWidth * maskHeight];

	short min_value=0, max_value=0;

	long shift_x = long( chip->dX[pch] * maskScale );
	long shift_y = long ( chip->dY[pch] * maskScale );

	// вычисление минимального и максимального значений пикселов в фрагменте
	data_find_min_max( &min_value, &max_value, pch,
					   fragmX1 - maxShiftColumn + shift_x/maskScale, fragmY1 - maxShiftLine + shift_y/maskScale,
					   fragmX2 + maxShiftColumn + shift_x/maskScale, fragmY2 + maxShiftLine + shift_y/maskScale );

	unsigned long * hist = new unsigned long [max_value - min_value + 1];

	hist_build_m( hist, pch,
				  fragmX1 - maxShiftColumn + shift_x/maskScale, fragmY1 - maxShiftLine + shift_y/maskScale,
				  fragmX2 + maxShiftColumn + shift_x/maskScale, fragmY2 + maxShiftLine + shift_y/maskScale,
				  min_value, max_value );

	short v_l = min_value, v_u = max_value;

	// оцениваем величины верхнего и нижнего порогов значений пикселов изображения
	hist_calc_q( &v_l, .05, hist, min_value, max_value );
	hist_calc_q( &v_u, .95, hist, min_value, max_value );

	delete [] hist;

	if ( v_l >= v_u ) {
		v_l = min_value;
		v_u = max_value;
	}

	// построение таблицы преобразования значений пикселов изображения для битмэпа
	uint8_t * lookup = new uint8_t[max_value+100];
	uint8_t * lp = lookup + 100;
	memset( lookup, 128, 100 );
	//    *( lp - 1 ) = 253;
	//    *( lp - 4 ) = 254;
	*( lp - 5 ) = 255;
	memset( &lookup[100], 0, v_l );
	memset( &lp[v_u + 1], 252, max_value - v_u );
	double c;
	if ( v_l == v_u )
		c = 252.;
	else
		c = 252. / ( v_u - v_l + 1 );

	for( long i = v_l; i<(v_u + 1); i++ )
		lp[i] = uint8_t( c * ( i - v_l ) + 0.5 );

	// преобразование значений пикселов для битмэпа
	uint8_t * pic_p = pic_buf;
	for( long i = 0; i < maskHeight; i++ ) {
		long data_y = fragmY1 - maxShiftLine + (i + shift_y)/maskScale;
		short * data_p = calibrData[pch] + 2048 * data_y;
		for( long j = 0; j < maskWidth; j++ ) {
			long data_x = fragmX1 - maxShiftColumn + (j + shift_x)/maskScale;
			if( fCloudyMask[2048 * data_y+data_x] == 1 )
				*pic_p++ = 255;
			else
				*pic_p++ = lp[ data_p[data_x] ];
		}
	}

	delete [] lookup;

	TGDB * gdb = new TGDB;
	gdb->again();

	// массивы x- и y-компонент координат точек ломаной
	long * xarr = new long [gdb->maxSegmentLength()];
	long * yarr = new long [gdb->maxSegmentLength()];

	int prevX = -1, prevY = -1;
	uint32_t point_count = 0;

	double d_x, d_y;
	long l_x, l_y;

	while( gdb->nextSegment() == 0 ) {
		TGDBPoint * segment_points = gdb->segmentPoints();
		// цикл по точкам сегмента
		for( int i = 0; i < gdb->segmentLength(); i++ ) {
			// если точка попадает в снимок
			if( fIR->ll2xy( double(segment_points[i].lon)*DR/3600., double(segment_points[i].lat)*DR/3600., &d_x, &d_y ) == 1
					&& d_x >= double( fragmX1 - maxShiftColumn ) && d_x < double( fragmX2 + maxShiftColumn ) && d_y >= double( fragmY1 - maxShiftLine ) && d_y < double( fragmY2 + maxShiftLine ) ) {
				l_x = long((d_x - double( fragmX1 - maxShiftColumn )) * maskScale);
				l_y = long((d_y - double( fragmY1 - maxShiftLine )) * maskScale);
				if( l_x != prevX || l_y != prevY ) {
					prevX = l_x;
					prevY = l_y;
					xarr[point_count] = l_x;
					yarr[point_count] = l_y;
					point_count++;
				}
			} else {
				if( point_count > 1 ) {
					// отрисовка накопленного сегмента
					for( long i = 1; i < point_count; i++ ) {
						vector_mem_8( xarr[i-1], yarr[i-1], xarr[i], yarr[i], 253, pic_buf, maskWidth );
					}

					for( long i = 0; i < point_count; i++ ) {
						pic_buf[ yarr[i]*maskWidth + xarr[i] ] = 254;
					}

				}

				if( point_count ) {
					prevX = prevY = -1;
					point_count = 0;
				}
			}
		}

		if( point_count > 1 ) {
			// отрисовка накопленного сегмента
			for( long i = 1; i < point_count; i++ ) {
				vector_mem_8( xarr[i-1], yarr[i-1], xarr[i], yarr[i], 253, pic_buf, maskWidth );
			}

			for( long i = 0; i < point_count; i++ ) {
				pic_buf[ yarr[i]*maskWidth + xarr[i] ] = 254;
			}
		}

		if( point_count ) {
			prevX = prevY = -1;
			point_count = 0;
		}
	}

	delete gdb;
	delete [] xarr;
	delete [] yarr;

	// Собственно запись PCX-файла.
	TPCXHdr hdr;
	pcx_ini_hdr_256( &hdr, maskWidth, maskHeight );

	char pal_pcx[256 * 3 + 1];
	pal_pcx[0] = 12;
	PCXRGB * pal = (PCXRGB *)(pal_pcx + 1);
	memset( pal, 0, sizeof( PCXRGB ) * 256 );

	for( int i=0; i<253; i++ )
		pal[i].r = pal[i].g = pal[i].b = i + 3;
	pal[253].r = 0;
	pal[253].g = 0;
	pal[253].b = 255;    // береговой контур - ломанные.
	pal[254].r = 0;
	pal[254].g = 255;
	pal[254].b = 0;   // береговой контур - узлы.
	pal[255].r = 255;
	pal[255].g = 0;
	pal[255].b = 0;  // облачность.

	uint8_t * buf;
	if( fNIP->fAscendFlag ) {
		buf = new uint8_t [maskWidth * maskHeight];
		uint8_t * dst_base = buf + maskWidth * (maskHeight - 1);
		uint8_t * dst = dst_base;
		uint8_t * dst_end = buf;
		uint8_t * src_base = pic_buf;
		uint8_t * src = src_base + maskWidth;

		while( dst != dst_end ) {
			*dst++ = *--src;
			if( src == src_base ) {
				src_base += maskWidth;
				src = src_base + maskWidth;
				dst_base -= maskWidth;
				dst = dst_base;
			}
		}
		pic_p = buf;
	} else {
		pic_p = pic_buf;
	}

	char * buffer = new char [maskWidth * 2];	// Используется для хранения одной строки PCX-данных.

	FILE * f;
	if( !(f = fopen( file_path, "wb" )) ) {
		throw ae1;
	}

	fwrite( &hdr, 1, 128, f );

	for( unsigned long i = 0; i < maskHeight; i++, pic_p += maskWidth ) {
		// Построчное PCX-кодирование данных из буфера картинки и запись их в файл.
		fwrite( buffer, 1, pcx_pack_str( buffer, (char *)pic_p, maskWidth ), f );
	}
	fwrite( pal_pcx, 1, (256 * 3 + 1), f );
	fclose( f );

	delete [] buffer;
	delete [] pic_buf;
	if( fNIP->fAscendFlag )
		delete [] buf;
}


bool TAufitChipMask :: result_MaskorContourChips_to_pcx( uint8_t option_chips_to_pcx,  string& fGPCXDir, string& fDataFilePath) throw( TAccessExc ) {
	switch( option_chips_to_pcx ) {
	case 1:
		print_log( "Запись результатов (в виде pcx-файлов) наложения маски суша/море\nдля чипов изображения без фильтрации облачности." );
		break;
	case 2:
		print_log( "Запись результатов (в виде pcx-файлов) наложения берегового контура\nдля чипов изображения без фильтрации облачности." );
		break;
	case 3:
		print_log( "Запись результатов (в виде pcx-файлов) наложения маски суша/море\nдля чипов изображения с фильтрацией облачности." );
		break;
	case 4:
		print_log( "Запись результатов (в виде pcx-файлов) наложения берегового контура\nдля чипов изображения с фильтрацией облачности." );
		break;
	}

	//TAufitChipMask::ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();

	unsigned long i = 0, j = 0, N = numberOfGraundChips();   // Число чипов.

	if( N <= 0 ) {
		print_log( "WARNING: Отсутствуют чипы на изображении!" );
		return false;
	}

	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], file_path[MAX_PATH];

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
		TDChip* chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip ) {
			i++;
			// Создание имени PCX-файла.
			#warning Ацкое зло, ломы щас думать
			splitpath( const_cast<char *>(fDataFilePath.c_str()), drive, dir, fname, 0 );
			int l = strlen( fname );
			char *p = fname + l;
			sprintf( p, "_%d_%ld", 4, i );
			#warning Ацкое зло, ломы щас думать
			splitpath( const_cast<char *>(fGPCXDir.c_str()), drive, dir, 0, 0 );
			makepath( file_path, drive, dir, fname, "pcx" );

			try {
				if( option_chips_to_pcx == 2 || option_chips_to_pcx == 4 )
					resultContour_to_pcx(  file_path, chip, i, 0 );
				if( option_chips_to_pcx == 1 || option_chips_to_pcx == 3 )
					resultMask_to_pcx( file_path, chip, i, 0 );
			} catch ( TAccessExc & ae1 ) {
				throw;
			}
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			j++;
			// Создание имени PCX-файла.
			#warning Ацкое зло, ломы щас думать
			splitpath( const_cast<char *>(fDataFilePath.c_str()), drive, dir, fname, 0 );
			int l = strlen( fname );
			char *p = fname + l;
			sprintf( p, "_%d_%ld", 2, j );
			#warning Ацкое зло, ломы щас думать
			splitpath( const_cast<char *>(fGPCXDir.c_str()), drive, dir, 0, 0 );

			makepath( file_path, drive, dir, fname, "pcx" );

			try {
				if( option_chips_to_pcx == 2 || option_chips_to_pcx == 4 )
					resultContour_to_pcx(  file_path, chip, j, 1 );
				if( option_chips_to_pcx == 1 || option_chips_to_pcx == 3 )
					resultMask_to_pcx( file_path, chip, j, 1 );
			} catch ( TAccessExc & ae1 ) {
				throw;
			}
		}
	}

	return true;
}


bool TAufitChipMask :: result_MaskorContour_GraundChipsorautoGCPs_to_pcx( bool graund_or_gcp, bool mask_or_contour,  string &fGPCXDir, string &fDataFilePath) throw( TAccessExc ) {
	if( !graund_or_gcp ) {
		if( !mask_or_contour )
			print_log( "Запись результатов (в виде pcx-файлов) наложения берегового контура\nс использованием вычисленных смещений опорных чипов." );
		else
			print_log( "Запись результатов (в виде pcx-файлов) наложения маски суша/море\nс использованием вычисленных смещений опорных чипов." );
	} else {
		if( !mask_or_contour )
			print_log( "Запись результатов (в виде pcx-файлов) наложения берегового контура\nс использованием вычисленных смещений autoGCPs." );
		else
			print_log( "Запись результатов (в виде pcx-файлов) наложения маски суша/море\nс использованием вычисленных смещений autoGCPs." );
	}

	//TAufitChipMask::ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();
	unsigned long i = 0, j = 0, N = numberOfGraundChips();   // Число опорных чипов или autoGCPs.

	if( N <= 0 ) {
		print_log( !graund_or_gcp  ? "WARNING: Отсутствуют опорные чипы!" :"WARNING: Отсутствуют autoGCPs!"  );
		return false;
	}

	char drive[MAX_DRIVE], dir[MAX_DIR], fname[MAX_FNAME], file_path[MAX_PATH];

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
		TDChip* chip = cursor.elementAt();

		if( chip->fChipType[0] == TDChip::GraundChip ) {
			i++;
			// Создание имени PCX-файла.
			#warning ацкое зло, но думать ломы
			splitpath( const_cast<char *>(fDataFilePath.c_str()), drive, dir, fname, 0 );
			int l = strlen( fname );
			char *p = fname + l;
			sprintf( p, "_%d_%ld", 4, i );
			#warning ацкое зло, но думать ломы
			splitpath( const_cast<char *>(fGPCXDir.c_str()), drive, dir, 0, 0 );
			if( !graund_or_gcp ) {
				l = strlen( dir );
				p = dir + l;
				sprintf( p, "GRAUND_CHIPS\\" );
			} else {
				l = strlen( dir );
				p = dir + l;
				sprintf( p, "AUTO_GCPs\\" );
			}
			makepath( file_path, drive, dir, fname, "pcx" );
			try {
				if( !mask_or_contour )
					resultContour_to_pcx(  file_path, chip, i, 0 );
				else
					resultMask_to_pcx( file_path, chip, i, 0 );
			} catch ( TAccessExc & ae1 ) {
				throw;
			}
		}
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			j++;
			// Создание имени PCX-файла.
			#warning ацкое зло, но думать ломы
			splitpath(const_cast<char *>(fDataFilePath.c_str()), drive, dir, fname, 0 );
			int l = strlen( fname );
			char *p = fname + l;
			sprintf( p, "_%d_%ld", 2, j );
			#warning ацкое зло, но думать ломы
			splitpath(const_cast<char *>(fGPCXDir.c_str()), drive, dir, 0, 0 );
			if( !graund_or_gcp ) {
				l = strlen( dir );
				p = dir + l;
				sprintf( p, "GRAUND_CHIPS\\" );
			} else {
				l = strlen( dir );
				p = dir + l;
				sprintf( p, "AUTO_GCPs\\" );
			}
			makepath( file_path, drive, dir, fname, "pcx" );
			try {
				if( !mask_or_contour )
					resultContour_to_pcx(  file_path, chip, j, 1 );
				else
					resultMask_to_pcx( file_path, chip, j, 1 );
			} catch ( TAccessExc & ae1 ) {
				throw;
			}
		}
	}

	return true;
}


void TAufitChipMask :: settingAvgShifts( long quantitylarge, double avgdX, double avgdY ) {
	quantity_largeChips = quantitylarge;
	avg_largeChips_dX = avgdX / (double)quantitylarge;
	avg_largeChips_dY = avgdY / (double)quantitylarge;
	maxShiftLine /= 3;
	maxShiftColumn /= 3;
}


void TAufitChipMask :: settingShiftsSearchZone( long shiftLine, long shiftColumn ) {
	maxShiftLine = shiftLine;
	maxShiftColumn = shiftColumn;
}


bool TAufitChipMask :: calculateShiftGraundChips( uint8_t number_algorithm ) {
	switch( number_algorithm ) {
	case 1:
		print_log( "Вычисление смещений опорных чипов по морфологическому алгоритму." );
		break;

	case 2:
		print_log( "Вычисление смещений опорных чипов по шаблонному (квантил.2.8) алгоритму." );
		break;
	}

	//TAufitChipMask::ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();
	unsigned long N = numberOfGraundChips();   // Число autoGCPs.

	if( N <= 0 ) {
		print_log( "WARNING: Отсутствуют опорные чипы!" );
		return false;
	}

	int number_gcp = 0;
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
		TDChip* chip = cursor.elementAt();

		if( chip->fChipType[0] == fChipType || chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == fChipType ) {
			number_gcp++;
			switch( number_algorithm ) {
			case 1:
				calculateShiftChip_Morphological_MaxAverage( number_gcp, chip );
				break;
			case 2:
				calculateShiftChip_Templet_Quant2080( chip );
				break;
			}
		}
	}

	return true;
}


void TAufitChipMask :: calculateShiftChip_Templet_Quant2080( TDChip * chip ) {
	fragmX1 = chip->X1 + (long)avg_largeChips_dX;
	fragmY1 = chip->Y1 + (long)avg_largeChips_dY;
	fragmX2 = chip->X2 + (long)avg_largeChips_dX;
	fragmY2 = chip->Y2 + (long)avg_largeChips_dY;
	fragmWidth = fragmX2 - fragmX1 + 1;
	fragmHeight = fragmY2 - fragmY1 + 1;

	buildMaskChip( chip );
	for( int i=0; i<2; i++ ) {
		if( chip->fChannels_forCalcShifts[i] && chip->fChipType[i] == fChipType ) {
			calc_land_sea_Quant2080_Levels( i );
			long shiftX = long(avg_largeChips_dX * maskScale), shiftY = long(avg_largeChips_dY * maskScale);
			minCost00 = 1e50;
			if( calcCostFunction_Templet( i, long(avg_largeChips_dX * maskScale), long(avg_largeChips_dY * maskScale) ) ) {
				calc_shift_Templet_1( i, shiftX, shiftY );
				calc_shift_Templet_2( i, shiftX, shiftY );
				chip->autoDisplacements( i, double(shiftX)/double(maskScale), double(shiftY)/double(maskScale) );
			}
		}
	}
}


void TAufitChipMask :: calculateShiftChip_Morphological_MaxAverage( unsigned number_gcp, TDChip * chip ) {
	fragmX1 = chip->X1 + (long)avg_largeChips_dX;
	fragmY1 = chip->Y1 + (long)avg_largeChips_dY;
	fragmX2 = chip->X2 + (long)avg_largeChips_dX;
	fragmY2 = chip->Y2 + (long)avg_largeChips_dY;
	fragmWidth = fragmX2 - fragmX1 + 1;
	fragmHeight = fragmY2 - fragmY1 + 1;

	buildMaskChip( chip );

	char * maskCloudy = new char[maskWidth * maskHeight];
	memset( maskCloudy, 0, maskWidth * maskHeight );

	TContourImage fCI;
	char mark_sea = 2, mark_land = 3, mark_cloudy_sea = 4, mark_cloudy_land = 5;
	fCI.coastal_lineBinaryImage_withThickness( number_gcp, uint8_t(THICKNESS * maskScale), maskBuf, 0, 1, maskCloudy, maskWidth, maskHeight, mark_sea, mark_land, mark_cloudy_sea, mark_cloudy_land, false );
	delete [] maskCloudy;

	for( int i=0; i<2; i++ ) {
		if( chip->fChannels_forCalcShifts[i] && chip->fChipType[i] == fChipType ) {
			calc_land_sea_QuantMedian_Levels( i );
			bool flagAbs = false;
			long shiftxAbs = 0, shiftyAbs = 0;
			double maxAbsCost = .0;

			long shiftX = long(avg_largeChips_dX * maskScale), shiftY = long(avg_largeChips_dY * maskScale);
			if( avg_largeChips_dX == .0 && avg_largeChips_dY == .0 )
				flagAbs = true;
			else
				flagAbs = false;

			minCost00 = calc_shift_Morphological_1( i, shiftX, shiftY, flagAbs, maxAbsCost, shiftxAbs, shiftyAbs );
			chip->fExtremAlgOptimum[i] = calc_shift_Morphological_2( i, minCost00, shiftX, shiftY, flagAbs, maxAbsCost, shiftxAbs, shiftyAbs );

			chip->autoDisplacements( i, double(shiftX)/double(maskScale), double(shiftY)/double(maskScale) );
			if( flagAbs )
				chip->autoDisplacementsAbs( i, maxAbsCost, double(shiftxAbs)/double(maskScale), double(shiftyAbs)/double(maskScale) );
		}
	}
}


void TAufitChipMask :: buildMaskChip( TDChip * chip ) {
	maskWidth = ( fragmWidth + 2 * maxShiftColumn ) * maskScale;
	maskHeight = ( fragmHeight + 2 * maxShiftLine ) * maskScale;

	if( maskBuf ) {
		delete [] maskBuf;
		maskBuf = 0;
	}
	// Выделение памяти под маску фрагмента.
	maskBuf = new char [maskWidth * maskHeight];

	// собственно создание маски
	char * p = maskBuf;
	double mask_pixel_step = 1. / double( maskScale );
	double y = double( fragmY1 - maxShiftLine );

	if( chip->Ball != 7 ) {
		for( long i = 0; i < maskHeight; i++, y += mask_pixel_step ) {
			double x = double( fragmX1 - maxShiftColumn );
			for( long j = 0; j < maskWidth; j++, x += mask_pixel_step ) {
				double lon, lat;
				fSR->xy2ll( x, y, &lon, &lat );
				long scan, col;
				scan = fPM->scan(lat);
				col = fPM->column(lon);
				if( fDataMask[fMFH->pixNum * scan + col] == 1 )
					*p = 1;
				else
					*p = 0;
				p++;
			}
		}
	} else {
		char mark_non = 6;
		for( long i = 0; i < maskHeight; i++, y += mask_pixel_step ) {
			double x = double( fragmX1 - maxShiftColumn );
			for( long j = 0; j < maskWidth; j++, x += mask_pixel_step ) {
				double lon, lat;
				fSR->xy2ll( x, y, &lon, &lat );
				long scan, col;
				scan = fPM->scan(lat);
				col = fPM->column(lon);
				if( ( col >= fMFH->pixNum ) || ( scan >= fMFH->scanNum ) ) {
					*p = mark_non;
				} else {
					if( fDataMask[fMFH->pixNum * scan + col] == 1 )
						*p = 1;
					else
						*p = 0;
				}
				p++;
			}
		}
	}
}


/*
    Вычисление значений уровней суши и моря по квантилям 0.2 и 0.8 для шаблона суша/море.
*/
void TAufitChipMask :: calc_land_sea_Quant2080_Levels( uint8_t pch ) {
	short min_value, max_value;

	data_find_min_max( &min_value, &max_value, pch,
					   fragmX1 - maxShiftColumn, fragmY1 - maxShiftLine, fragmX2 + maxShiftColumn, fragmY2 + maxShiftLine );

	unsigned long * hist_land = new unsigned long [max_value - min_value + 1];
	unsigned long * hist_sea = new unsigned long [max_value - min_value + 1];

	hist_build_m( hist_land, hist_sea, pch,
				  fragmX1 - maxShiftColumn, fragmY1 - maxShiftLine, fragmX2 + maxShiftColumn, fragmY2 + maxShiftLine,
				  min_value, max_value );

	short vl, vs;

	// оцениваем средние величины пикселов суши и моря
	hist_calc_q( &vl, .5, hist_land, min_value, max_value );
	hist_calc_q( &vs, .5, hist_sea, min_value, max_value );

	// а теперь, в зависимости от того, что ярче - суша или море, берём квантили .2 и .8, или .8 и .2
	if( vl > vs ) {
		hist_calc_q( &vl, .8, hist_land, min_value, max_value );
		hist_calc_q( &vs, .2, hist_sea, min_value, max_value );
	} else {
		hist_calc_q( &vl, .2, hist_land, min_value, max_value );
		hist_calc_q( &vs, .8, hist_sea, min_value, max_value );
	}

	delete [] hist_land;
	delete [] hist_sea;

	landPixValue[pch] = double(vl);
	seaPixValue[pch] = double(vs);
}


void TAufitChipMask :: calc_land_sea_QuantMedian_Levels( uint8_t pch ) {
	short min_value, max_value;

	data_find_min_max( &min_value, &max_value, pch,
					   fragmX1 - maxShiftColumn, fragmY1 - maxShiftLine, fragmX2 + maxShiftLine, fragmY2 + maxShiftColumn );

	unsigned long * hist_land = new unsigned long [max_value - min_value + 1];
	unsigned long * hist_sea = new unsigned long [max_value - min_value + 1];

	hist_build_m( hist_land, hist_sea, pch,
				  fragmX1 - maxShiftColumn, fragmY1 - maxShiftLine, fragmX2 + maxShiftLine, fragmY2 + maxShiftColumn,
				  min_value, max_value );

	short vl, vs;
	hist_calc_q( &vl, .5, hist_land, min_value, max_value );
	hist_calc_q( &vs, .5, hist_sea, min_value, max_value );

	delete [] hist_land;
	delete [] hist_sea;

	landPixValue[pch] = double(vl);
	seaPixValue[pch] = double(vs);
}


void TAufitChipMask :: calc_land_sea_Average_Levels( uint8_t pch ) {
	short min_value, max_value;

	data_find_min_max( &min_value, &max_value, pch,
					   fragmX1 - maxShiftColumn, fragmY1 - maxShiftLine, fragmX2 + maxShiftColumn, fragmY2 + maxShiftLine );

	unsigned long * hist_land = new unsigned long [max_value - min_value + 1];
	unsigned long * hist_sea = new unsigned long [max_value - min_value + 1];

	hist_build_m( hist_land, hist_sea, pch,
				  fragmX1 - maxShiftColumn, fragmY1 - maxShiftLine, fragmX2 + maxShiftColumn, fragmY2 + maxShiftLine,
				  min_value, max_value );

	short vl = 0, vs = 0;
	long n_vl = 0, n_vs = 0;
	for( short i = 0; i < (max_value - min_value + 1); i++ ) {
		n_vs += hist_sea[i];
		vs += hist_sea[i] * (i + min_value);
		n_vl += hist_land[i];
		vl += hist_land[i] * (i + min_value);
	}

	delete [] hist_land;
	delete [] hist_sea;

	landPixValue[pch] = double(vl) / double(n_vl);
	seaPixValue[pch] = double(vs) / double(n_vs);
}


bool TAufitChipMask :: verification_parametrsGraundChips( bool flag ) {
	//TAufitChipMask::ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();
	unsigned long N;

	if( !flag ) {
		N = numberOfGraundChips();   // Число autoGCPs.
		fChipType = TDChip::GraundChip;
	} else {
		N = numberOfControlChips();   // Число autoGCPs.
		fChipType = TDChip::ControlChip;
	}

	if( N <= 0 ) {
		print_log( "WARNING: Отсутствуют опорные чипы!" );
		return false;
	}

	uint8_t number_gcp = 1;
	if( !fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
			if(cursor.elementAt()->fChipType[0] != fChipType ) continue;
			verification_parametrsGraundChip( number_gcp++, 0, cursor.elementAt() );
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
			if( cursor.elementAt()->fChipType[0] == fChipType)
				verification_parametrsGraundChip( number_gcp++, 0, cursor.elementAt() );
			if( cursor.elementAt()->fChannels_forCalcShifts[1] && cursor.elementAt()->fChipType[1] == fChipType )
				verification_parametrsGraundChip( number_gcp++, 1, cursor.elementAt() );
		}
	}

	return true;
}


void TAufitChipMask :: verification_parametrsGraundChip(  unsigned number_gcp, TDChip * chip ) {
	if( !fChannels_forCalcGCPs[1] ) {
		if( chip->fChipType[0] == fChipType )
			verification_parametrsGraundChip( number_gcp, 0, chip );
	} else {
		if( chip->fChipType[0] == fChipType )
			verification_parametrsGraundChip( number_gcp, 0, chip );
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == fChipType )
			verification_parametrsGraundChip( number_gcp, 1, chip );
	}
}


void TAufitChipMask :: verification_parametrsGraundChip( unsigned number_gcp, uint8_t pch, TDChip * chip ) {
	fragmX1 = chip->X1;
	fragmY1 = chip->Y1;
	fragmX2 = chip->X2;
	fragmY2 = chip->Y2;
	fragmWidth = fragmX2 - fragmX1 + 1;
	fragmHeight = fragmY2 - fragmY1 + 1;

	long shift_x = long(chip->dX[pch]), shift_y = long(chip->dY[pch]);
	fragmX1 += shift_x;
	fragmY1 += shift_y;
	fragmX2 += shift_x;
	fragmY2 += shift_y;

	if( maskBuf ) {
		delete [] maskBuf;
		maskBuf = 0;
	}

	// Выделение памяти под маску фрагмента.
	maskWidth = fragmWidth;
	maskHeight = fragmHeight;
	maskBuf = new char [maskWidth * maskHeight];

	// собственно создание маски
	char * p = maskBuf;
	double mask_pixel_step = 1.;
	double y = double( fragmY1 ) - chip->dY[pch];

	char mark_non = 6;
	if( chip->Ball != 7 ) {
		for( long i = 0; i < maskHeight; i++, y += mask_pixel_step ) {
			double x = double( fragmX1 ) - chip->dX[pch];
			for( long j = 0; j < maskWidth; j++, x += mask_pixel_step ) {
				double lon, lat;
				fSR->xy2ll( x, y, &lon, &lat );
				long scan, col;
				scan = fPM->scan(lat);
				col = fPM->column(lon);
				if( fDataMask[fMFH->pixNum * scan + col] == 1 )
					*p = 1;
				else
					*p = 0;
				p++;
			}
		}
	} else {
		for( long i = 0; i < maskHeight; i++, y += mask_pixel_step ) {
			double x = double( fragmX1 ) - chip->dX[pch];
			for( long j = 0; j < maskWidth; j++, x += mask_pixel_step ) {
				double lon, lat;
				fSR->xy2ll( x, y, &lon, &lat );
				long scan, col;
				scan = fPM->scan(lat);
				col = fPM->column(lon);
				if( ( col >= fMFH->pixNum ) || ( scan >= fMFH->scanNum ) ) {
					*p = mark_non;
				} else {
					if( fDataMask[fMFH->pixNum * scan + col] == 1 )
						*p = 1;
					else
						*p = 0;
				}
				p++;
			}
		}
	}

	char * maskCloudy = new char[maskWidth * maskHeight];
	if( !flagMaskCloudClear ) {
		memset( maskCloudy, 0, maskWidth * maskHeight );
	} else {
		for( long i = 0; i < maskHeight; i++ ) {
			char * mask_cp = maskCloudy + i * maskWidth;
			long data_y = fragmY1 + i;
			char * data_cp = fCloudyMask + 2048 * data_y;
			for( long j = 0; j < maskWidth; j++ ) {
				mask_cp[j] = data_cp[fragmX1 + j];
			}
		}
	}

	short min_value, max_value;
	data_find_min_max( &min_value, &max_value, pch, fragmX1, fragmY1, fragmX2, fragmY2 );

	TContourImage fCI;
	char mark_sea = 2, mark_land = 3, mark_cloudy_sea = 4, mark_cloudy_land = 5;
	//    fCI.coastal_lineBinaryImage( maskBuf, 0, 1, maskCloudy, maskWidth, maskHeight, mark_sea, mark_land, mark_cloudy_sea, mark_cloudy_land );
	fCI.coastal_lineBinaryImage_withThickness( number_gcp, (uint8_t)THICKNESS, maskBuf, 0, 1, maskCloudy, maskWidth, maskHeight, mark_sea, mark_land, mark_cloudy_sea, mark_cloudy_land, true );
	delete [] maskCloudy;

	long col_sea = 0, col_land = 0;
	double sr_sea = 0.0, disp_sea = 0.0, sr_land = 0.0, disp_land = 0.0, psi = 0.0, norm_psi = 0.0;

	for( long i = 0; i < maskHeight; i++ ) {
		char * mask_p = maskBuf + i * maskWidth;
		long data_y = fragmY1 + i;
		short * data_p = calibrData[pch] + 2048 * data_y;
		for( long j = 0; j < maskWidth; j++ ) {
			long data_x = fragmX1 + j;
			if( mask_p[j] == mark_sea ) {
				if( data_p[data_x] >= 0 && data_p[data_x] <= maxPixValue[pch] ) {
					sr_sea += double(data_p[data_x]) * ka[pch];
					disp_sea += double(data_p[data_x] * data_p[data_x]) * ka[pch] * ka[pch];
					col_sea++;
				}
			} else {
				if( mask_p[j] == mark_land ) {
					if( data_p[data_x] >= 0 && data_p[data_x] <= maxPixValue[pch] ) {
						sr_land += double(data_p[data_x]) * ka[pch];
						disp_land += double(data_p[data_x] * data_p[data_x]) * ka[pch] * ka[pch];
						col_land++;
					}
				}
			}
		}
	}
	if( col_sea == 0 )
		sr_sea = -1000.;
	else
		sr_sea /= col_sea;
	if( col_land == 0 )
		sr_land = -1000.;
	else
		sr_land /= col_land;

	if( col_sea < 2 )
		disp_sea = -1000.;
	else
		disp_sea = ( disp_sea - col_sea * sr_sea * sr_sea ) / ( col_sea - 1 );
	if( col_land < 2 )
		disp_land = -1000.;
	else
		disp_land = ( disp_land - col_land * sr_land * sr_land ) / ( col_land - 1 );

	if( col_sea != 0 )
		sr_sea += kb[pch];
	if( col_land != 0 )
		sr_land += kb[pch];

	chip->avgLand[pch] = sr_land;
	chip->avgSea[pch] = sr_sea;
	chip->dyspLand[pch] = disp_land;
	chip->dyspSea[pch] = disp_sea;


	TChecking_of_StatisticalHypothesises fCSH;

	if( col_sea >= 2 && col_land >= 2 )
		psi = fCSH.verificationLevel_of_CriterionStjudent( (double)col_sea, sr_sea, disp_sea, (double)col_land, sr_land, disp_land );
	else
		psi = -1000.;
	if( col_sea >= 25 && col_land >= 25 )
		norm_psi = fCSH.verificationNormalLevel_of_CriterionStjudent( (double)col_sea, sr_sea, disp_sea, (double)col_land, sr_land, disp_land );
	else
		norm_psi = -1000.;

	chip->col_sea[pch] = col_sea;
	chip->col_land[pch] = col_land;
	chip->numberDegreeofFreedom[pch] = col_sea + col_land - 2;
	chip->psyCryt[pch] = psi;
	chip->fNormStatSign[pch] = norm_psi;
	if( psi > .0 && col_sea >= 2 && col_land >= 2 )
		chip->Ver[pch] = fCSH.probability_of_DistributionStjudent( psi, (col_sea + col_land - 2) );
	else
		chip->Ver[pch] = -1000.;
}



bool TAufitChipMask :: verification_parametrsGraundChipsAbs( bool flag ) {
	//TAufitChipMask::ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();
	unsigned long N;

	if( !flag ) {
		N = numberOfGraundChips();   // Число autoGCPs.
		fChipType = TDChip::GraundChip;
	} else {
		N = numberOfControlChips();   // Число autoGCPs.
		fChipType = TDChip::ControlChip;
	}

	if( N <= 0 ) {
		print_log( "WARNING: Отсутствуют опорные чипы!" );
		return false;
	}

	uint8_t number_gcp = 1;
	if( !fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			if(cursor.elementAt()->fChipType[0] != fChipType ) continue;
			//if( chipAt(cursor)->fChipType[0] != fChipType ) continue;
			verification_parametrsGraundChipAbs( number_gcp++, 0, cursor.elementAt() );
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			if( cursor.elementAt()->fChipType[0] == fChipType )
				verification_parametrsGraundChipAbs( number_gcp++, 0, cursor.elementAt() );
			if( cursor.elementAt()->fChannels_forCalcShifts[1] && cursor.elementAt()->fChipType[1] == fChipType )
				verification_parametrsGraundChipAbs( number_gcp++, 1, cursor.elementAt() );
		}
	}

	return true;
}



void TAufitChipMask :: verification_parametrsGraundChipAbs( unsigned number_gcp, uint8_t pch, TDChip * chip ) {
	fragmX1 = chip->X1;
	fragmY1 = chip->Y1;
	fragmX2 = chip->X2;
	fragmY2 = chip->Y2;
	fragmWidth = fragmX2 - fragmX1 + 1;
	fragmHeight = fragmY2 - fragmY1 + 1;

	long shift_x = long(chip->dXAbs[pch]), shift_y = long(chip->dYAbs[pch]);
	fragmX1 += shift_x;
	fragmY1 += shift_y;
	fragmX2 += shift_x;
	fragmY2 += shift_y;

	if( maskBuf ) {
		delete [] maskBuf;
		maskBuf = 0;
	}

	// Выделение памяти под маску фрагмента.
	maskWidth = fragmWidth;
	maskHeight = fragmHeight;
	maskBuf = new char [maskWidth * maskHeight];

	// собственно создание маски
	char * p = maskBuf;
	double mask_pixel_step = 1.;
	double y = double( fragmY1 ) - chip->dYAbs[pch];

	char mark_non = 6;
	if( chip->Ball != 7 ) {
		for( long i = 0; i < maskHeight; i++, y += mask_pixel_step ) {
			double x = double( fragmX1 ) - chip->dXAbs[pch];
			for( long j = 0; j < maskWidth; j++, x += mask_pixel_step ) {
				double lon, lat;
				fSR->xy2ll( x, y, &lon, &lat );
				long scan, col;
				scan = fPM->scan(lat);
				col = fPM->column(lon);
				if( fDataMask[fMFH->pixNum * scan + col] == 1 )
					*p = 1;
				else
					*p = 0;
				p++;
			}
		}
	} else {
		for( long i = 0; i < maskHeight; i++, y += mask_pixel_step ) {
			double x = double( fragmX1 ) - chip->dXAbs[pch];
			for( long j = 0; j < maskWidth; j++, x += mask_pixel_step ) {
				double lon, lat;
				fSR->xy2ll( x, y, &lon, &lat );
				long scan, col;
				scan = fPM->scan(lat);
				col = fPM->column(lon);
				if( ( col >= fMFH->pixNum ) || ( scan >= fMFH->scanNum ) ) {
					*p = mark_non;
				} else {
					if( fDataMask[fMFH->pixNum * scan + col] == 1 )
						*p = 1;
					else
						*p = 0;
				}
				p++;
			}
		}
	}

	char * maskCloudy = new char[maskWidth * maskHeight];
	if( !flagMaskCloudClear ) {
		memset( maskCloudy, 0, maskWidth * maskHeight );
	} else {
		for( long i = 0; i < maskHeight; i++ ) {
			char * mask_cp = maskCloudy + i * maskWidth;
			long data_y = fragmY1 + i;
			char * data_cp = fCloudyMask + 2048 * data_y;
			for( long j = 0; j < maskWidth; j++ ) {
				mask_cp[j] = data_cp[fragmX1 + j];
			}
		}
	}

	short min_value, max_value;
	data_find_min_max( &min_value, &max_value, pch, fragmX1, fragmY1, fragmX2, fragmY2 );

	TContourImage fCI;
	char mark_sea = 2, mark_land = 3, mark_cloudy_sea = 4, mark_cloudy_land = 5;
	//    fCI.coastal_lineBinaryImage( maskBuf, 0, 1, maskCloudy, maskWidth, maskHeight, mark_sea, mark_land, mark_cloudy_sea, mark_cloudy_land );
	fCI.coastal_lineBinaryImage_withThickness( number_gcp, (uint8_t)THICKNESS, maskBuf, 0, 1, maskCloudy, maskWidth, maskHeight, mark_sea, mark_land, mark_cloudy_sea, mark_cloudy_land, true );
	delete [] maskCloudy;

	long col_sea = 0, col_land = 0;
	double sr_sea = 0.0, disp_sea = 0.0, sr_land = 0.0, disp_land = 0.0, /*psi = 0.0,*/ norm_psi = 0.0;

	for( long i = 0; i < maskHeight; i++ ) {
		char * mask_p = maskBuf + i * maskWidth;
		long data_y = fragmY1 + i;
		short * data_p = calibrData[pch] + 2048 * data_y;
		for( long j = 0; j < maskWidth; j++ ) {
			long data_x = fragmX1 + j;
			if( mask_p[j] == mark_sea ) {
				if( data_p[data_x] >= 0 && data_p[data_x] <= maxPixValue[pch] ) {
					sr_sea += double(data_p[data_x]) * ka[pch];
					disp_sea += double(data_p[data_x] * data_p[data_x]) * ka[pch] * ka[pch];
					col_sea++;
				}
			} else {
				if( mask_p[j] == mark_land ) {
					if( data_p[data_x] >= 0 && data_p[data_x] <= maxPixValue[pch] ) {
						sr_land += double(data_p[data_x]) * ka[pch];
						disp_land += double(data_p[data_x] * data_p[data_x]) * ka[pch] * ka[pch];
						col_land++;
					}
				}
			}
		}
	}
	if( col_sea == 0 )
		sr_sea = -1000.;
	else
		sr_sea /= col_sea;
	if( col_land == 0 )
		sr_land = -1000.;
	else
		sr_land /= col_land;

	if( col_sea < 2 )
		disp_sea = -1000.;
	else
		disp_sea = ( disp_sea - col_sea * sr_sea * sr_sea ) / ( col_sea - 1 );
	if( col_land < 2 )
		disp_land = -1000.;
	else
		disp_land = ( disp_land - col_land * sr_land * sr_land ) / ( col_land - 1 );

	if( col_sea != 0 )
		sr_sea += kb[pch];
	if( col_land != 0 )
		sr_land += kb[pch];

	TChecking_of_StatisticalHypothesises fCSH;

	if( col_sea >= 25 && col_land >= 25 )
		norm_psi = fCSH.verificationNormalLevel_of_CriterionStjudent( (double)col_sea, sr_sea, disp_sea, (double)col_land, sr_land, disp_land );
	else
		norm_psi = -1000.;

	chip->fAbsNormStatSign[pch] = norm_psi;
}


/*
    Грубое вычисление совмещения маски и изображения.
*/
bool TAufitChipMask :: calc_shift_Templet_1( uint8_t pch, long & shiftx, long & shifty ) {
	long xx = shiftx, yy = shifty;
	long min_shift_x = xx, min_shift_y = yy;
	bool flag;

	for( long shift_y = yy - maxShiftLine * maskScale; shift_y <= yy + maxShiftLine * maskScale; shift_y += maskScale ) {
		for( long shift_x = xx - maxShiftColumn * maskScale; shift_x <= xx + maxShiftColumn * maskScale; shift_x += maskScale ) {
			flag = calcCostFunction_Templet( pch, shift_x, shift_y );
			if( flag ) {
				min_shift_x = shift_x;
				min_shift_y = shift_y;
			}
		}
	}

	shiftx = min_shift_x;
	shifty = min_shift_y;
	return flag;
}


/*
    Уточнение совмещения.
    shift_x, shift_y    Вход: начальное приближение.
                        Выход: результат уточнения.
    c                   Значение штрафной функции, полученное при грубом совмещении.
*/
bool TAufitChipMask :: calc_shift_Templet_2( uint8_t pch, long & shiftx, long & shifty ) {
	long xx = shiftx, yy = shifty;
	long min_shift_x = xx, min_shift_y = yy;
	bool flag;

	for( long shift_y = yy - maskScale; shift_y <= yy + maskScale; shift_y++ ) {
		for( long shift_x = xx - maskScale; shift_x <= xx + maskScale; shift_x++ ) {
			flag = calcCostFunction_Templet( pch, shift_x, shift_y );
			if( flag ) {
				min_shift_x = shift_x;
				min_shift_y = shift_y;
			}
		}
	}

	shiftx = min_shift_x;
	shifty = min_shift_y;
	return flag;
}


/*
    Грубое вычисление совмещения маски и изображения.
*/
double TAufitChipMask :: calc_shift_Morphological_1( uint8_t pch, long & shiftx, long & shifty, bool flagAbs, double & maxAbsCost, long & shiftxAbs, long & shiftyAbs ) {
	long xx = shiftx, yy = shifty;
	long min_shift_x = xx, min_shift_y = yy;
	double cost = .0, maxCost = .0;

	long min_shift_Absx = 0L, min_shift_Absy = 0L;

	double seaV = seaPixValue[pch];
	double landV = landPixValue[pch];

	for( long shift_y = yy - maxShiftLine * maskScale; shift_y <= yy + maxShiftLine * maskScale; shift_y += maskScale ) {
		for( long shift_x = xx - maxShiftColumn * maskScale; shift_x <= xx + maxShiftColumn * maskScale; shift_x += maskScale ) {
			cost = calcCostFunction_Morphological_MaxAverage_onContour( pch, shift_x, shift_y );
			if( maxCost < fabs(cost) && cost/(landV - seaV) > 0 ) {
				min_shift_x = shift_x;
				min_shift_y = shift_y;
				maxCost = fabs(cost);
			}
			if( flagAbs && maxAbsCost < fabs(cost) ) {
				min_shift_Absx = shift_x;
				min_shift_Absy = shift_y;
				maxAbsCost = fabs(cost);
			}
		}
	}

	if( flagAbs ) {
		shiftxAbs = min_shift_Absx;
		shiftyAbs = min_shift_Absy;
	}

	shiftx = min_shift_x;
	shifty = min_shift_y;
	return maxCost;
}


/*
    Уточнение совмещения.
    shift_x, shift_y    Вход: начальное приближение.
                        Выход: результат уточнения.
    c                   Значение штрафной функции, полученное при грубом совмещении.
*/
double TAufitChipMask :: calc_shift_Morphological_2( uint8_t pch, double maxcost, long & shiftx, long & shifty, bool flagAbs, double & maxAbscost, long & shiftxAbs, long & shiftyAbs ) {
	long xx = shiftx, yy = shifty;
	long min_shift_x = xx, min_shift_y = yy;
	double cost = 0.0, maxCost = maxcost;

	for( long shift_y = yy - maskScale; shift_y <= yy + maskScale; shift_y++ ) {
		for( long shift_x = xx - maskScale; shift_x <= xx + maskScale; shift_x++ ) {
			cost = calcCostFunction_Morphological_MaxAverage_onContour( pch, shift_x, shift_y );
			if( maxCost < fabs(cost) ) {
				min_shift_x = shift_x;
				min_shift_y = shift_y;
				maxCost = fabs(cost);
			}
		}
	}

	if( flagAbs ) {
		xx = shiftxAbs, yy = shiftyAbs;
		long min_shift_Absx = xx, min_shift_Absy = yy;
		double maxAbsCost = maxAbscost;
		for( long shift_y = yy - maskScale; shift_y <= yy + maskScale; shift_y++ ) {
			for( long shift_x = xx - maskScale; shift_x <= xx + maskScale; shift_x++ ) {
				cost = calcCostFunction_Morphological_MaxAverage_onContour( pch, shift_x, shift_y );
				if( maxAbsCost < fabs(cost) ) {
					min_shift_Absx = shift_x;
					min_shift_Absy = shift_y;
					maxAbsCost = fabs(cost);
				}
			}
		}
		maxAbscost = maxAbsCost;
		shiftxAbs = min_shift_Absx;
		shiftyAbs = min_shift_Absy;
	}

	shiftx = min_shift_x;
	shifty = min_shift_y;
	return maxCost;
}


/*
    shift_x, shift_y    - задают смещение маски относительно фрагмента в пикселах маски
*/
bool TAufitChipMask :: calcCostFunction_Templet( uint8_t pch, long shift_x, long shift_y ) {
	clearPixels = .0;
	double cost = 0;
	for( long i = 0; i < fragmHeight * maskScale; i++ ) {
		char * mask_p = maskBuf + (maxShiftLine * maskScale + i)*maskWidth + maxShiftColumn * maskScale;
		long data_y = fragmY1 + (i + shift_y)/maskScale;
		short * data_p = calibrData[pch] + 2048 * data_y;
		for( long j = 0; j < fragmWidth * maskScale; j++ ) {
			long data_x = fragmX1 + (j + shift_x)/maskScale;
			if( fCloudyMask[2048 * data_y+data_x] != 1 && data_p[data_x] >= 0 && data_p[data_x] <= maxPixValue[pch] ) {
				double rf;
				if( *mask_p == 1 ) {
					rf = landPixValue[pch];
				} else {
					if( *mask_p == 0 )
						rf = seaPixValue[pch];
				}
				cost += pow( double(data_p[data_x]) - rf, 2. );
				clearPixels += 1.;
				mask_p++;
			}
		}
	}
	cost /= clearPixels;
	if( cost > minCost00 )
		return false;
	minCost00 = cost;
	return true;
}


double TAufitChipMask :: calcCostFunction_Morphological_MaxAverage_onContour( uint8_t pch, long shift_x, long shift_y ) {
	unsigned long col_land = 0, col_sea = 0;
	landPixValue[pch] = 0.0;
	seaPixValue[pch] = 0.0;

	char mark_sea = 2, mark_land = 3/*, mark_cloudy_sea = 4, mark_cloudy_land = 5*/;
	for( long i = 0; i < fragmHeight * maskScale; i++ ) {
		char * mask_p = maskBuf + (maxShiftLine * maskScale + i)*maskWidth + maxShiftColumn * maskScale;
		long data_y = fragmY1 + (i + shift_y)/maskScale;
		short * data_p = calibrData[pch] + 2048 * data_y;
		for( long j = 0; j < fragmWidth * maskScale; j++, mask_p++ ) {
			long data_x = fragmX1 + (j + shift_x)/maskScale;
			if( fCloudyMask[2048 * data_y+data_x] != 1 && data_p[data_x] >= 0 && data_p[data_x] <= maxPixValue[pch] ) {
				if( *mask_p == mark_land ) {
					landPixValue[pch] += double(data_p[data_x]);
					col_land++;
				} else {
					if( *mask_p == mark_sea ) {
						seaPixValue[pch] += double(data_p[data_x]);
						col_sea++;
					}
				}
			}
		}
	}

	return ( (landPixValue[pch] / (double)col_land) - (seaPixValue[pch] / (double)col_sea) );
}


/*
    Построение гистограмм для суши и моря с использованием маски.
    Гистограммирование производится в прямоугольном участке изображения
    (data_x1,data_y1)-(data_x2,data_y2) с разрешением маски.
*/
void TAufitChipMask :: hist_build_m(
	unsigned long * hist_1, // гистограмма для ненулевых пикселов маски
	unsigned long * hist_0, // гистограмма для нулевых пикселов маски

	uint8_t pch,

	// координаты гистограммируемого фрагмента в пикселах
	long data_x1,
	long data_y1,
	long data_x2,
	long data_y2,

	// гистограммируемый диапазон значений пикселов изображения
	short min_hist_value,
	short max_hist_value ) {
	memset( hist_1, 0, (long(max_hist_value) - long(min_hist_value) + 1) * sizeof(long) );
	memset( hist_0, 0, (long(max_hist_value) - long(min_hist_value) + 1) * sizeof(long) );

	const char * base_mask_p = maskBuf;
	for( long i = 0; i < (data_y2 - data_y1 + 1) * maskScale; i++ ) {
		const short * base_data_p = calibrData[pch] + 2048 * (data_y1 + i/maskScale) + data_x1;
		for( long shift_j = 0; shift_j < maskScale; shift_j++ ) {
			const char * mask_p = base_mask_p + shift_j;
			const short * data_p = base_data_p;
			const short * data_p_end = data_p + (data_x2 - data_x1 + 1);
			long data_x = data_x1;
			// в этом цикле двигаемся по пикселам строки изображения
			do {
				unsigned long * hist = *mask_p ? hist_1 : hist_0;
				if( fCloudyMask[2048 * (data_y1 + i/maskScale)+data_x] != 1 && *data_p >= min_hist_value && *data_p <= max_hist_value )
					hist[ *data_p - min_hist_value ]++;
				mask_p += maskScale;
				data_x++;
			} while( ++data_p != data_p_end );
		}
		base_mask_p += maskWidth;
	}
}


/*
    Построение гистограммы.
    Гистограммирование производится в прямоугольном участке изображения
    (data_x1,data_y1)-(data_x2,data_y2) с разрешением изображения.
*/
void TAufitChipMask :: hist_build_m(
	unsigned long * hist, // гистограмма для значений пикселов изображения
	uint8_t pch,
	// координаты гистограммируемого фрагмента в пикселах
	long data_x1,
	long data_y1,
	long data_x2,
	long data_y2,

	// гистограммируемый диапазон значений пикселов изображения
	short min_hist_value,
	short max_hist_value ) {
	memset( hist, 0, (long(max_hist_value) - long(min_hist_value) + 1) * sizeof(long) );

	for( long i = 0; i < (data_y2 - data_y1 + 1); i++ ) {
		const short * base_data_p = calibrData[pch] + 2048 * (data_y1 + i) + data_x1;
		const short * data_p = base_data_p;
		const short * data_p_end = data_p + (data_x2 - data_x1 + 1);
		long data_x = data_x1;
		// в этом цикле двигаемся по пикселам строки изображения
		do {
			if( fCloudyMask[2048 * (data_y1 + i)+data_x] != 1 && *data_p >= min_hist_value && *data_p <= max_hist_value )
				hist[ *data_p - min_hist_value ]++;
			data_x++;
		} while( ++data_p != data_p_end );
	}
}


/*
    Нахождение минимального и максимального значения пикселов в заданном
    фрагменте изображения.

    Параметры:
    min_value, max_value    Результат.
    pch                     Псевдоканал данных AVHRR
    x1, y1, x2, y2          Левый нижний и правый верхний углы фрагмента.

    Возвращаемое значение:
    0   ok
    1   Все пикселы фрагмента не принадлежат рассматриваемому диапазону.
        В переменные min_value, max_value ничего не записывается.
*/
void TAufitChipMask :: data_find_min_max( short * min_value, short * max_value,
		uint8_t pch, long x1, long y1, long x2, long y2 ) {
	short min_acc_value = 0, max_acc_value = maxPixValue[pch];  // допустимый диапазон значений пикселов, всё что вне его - не учитывается

	short min_found = max_acc_value;
	short max_found = min_acc_value;
	const short * base_data_p = calibrData[pch] + 2048 * y1 + x1;

	for( long i = y1; i <= y2; i++ ) {
		const short * data_p = base_data_p;
		const short * data_p_end = data_p + (x2 - x1 + 1);
		long x = x1;
		do {
			if( fCloudyMask[2048 * i + x] != 1 && *data_p >= min_acc_value && *data_p <= max_acc_value ) {
				if( *data_p < min_found )
					min_found = *data_p;
				if( *data_p > max_found )
					max_found = *data_p;
			}
			x++;
		} while( ++data_p != data_p_end );
		base_data_p += 2048;
	}

	if( min_found > max_found )
		return;   // ошибка - все пикселы фрагмента лежат вне диапазона рассматриваемых значений

	*min_value = min_found;
	*max_value = max_found;
}


/*
    Вычисление квантили.
*/
void TAufitChipMask :: hist_calc_q( short * result, double p, unsigned long * hist, short min_hist_value, short max_hist_value ) {
	unsigned long n = 0;
	for( long i = 0; i < max_hist_value - min_hist_value + 1; i++ )
		n += hist[i];
	n = (unsigned long)(double(n) * p);

	short i = 0;
	unsigned long a = 0;
	while( a < n )
		a += hist[i++];
	if( a > n && i > 0 )
		i--;

	*result = i + min_hist_value;
}


bool TAufitChipMask :: check_GraundChipOnBeatenPixels( uint8_t pch, TDChip * chip ) {
	long quan_bad_pix = 0;

	fragmX1 = chip->X1 - maxShiftColumn;
	fragmY1 = chip->Y1 - maxShiftLine;
	fragmX2 = chip->X2 + maxShiftColumn;
	fragmY2 = chip->Y2 + maxShiftLine;

	const short * base_data_p = calibrData[pch] + 2048 * fragmY1 + fragmX1;

	for( long i = fragmY1; i <= fragmY2; i++ ) {
		const short * data_p = base_data_p;
		const short * data_p_end = data_p + (fragmX2 - fragmX1 + 1);
		long x = fragmX1;
		do {
			if( *data_p < 0 || *data_p > maxPixValue[pch] )
				quan_bad_pix++;
			x++;
		} while( ++data_p != data_p_end );
		base_data_p += 2048;
	}

	if( (100. * quan_bad_pix) / ((fragmX2 - fragmX1 + 1)*(fragmY2 - fragmY1 + 1)) >= MAX_PERCENTAGE_BAD_PIXELS_GRAUND_CHIPS )
		return false;   // ошибка - все пикселы фрагмента "плохие"
	else
		return true;
}


//TAufitChipMask :: ChipCursor :: ChipCursor( const TAufitChipMask & acm ) : fChipCursor( *(acm.fChipList) ) {}


unsigned long TAufitChipMask :: numberOfChips() const {
	return ( fChipList->numberOfElements() - numberOfStatChips() );
}


unsigned long TAufitChipMask :: numberOfStatChips() const {
	unsigned n=0;
	TCLCursor cursor = fChipList->createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() )
		if( cursor.elementAt()->Ball == 6 )	n++;
	return n;
}


unsigned long TAufitChipMask :: numberOfGraundChips() const {
	unsigned long n=0;
	TCLCursor cursor = fChipList->createCursor();
	if( fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
			TDChip * chip(cursor.elementAt());
			//             if( chip->Ball == 6 ) continue;
			if( chip->fChipType[0] == TDChip::GraundChip ) n++;
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) n++;
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ){
			//             if( chipAt( cursor )->Ball == 6 ) continue;
			if( cursor.elementAt()->fChipType[0] == TDChip::GraundChip ) n++;
		}
	}

	return n;
}


unsigned long TAufitChipMask :: numberOfReperChips( TDChip::ChipType chiptype = TDChip::GraundChip ) const {
	unsigned long n=0;
	//ChipCursor cursor( *this );

	for( TCLCursor cursor = fChipList->createCursor(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == chiptype ) n++;
		else if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == chiptype ) n++;
	}

	return n;
}


unsigned TAufitChipMask :: duplicateOfGCPs() const {
	unsigned duplicate = 0;
	//ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( (chip->fChipType[0] == TDChip::GraundChip) && (chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip) )
			duplicate++;
	}

	return duplicate;
}


bool TAufitChipMask :: duplicateOfGraundChips() const {
	bool duplicate = false;
	//ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();

	if( fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip )
				duplicate = true;
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
				if( duplicate )
					return true;
				else
					return false;
			}
			duplicate = false;
		}
	}

	return false;
}


bool TAufitChipMask :: duplicateOfGraundChips( int n, TDChip * chips ) const {
	bool duplicate = false;
	long xc = 0, yc = 0;
	for( int i = 0; i < n; i++ ) {
	if( chips[i].fChipType[0] == TDChip::GraundChip ) {
			duplicate = true;
			xc = chips[i].X1 + chips[i].ChipSize().x()/2;
			yc = chips[i].Y1 + chips[i].ChipSize().y()/2;
			continue;
		}
		if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip ) {
			if( duplicate && abs(xc - chips[i].X1 - chips[i].ChipSize().x()/2) < 3
					&& abs(yc - chips[i].Y1 - chips[i].ChipSize().y()/2) < 3 )
				return true;
			else
				return false;
		}
	}

	return false;
}


unsigned long TAufitChipMask :: numberOfGraundChipsStat() const {
	unsigned long n=0;
	//ChipCursor cursor( *this );
	TCLCursor cursor = fChipList->createCursor();
	if( fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->Ball != 6 ) continue;
			if( chip->fChipType[0] == TDChip::GraundChip ) n++;
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) n++;
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->Ball != 6 ) continue;
			if( chip->fChipType[0] == TDChip::GraundChip ) n++;
		}
	}
	return n;
}


unsigned long TAufitChipMask :: numberOfControlChips() const {
	unsigned long n=0;
//	ChipCursor cc( *this );
	TCLCursor cursor = fChipList->createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		//         if( chip->Ball == 6 ) continue;
		if( chip->fChipType[0] == TDChip::ControlChip ) n++;
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip ) n++;
	}
	return n;
}

void TAufitChipMask :: removeAllChips() {
	//TCLCursor cursor = fChipList->createTCLCursor();
	// Чипы уничтожаем вручную, так как стандартная removeAll() этого не делает.
	//for( cursor.setToFirst(); cursor.isValid(); cc.setToNext() ) {
	//	delete chipAt( cc );
	//}

	fChipList->removeAll();
}


void TAufitChipMask :: allGraundChips( TDChip * g_chip ) const {
	TCLCursor cursor = fChipList->createCursor();

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->Ball == 6 )
			continue;
		if( chip->fChipType[0] == TDChip::GraundChip ||
				( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) )
			memcpy( g_chip++, chip, sizeof( TDChip ) );
	}
}

void TAufitChipMask :: setControlToCheckChips() {
	TCLCursor cursor = fChipList->createCursor();

	if( fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::ControlChip )
				chip->fChipType[0] = TDChip::CheckChip;
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
				chip->fChipType[1] = TDChip::CheckChip;
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::ControlChip )
				chip->fChipType[0] = TDChip::CheckChip;
		}
	}
}


void TAufitChipMask :: setGraundToCheckChips() {
	TCLCursor cursor = fChipList->createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip )
			chip->fChipType[0] = TDChip::CheckChip;
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip ) {
			chip->fChannels_forCalcShifts[1] = false;
			chip->fChipType[1] = TDChip::CheckChip;
		}
	}
}


void TAufitChipMask :: setGraundToControlChips() {
	TCLCursor cursor = fChipList->createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::GraundChip )
			chip->fChipType[0] = TDChip::ControlChip;
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
			chip->fChipType[1] = TDChip::ControlChip;
	}
}


void TAufitChipMask :: setControlToGraundChips() {
	TCLCursor cursor = fChipList->createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::ControlChip )
			chip->fChipType[0] = TDChip::GraundChip;
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
			chip->fChipType[1] = TDChip::GraundChip;
	}
}


void TAufitChipMask :: setAllToGraundChips() {
	TCLCursor cursor = fChipList->createCursor();
	if( fChannels_forCalcGCPs[1] ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			//             if( chip->fChipType[0] != TDChip::GraundChip ){
			chip->nullChip();
			chip->fChipType[0] = TDChip::GraundChip;
			//             }
			if( chip->fChannels_forCalcShifts[1] /*&& chip->fChipType[1] != TDChip::GraundChip*/ ) {
				chip->nullChip();
				chip->fChipType[1] = TDChip::GraundChip;
			}
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			//             if( chip->fChipType[0] != TDChip::GraundChip ){
			chip->nullChip();
			chip->fChipType[0] = TDChip::GraundChip;
			//             }
		}
	}
}


void TAufitChipMask :: setAll6ToCheckChips() {
	TCLCursor cursor = fChipList->createCursor();
	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->Ball != 6 )
			continue;
		chip->fChipType[0] = TDChip::CheckChip;
		chip->fChipType[1] = TDChip::CheckChip;
	}
}


void TAufitChipMask :: setCorrectionParams( bool flag, const TCorrectionParams & cop ) {
	fCOP = cop;
	TCLCursor cursor = fChipList->createCursor();

	if( flag ) {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip )
				calculateAutoGraundChip( 0, chip );
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
				calculateAutoGraundChip( 1, chip );
		}
	} else {
		for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
			TDChip * chip = cursor.elementAt();
			if( chip->fChipType[0] == TDChip::GraundChip )
				calculateHandGraundChip( 0, chip );
			if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::GraundChip )
				calculateHandGraundChip( 1, chip );
		}
	}
}


void TAufitChipMask :: setCorrectionParams_forControlChips() {
	TCLCursor cursor = fChipList->createCursor();

	for( cursor.setToFirst(); cursor.isValid(); cursor.setToNext() ) {
		TDChip * chip = cursor.elementAt();
		if( chip->fChipType[0] == TDChip::ControlChip )
			calculateAutoGraundChip( 0, chip );
		if( chip->fChannels_forCalcShifts[1] && chip->fChipType[1] == TDChip::ControlChip )
			calculateAutoGraundChip( 1, chip );
	}
}


void TAufitChipMask :: setCorrectionParams( bool flag, const TCorrectionParams & cop, int n, TDChip * chips ) {
	fCOP = cop;

	for( int i = 0; i < n; i++ ) {
		if( chips[i].fChipType[0] == TDChip::GraundChip ) {
			calculateAutoGraundChip( 0, &(chips[i]) );
		} else {
			if( chips[i].fChannels_forCalcShifts[1] && chips[i].fChipType[1] == TDChip::GraundChip )
				calculateAutoGraundChip( 1, &(chips[i]) );
		}
	}
}



// вычислить текущие координаты GCP по оригинальным или стандартизированным

void TAufitChipMask :: calculateAutoGraundChip( uint8_t pch, TDChip * chip ) {
	double x2, y2, lon, lat;
	TStraightReferencer * sr;
	TInverseReferencer * ir;

	if( fCOP.fVersion == 2 && fCOP.isZero() ) {
		// в этом случае текущая невязка совпадает с вычисленной
		chip->dX[pch] = chip->std_dX[pch];
		chip->dY[pch] = chip->std_dY[pch];
	} else {
		double c_x = double( chip->X1 + chip->ChipSize().x()/2 ), c_y = double( chip->Y1 + chip->ChipSize().y()/2 );

		TCorrectionParams c( 2 );
		sr = new TStraightReferencer( *fISP, *fNIP, c );
		ir = new TInverseReferencer( *fISP, *fNIP, fCOP );

		x2 = c_x - chip->std_dX[pch];
		y2 = c_y - chip->std_dY[pch];

		sr->xy2ll( x2, y2, &lon, &lat );
		ir->ll2xy( lon, lat, &x2, &y2 );

		delete ir;
		delete sr;

		chip->dX[pch] = c_x - x2;
		chip->dY[pch] = c_y - y2;
	}
}


void TAufitChipMask :: calculateHandGraundChip( uint8_t pch, TDChip * chip ) {
	double x2, y2, lon, lat;
	TStraightReferencer * sr;
	TInverseReferencer * ir;

	if( fCOP.fVersion == 2 && fCOP.isZero() ) {
		// в этом случае текущая невязка совпадает с вычисленной
		chip->dhX[pch] = chip->std_dhX[pch];
		chip->dhY[pch] = chip->std_dhY[pch];
	} else {
		double c_x = double( chip->X1 + chip->ChipSize().x()/2 ), c_y = double( chip->Y1 + chip->ChipSize().y()/2 );

		TCorrectionParams c( 2 );
		sr = new TStraightReferencer( *fISP, *fNIP, c );
		ir = new TInverseReferencer( *fISP, *fNIP, fCOP );

		x2 = c_x - chip->std_dhX[pch];
		y2 = c_y - chip->std_dhY[pch];

		sr->xy2ll( x2, y2, &lon, &lat );
		ir->ll2xy( lon, lat, &x2, &y2 );

		delete ir;
		delete sr;

		chip->dhX[pch] = c_x - x2;
		chip->dhY[pch] = c_y - y2;
	}
}

TCLCursor TAufitChipMask :: createCursor(){
	return fChipList->createCursor();
}

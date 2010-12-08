/*-------------------------------------------------------------------------
    tgdbdirect.cpp
-------------------------------------------------------------------------*/


#include "region.cpp"       // файл с данными береговой линии



TGDBDirect::TGDBDirect( long * ranks, long lon, long lat, long lon_size, long lat_size ) :
fCurSegNum(0) {
	if( ranks==0 && lon==0 && lat==0 && lon_size==0 && lat_size==0 ) {   // если параметры не указаны, предоставляем доступ ко всей береговой линии
		fNumberOfSegments = gdb_seg_num;
		fSegments = (TGDBDirectSeg *)gdb_data;
		fCurSeg = fSegments;
		return;
	}


	long max_seg_length = 2048;
	TGDBPoint * pb = 0;

	// создание маски рангов
	long mask_size = 0;
	bool * mask = 0;
	if( ranks ) {
		// определение размера маски
		long *p = ranks;
		while( *p != -1 ) {      // число -1 отмечает конец массива требуемых рангов, например: { 1, 2, -1 }
			if( *p > mask_size )
				mask_size = *p;
			p++;
		}
		mask_size++;
		mask = new bool [mask_size];
		// инициализация маски
		for( long i=0; i<mask_size; i++ )
			mask[i] = false;
		p = ranks;
		while( *p != -1 )
			mask[*p++] = true;
	}

	pb = new TGDBPoint [max_seg_length];
	while( fread( rec_buf, 1, 21, f ) == 21 && strlen( rec_buf ) == 21 ) {  // чтение заголовка сегмента

		TGDBDirectSeg * s = (TGDBDirectSeg *)gdb_data;
		for( long seg_num = 0; seg_num < gdb_seg_num; seg_num++ ) {
			if( !mask || (s->rank<mask_size && mask[s->rank]) ) {    // если сегмент нужного ранга
				if( lon_size && lat_size ) {
					TGDBPoint * first_seg_point = 0;
					TGDBPoint * p = s->points;
					TGDBPoint * p_end = p + s->length;
					// цикл разбиения сегмента из файла на подсегменты и их занесения в список
					while( p != p_end ) {
						if( !first_seg_point ) { // если ищем первую точку подсегмента
							if( pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) == true ) {
								first_seg_point = p;
							}
						} else {   // если ищем последнюю точку подсегмента
							if( pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) == false ) {
								addSegment( first_seg_point, p - first_seg_point, rank );
								first_seg_point = 0;
							}
						}
						p++;
					}
					if( first_seg_point ) {  // если последняя точка сегмента из файла принадлежит географическому району
						addSegment( first_seg_point, p_end - first_seg_point, rank );
					}
				} else {   // если географический район не задан, сегмент из файла полностью заносится в список
					addSegment( pb, n, rank );
				}

			} else {
				// пропуск точек сегмента
				if( fseek( f, 21 * n, SEEK_CUR ) != 0 ) {
					// если пропустить текст точек сегмента в файле не удалось, считаем, что он испорчен
					result = 1;
					goto end;
				}
			}
		}





		// чтение точек сегмента
		for( long i=0; i<n; i++ ) {
			if( fread( rec_buf, 1, 21, f ) != 21 ) {
				// если из файла не удалось прочитать текст очередной точки сегмента, считаем, что он испорчен
				result = 1;
				goto end;
			}
			geoPointFromText( rec_buf, pb + i );
		}

		if( lon_size && lat_size ) { // если задан географический район
			TGDBPoint *first_seg_point = 0;
			TGDBPoint *p = pb;
			TGDBPoint *p_end = pb + n;
			// цикл разбиения сегмента из файла на подсегменты и их занесения в список
			while( p != p_end ) {
				if( !first_seg_point ) { // если ищем первую точку подсегмента
					if( pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) == true ) {
						first_seg_point = p;
					}
				} else {   // если ищем последнюю точку подсегмента
					if( pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) == false ) {
						addSegment( first_seg_point, p - first_seg_point, rank );
						first_seg_point = 0;
					}
				}
				p++;
			}
			if( first_seg_point ) {  // если последняя точка сегмента из файла принадлежит географическому району
				addSegment( first_seg_point, p_end - first_seg_point, rank );
			}
		} else {   // если географический район не задан, сегмент из файла полностью заносится в список
			addSegment( pb, n, rank );
		}

	}
	else {
		// пропуск точек сегмента
		if( fseek( f, 21 * n, SEEK_CUR ) != 0 ) {
			// если пропустить текст точек сегмента в файле не удалось, считаем, что он испорчен
			result = 1;
			goto end;
		}
	}
}

end:
if( pb )
	delete [] pb;
if( mask )
	delete [] mask;
if( f != NULL )
	fclose( f );
if( result != 0 )
	removeAllSegments();
return result;

}


TGDBDirect::~TGDBDirect() {}


unsigned long TGDBDirect::numberOfSegments() {
	return fNumberOfSegments;
}


void TGDBDirect::again() {
	fCurSegNum = 0;
	fCurSeg = fSegments;
}

int TGDBDirect::nextSegment() {
	if( fCurSegNum + 1 == fNumberOfSegments )
		return 1;
	fCurSegNum++;
	fCurSeg = (TGDBDirectSeg *)((char *)fCurSeg + sizeof( TGDBDirectSeg ) + fCurSeg->length * sizeof( TGDBPoint ));
	return 0;
}

unsigned long TGDBDirect::segmentRank() {
	return fCurSeg->rank;
}

unsigned long TGDBDirect::segmentLength() {
	return fCurSeg->length;
}

TGDBPoint * TGDBDirect::getSegment() {
	return fCurSeg->points;
}

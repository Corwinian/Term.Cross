/*-------------------------------------------------------------------------
    tgdb.cpp
-------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tgdb.hpp>

//#include "region.cpp"

extern long gdb_seg_num;

extern long gdb_data[]; //

#pragma pack(4)
struct TGDBDirectSeg {
	long rank;				/* ранг сегмента */
	long length;			/* число точек в сегменте */
	TGDBPoint points[];		/* отсюда начинается собственно массив точек сегмента */
};
#pragma pack()



TGDB::TGDB( const char * file_name,
			int * ranks, long lon,
			long lat, long lon_size,
			long lat_size ) throw ( TAccessExc ) :
fSegNum(0), fMaxSegmentLength(0), fSegList(0), fCurSeg(0), fLastSeg(0){

	//TAccessExc ae1( 1, "TGDB::TGDB: ошибка доступа к файлу базы" );

	FILE * f = fopen( file_name, "rb" );
	if( f == NULL )
		throw TAccessExc ( 1, "TGDB::TGDB: ошибка доступа к файлу базы" );

	int mask_size = 0;
	int * mask = 0;     /* маска рангов сегментов */
	int r[3] = { 1, 2, -1 };
	int * p;

	/* инициализация маски рангов сегментов */
	p  = ranks ? ranks : r;
	while( *p != -1 ) {
		if( *p > mask_size )
			mask_size = *p;
		p++;
	}
	mask_size++;
	mask = new int [mask_size];     // mask_size >= 1
	memset( mask, 0, sizeof(int) * mask_size );
	p  = ranks ? ranks : r;
	while( *p != -1 )
		mask[*p++] = 1;


	int result = 0;

	long max_seg_length = 2048;
	TGDBPoint * pb = 0;

	pb = new TGDBPoint [max_seg_length];
	char rec_buf[22];
	rec_buf[21] = '\0';
	while( fread( rec_buf, 1, 21, f ) == 21 && strlen( rec_buf ) == 21 ) {  /* чтение заголовка сегмента */
		long n = pointNumFromText( rec_buf );
		long rank = rankFromText( rec_buf );
		if( !mask || (rank < mask_size && mask[rank]) ) {    /* если сегмент нужного ранга */

			if( n > max_seg_length ) {    /* если для сегмента нужен буфер большего размера */
				delete [] pb;
				max_seg_length = n;
				pb = new TGDBPoint [max_seg_length];
			}

			/* чтение точек сегмента */
			for( long i=0; i<n; i++ ) {
				/* если из файла не удалось прочитать текст очередной точки сегмента, считаем, что он испорчен*/
				if( fread( rec_buf, 1, 21, f ) != 21 ) {
					result = 1;
					goto end;
				}
				geoPointFromText( rec_buf, pb + i );
			}

			/* если задан географический район */
			if( lon_size && lat_size ) { 
				TGDBPoint * first_seg_point = 0;
				TGDBPoint * p = pb;
				TGDBPoint * p_end = pb + n;
				/* цикл разбиения сегмента из файла на подсегменты и их занесения в список */
				while( p != p_end ) {
					/* если ищем первую точку подсегмента */
					if( !first_seg_point ) { 
						if( pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) ) {
							first_seg_point = p;
						}
					} else {   /* если ищем последнюю точку подсегмента */
						if( !pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) ) {
							unsigned long l = p - first_seg_point;
							TGDBPoint * seg = new TGDBPoint [l];
							memcpy( seg, first_seg_point, sizeof( TGDBPoint ) * l );
							addSegment( seg, l, rank );
							first_seg_point = 0;
						}
					}
					p++;
				}
				if( first_seg_point ) {  /* если последняя точка сегмента из файла принадлежит географическому району */
					unsigned long l = p_end - first_seg_point;
					TGDBPoint * seg =  new TGDBPoint [l];
					memcpy( seg, first_seg_point, sizeof( TGDBPoint ) * l );
					addSegment( seg, l, rank );
				}
			} else {   /* если географический район не задан, сегмент из файла полностью заносится в список */
				TGDBPoint * seg = new TGDBPoint [n];
				memcpy( seg, pb, sizeof( TGDBPoint ) * n );
				addSegment( seg, n, rank );
			}

		} else {
			/* пропуск точек сегмента */
			if( fseek( f, 21 * n, SEEK_CUR ) != 0 ) {
				/* если пропустить текст точек сегмента в файле не удалось, считаем, что он испорчен */
				result = 1;
				goto end;
			}
		}
	}

end:
	fclose( f );
	delete [] pb;
	delete [] mask;

	if( result != 0 ) {
		removeAllSegments();
		throw TAccessExc ( 1, "TGDB::TGDB: ошибка доступа к файлу базы" );
	}

}



TGDB::TGDB( int * ranks, long lon, long lat, long lon_size, long lat_size ) :
fSegNum(0), fMaxSegmentLength(0), fSegList(0), fCurSeg(0), fLastSeg(0){
	int mask_size = 0;
	int * mask = 0;
	int r[3] = { 1, 2, -1 };
	int * p;

	p  = ranks ? ranks : r;
	while( *p != -1 ) {
		if( *p > mask_size )
			mask_size = *p;
		p++;
	}
	mask_size++;
	mask = new int [mask_size];
	memset( mask, 0, sizeof(int) * mask_size );
	p  = ranks ? ranks : r;
	while( *p != -1 )
		mask[*p++] = 1;


	TGDBDirectSeg * gdb_seg = (TGDBDirectSeg *)gdb_data;    /* указатель на текущий сегмент базы  */
	for( long seg_num = 0; seg_num < gdb_seg_num; seg_num++ ) {
		if( !mask || (gdb_seg->rank < mask_size && mask[gdb_seg->rank]) ) {    /* если сегмент нужного ранга */

			if( lon_size && lat_size ) { /* если задан географический район */
				TGDBPoint * first_seg_point = 0;
				TGDBPoint * p = gdb_seg->points;
				TGDBPoint * p_end = p + gdb_seg->length;
				/* цикл разбиения сегмента из базы на подсегменты и их занесения в список */
				while( p != p_end ) {
					if( !first_seg_point ) { /* если ищем первую точку подсегмента */
						if( pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) ) {
							first_seg_point = p;
						}
					} else {   /* если ищем последнюю точку подсегмента */
						if( !pointBelongsRegion( *p, lon, lat, lon_size, lat_size ) ) {
							unsigned long l = p - first_seg_point;
							TGDBPoint * seg = new TGDBPoint [l];
							memcpy( seg, first_seg_point, sizeof( TGDBPoint ) * l );
							addSegment( seg, l, gdb_seg->rank );
							first_seg_point = 0;
						}
					}
					p++;
				}
				if( first_seg_point ) {  /* если последняя точка сегмента из файла принадлежит географическому району */
					unsigned long l = p_end - first_seg_point;
					TGDBPoint * seg =  new TGDBPoint [l];
					memcpy( seg, first_seg_point, sizeof( TGDBPoint ) * l );
					addSegment( seg, l, gdb_seg->rank );
				}
			} else {   /* если географический район не задан, сегмент из файла полностью заносится в список */
				TGDBPoint * seg = new TGDBPoint [gdb_seg->length];
				memcpy( seg, gdb_seg->points, sizeof( TGDBPoint ) * gdb_seg->length );
				addSegment( seg, gdb_seg->length, gdb_seg->rank );
			}
		}
		gdb_seg = (TGDBDirectSeg *)(gdb_seg->points + gdb_seg->length); /* перемещаемся на следующий сегмент базы */
	}


	delete [] mask;
}



int TGDB::nextSegment() {
	if( !fCurSeg ) {  /* если были вызваны непосредственно после создания объекта или вызова again() */
		if( !(fCurSeg = fSegList) )
			return 1; /* список сегментов пуст */
	} else {
		if( !fCurSeg->next )
			return 1;   /* достигли конца списка сегментов */
		fCurSeg = fCurSeg->next;
	}
	return 0;
}


TGDBPoint * TGDB::segmentPoints() {
	if( fCurSeg )
		return fCurSeg->points;
	else
		return 0;
}


long TGDB::segmentLength() {
	if( fCurSeg )
		return fCurSeg->length;
	else
		return 0;
}


long TGDB::segmentRank() {
	if( fCurSeg )
		return fCurSeg->rank;
	else
		return 0;
}


void TGDB::addSegment( TGDBPoint * points, long point_num, long rank ) {
	/* коррекция maxPointNum */
	if( fMaxSegmentLength < point_num )
		fMaxSegmentLength = point_num;
	/* вставляем сегмент в конец списка сегментов */
	TGDBSegList * seg = new TGDBSegList( rank, point_num, points, 0 );
	if( fSegList ) {     /* если список непуст */
		fLastSeg->next = seg;
		fLastSeg = seg;
	} else {
		fSegList = fLastSeg = seg;
	}
	fSegNum++;
}


void TGDB::removeAllSegments() {
	TGDBSegList * p;
	/* цикл уничтожения списка начиная с головы */
	while( (p = fSegList) != 0 ) {
		fSegList = p->next;
		delete [] p->points;
		delete p;
	}

	fSegList = fLastSeg = fCurSeg = 0;
	fSegNum = fMaxSegmentLength = 0;
}


int TGDB::pointBelongsRegion( const TGDBPoint & point, long lon, long lat, long lon_size, long lat_size ) {
	long lon2 = lon + lon_size;
	long lat2 = lat + lat_size;

	if( point.lon < lon )
		return 0;
	if( lon2 > 180*3600 ) {  /* если географический прямоугольник накрывает 180-меридиан */
		if( point.lon + 360*3600 >= lon2 )
			return 0;
	} else {
		if( point.lon >= lon2 )
			return 0;
	}

	if( point.lat < lat )
		return 0;
	if( point.lat >= lat2 )
		return 0;

	return 1;
}



long TGDB::pointNumFromText( const char *hdr_rec_text ) {
	char t[7];
	memcpy( t, hdr_rec_text + 9, 6 );
	t[6] = '\0';
	return atol( t );
}


long TGDB::rankFromText( const char *hdr_rec_text ) {
	char t[3];
	memcpy( t, hdr_rec_text + 7, 2 );
	t[2] = '\0';
	return atol( t );
}


void TGDB::geoPointFromText( const char *point_rec_text, TGDBPoint * point ) {
	const char *q = point_rec_text;
	char t[4];
	char *p;

	point->lon = point->lat = 0;

	/* получаем градусы широты */
	p = t;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = '\0';
	point->lat += atol(t) * 3600;
	/* получаем минуты широты */
	p = t;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = '\0';
	point->lat += atol(t) * 60;
	/* получаем секунды широты */
	p = t;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = '\0';
	point->lat += atol(t);

	if( *q++ == 'S' )
		point->lat *= -1;

	/* получаем градусы долготы */
	p = t;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = '\0';
	point->lon += atol(t) * 3600;
	/* получаем минуты долготы */
	p = t;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = '\0';
	point->lon += atol(t) * 60;
	/* получаем секунды долготы */
	p = t;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = '\0';
	point->lon += atol(t);

	if( *q++ == 'W' )
		point->lon *= -1;
}

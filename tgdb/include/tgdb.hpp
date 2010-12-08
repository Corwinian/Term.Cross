/*-------------------------------------------------------------------------
   tgdb.hpp
-------------------------------------------------------------------------*/
#ifndef _TGDB_HPP_
#define _TGDB_HPP_

#include <tc_config.h>
#include <c_lib.hpp>

#pragma pack(1)
struct TGDBPoint {
	long lon;
	long lat;
};
#pragma pack()


struct TGDBSegList {
	long rank;
	long length;
	TGDBPoint * points;
	TGDBSegList * next;

	TGDBSegList( long r, long l, TGDBPoint * p, TGDBSegList * n ) : rank(r), length(l), points(p), next(n) {}
}
;


/*-------------------------------------------------------------------------
    class TGDB
-------------------------------------------------------------------------*/
class TGDB {
public:
	/*
	 *  Конструктор, считывающий данные о географических объектах из текстового
	 *  файла формата CIA_DB.
	 *  Параметры:
	 *  ranks   Указатель на массив рангов сегментов, которые должны быть
	 *          из файла базы. Последним элементом массива должно быть число
	 *          -1, служащее признаком его конца. Если этот параметр равен 0,
	 *          из файла будут загружены сегменты только рангов 1 и 2, попавшие в
	 *          заданный параметрами lon,lat,lon_size,lat_size географический
	 *          район.
	 *  lon,lat,lon_size,lat_size    Задают географический район. Единицы
	 *          измерения - угловые секунды. Отсчёт ведётся от Гринвича для
	 *          долготы и от экватора для широты. Если хотя бы один из параметров
	 *          lon_size или lat_size равен 0, отсечения по границам района
	 *          при считывании базы производиться не будет.
	 *  Исключения:
	 *  TAccessExc ae1( 1, "TGDB::TGDB: ошибка доступа к файлу базы" );
	 */
	TGDB( const char * file_name, int * ranks = 0, long lon = 0, long lat = 0, long lon_size = 0, long lat_size = 0 ) throw ( TAccessExc );

	/* Конструктор, использующий данные из самой dll. */
	TGDB( int * ranks = 0, long lon = 0, long lat = 0, long lon_size = 0, long lat_size = 0 );

	~TGDB() {
		removeAllSegments();
	}

	/* Число сегментов, содержащихся в объекте. */
	long numberOfSegments() {
		return fSegNum;
	}

	/* Максимальная длина сегмента в базе. */
	long maxSegmentLength() {
		return fMaxSegmentLength;
	}

	/*
	 * Функция должна вызываться, если возникла необходимость повторного доступа
	 * к сегментам базы. Сразу после создания объекта эту функцию вызывать не нужно.
	 */
	void again() {
		fCurSeg = 0;
	}

	/*
	 * Функция должна вызываться перед началом доступа к данным следующего сегмента базы.
	 * Должна вызываться, когда были прочитаны все точки текущего сегмента, а также
	 * сразу после создания объекта или вызова функции again() для начала работы с
	 * первым сегментом.
	 * Возвращаемое значение:
	 * 0    Успешно.
	 * 1    Были прочитаны все сегменты базы.
	 */
	int nextSegment();

	/* Получить доступ к точкам сегмента. */
	TGDBPoint * segmentPoints();

	/* Возвращает количество точек в текущем сегменте базы. */
	long segmentLength();
	/* Возвращает ранг текущего сегмента базы. */
	long segmentRank();


protected:
	static long pointNumFromText( const char * hdr_rec_text );
	static long rankFromText( const char * hdr_rec_text );
	static void geoPointFromText( const char * point_rec_text, TGDBPoint * point );
	static int pointBelongsRegion( const TGDBPoint & point, long lon, long lat, long lon_size, long lat_size );

	/* Создаёт элемент списка сегментов. */
	void addSegment( TGDBPoint * points, long point_num, long rank );
	/* Полностью уничтожить список сегментов. */
	void removeAllSegments();

private:
	long fSegNum;               /* число сегментов в списке */
	long fMaxSegmentLength;     /* самый длинный сегмент */
	TGDBSegList * fSegList;     /* список сегментов */
	TGDBSegList * fCurSeg;      /* текущий сегмент */
	TGDBSegList * fLastSeg;     /* указатель на последний сегмент списка */
};

#endif

/*-------------------------------------------------------------------------
    makecpp.cpp
-------------------------------------------------------------------------*/
#include <stdio.h>

#include <tgdb.hpp>

char gdbFileName[] = "region.cil";
char cppFileName[] = "region.cpp";

char cppHdr[] = "\
/*-------------------------------------------------------------------------\n\
 region.cpp\n\
-------------------------------------------------------------------------*/\n";


int main() {
	TGDB gdb( gdbFileName );
	FILE * f = fopen( cppFileName, "w" );
	fprintf( f, "%s\n", cppHdr );
	fprintf( f, "long gdb_seg_num = %d;\n\n", gdb.numberOfSegments() );

	unsigned long data_size = 0;

	while( gdb.nextSegment() == 0 ) {
		data_size += 2;     // поля rank и length
		data_size += gdb.segmentLength() * 2;   // координаты точек
	}

	fprintf( f, "long gdb_data[%d] = {\n", data_size );

	gdb.again();
	for( long i=0; i<gdb.numberOfSegments(); i++ ) {
		gdb.nextSegment();
		fprintf( f, "%d, %d,\n", gdb.segmentRank(), gdb.segmentLength() );  // заголовок сегмента
		TGDBPoint * p = gdb.segmentPoints();
		for( long j=0; j<gdb.segmentLength()-1; j++ )
			fprintf( f, "%d,%d, ", p[j].lon, p[j].lat );
		fprintf( f, "%d,%d", p[gdb.segmentLength()-1].lon, p[gdb.segmentLength()-1].lat );  // последняя точка сегмента
		if( i != gdb.numberOfSegments()-1 )
			fprintf( f, ",\n" );
	}

	fprintf( f, "\n};\n" );

	fclose( f );
}

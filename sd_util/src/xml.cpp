#include <tc_config.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <c_lib.hpp>

#include <xml.hpp>

static char msg[2048];

XML::XML( char* file_name ) throw (TAccessExc) {
	number_tags = 0;
	line = 0;
	strcpy( this->file_name, file_name );

	file_header = fopen( file_name, "r" );
	if( file_header == 0 ) {
		sprintf( msg, "XML::XML  Не могу открыть файл %s", file_name );
		throw TAccessExc( 1, msg );
	}
	fseek( file_header, 0, SEEK_END );
	int file_length = ftell( file_header );
	fseek( file_header, 0, SEEK_SET );

	buffer = (char*)malloc( file_length+1 );
	if( buffer == NULL) {
		throw TAccessExc( 2, "XML::XML  Не могу выделить память под буфер" );
	}
	fread( buffer, file_length, 1, file_header );
	fclose( file_header );
	current_char = -1;
	buffer_length=file_length;
	buffer[buffer_length]=0;
}

XML::XML( char* buffer_old, int length ) throw (TRequestExc) {
	number_tags = 0;
	buffer_length = length + 1;

	//   printf( "XML:XML выделяем %d байтов памяти\n", buffer_length );
	buffer = (char*)malloc( buffer_length );
	//   printf( "XML:XML выделили %p\n", buffer );

	if( buffer == NULL ) {
		throw TRequestExc( 3, "XML::XML  Не могу выделить память под буфер" );
	}

	//   printf( "Копируем %d из %p в %p\n", length, buffer_old, buffer );
	memcpy( buffer, buffer_old, length );
	//   printf( "Скопировали\n" );
	buffer[length] = 0;
	current_char = -1;
}

char XML::get_ahead_char() {
	if( current_char + 1 < buffer_length )
		return buffer[current_char+1];
	return 0;
}

char XML::get_next_char() {
	if( current_char + 1 < buffer_length ) {
		current_char++;
		return buffer[current_char];
	}
	return 0;
}

int XML::get_lexem() throw (TRequestExc) {
	char a;
	tag.clear();
	text.clear();
	a = get_ahead_char();
	if( a == 0 ) {
		return 0;
	} else if( a == '<' ) {        // Встречен тег
		bool basic_tag = true;  // мы считываем основную часть тега
		a = get_next_char();
		while( 1 ) {
			a = get_next_char();
			if( a == '>' )
				return 1;
			else if( a == '<' ) {
				sprintf( msg, "XML::get_lexem  Символ '<' в имени тега. Файл %s. Строка %d.", file_name, line  );
				throw TRequestExc( 1, msg );
			} else if( a == 0 ) {
				sprintf( msg, "XML::get_lexem  Не закрыт тег. Файл %s. Строка %d.", file_name, line );
				throw TRequestExc( 2, msg );
			} else {
				text += a;
				if( basic_tag ) {
					if( a == ' ' )
						basic_tag = false;
					else
						tag += a;
				}
				if( a == 10 )
					line++;
			}
		}
	} else {               /* Встречен текст */
		while( 1 ) {
			if( a == '>' ) {
				sprintf( msg, "XML::get_lexem  Символ '>' в тексте файла. Файл %s. Строка %d.", file_name, line );
				throw TRequestExc( 3, msg );
			} else if( a == '<' ) {
				return 2;
			} else if( a == 0 ) {
				return 2;
			} else {
				text += (a = get_next_char());
				if( a == 10 )
					line++;
			}
			a = get_ahead_char();
		}
	}
}

XML::~XML() {
	free(buffer);
}

TCharString& XML::get_text( int n, char** mask )  throw (TRequestExc) {
	int l;
	result.clear();
	int count = 0;

	while( (l = get_lexem()) != 0 ) {
		count++;
		//        printf( "count = %d\n", count );
		if( l == 1 ) {
			//            printf( "count = %d text[1] = %d %c\n", count, int(text[1]), char(text[1]) );
			if( text[0] == '/' ) {
				if( number_tags == 0 ) {
					sprintf( msg, "XML::get_text  Встречен закрывающий тег <%s> для которого нет открывающего. Файл %s. Строка %d.", (const char*)text, file_name, line );
					throw TRequestExc( 1, msg );
				}

				const char * a = tag;
				a++;
				const char * b = btags[number_tags-1];
				if( strcmp(a,b) != 0 ) {
					sprintf( msg, "XML::get_text  Закрыт неверный тег. Файл %s. Строка %d.", file_name, line );
					throw TRequestExc( 2, msg );
				};
				number_tags--;
			} else {
				btags[number_tags] = tag;
				tags[number_tags] = text;
				number_tags++;
			}
		} else if( l == 2 ) {
			if( number_tags == n ) {
				bool flag = true;
				int i;
				for( i = 0; i < number_tags; i++ ) {
					if( strcmp( (const char*)(mask[i]), (const char*)(tags[i]) ) != 0 )
						flag = false;
				}
				if( flag == true ) {
					result += text;
				}
			}
		}
	}
	if( number_tags != 0 ) {
		sprintf( msg, "XML::get_text  При выходе из разбора остались незакрытые теги. Файл %s. Строка %d.", file_name, line );
		throw TRequestExc( 3, msg );
	}
	return result;
}

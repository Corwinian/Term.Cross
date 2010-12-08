/*-----------------------------------------------------------------------------
    c_cfg.cpp
-----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tc_config.h>
#include "c_lib/c_cfg.hpp"


// Расположение элементов строки в массиве (относится к классу ConfString)
static const int posOneSpaces = 0;
static const int posKey = 1;
static const int posTwoSpaces = 2;
static const int posValue = 3;
static const int posThreeSpaces = 4;
static const int posRemChars = 5;
static const int posRemOneSpaces = 6;
static const int posRemValue = 7;
static const int posRemTwoSpaces = 8;
static const int posEndOfLine = 9;


// Число состояний у автомата
static const int NStates = 14;
// Количество классов символов
static const int NClassesOfChars = 6;
// Классы символов
// 0 --- конец файла
// 1 --- пробел
// 2 --- слеш (экранирование)
// 3 --- решетка '#' --- начало комментария
// 4 --- конец строки
// 5 --- обычный символ (то, что не относится к вышеопределенному)
static int automat[NStates][NClassesOfChars] = {
			//       0     1     2     3     4     5
			{ 1201,   10,  210,  710, 1110,  110 },  // 0  начало строки (пробелы)
			{ 1201,  310,  200,  710, 1110,  110 },  // 1  ключ (обычные символы)
			{ 1201,  110,  110,  110, 1110,  110 },  // 2  экранирование (ключ)
			{ 1201,  310,  500,  710, 1110,  410 },  // 3  разделитель между ключем и значением параметра
			{ 1201,  610,  500,  710, 1110,  410 },  // 4  значение параметра
			{ 1201,  410,  410,  410, 1110,  410 },  // 5  экранирование (значение параметра)
			{ 1201,  610, 1310,  710, 1110, 1310 },  // 6  разделитель между ключем и комментарием
			{ 1201,  810,  910,  910, 1110,  910 },  // 7  начало комментария
			{ 1201,  810,  910,  910, 1110,  910 },  // 8  пробелы перед текстом комментария
			{ 1201,  910,  910,  910, 1110,  910 },  // 9  текст комментария
			{ 1201,    0,    0,    0,    0,    0 },  // 10 пробелы после текста комментария
			{ 1201,   11,  201,  711, 1110,  111 },  // 11 конец строки
			{    0,    0,    0,    0,    0,    0 },  // 12 конец файла
			{    0,    0,    0,    0,    0,    0 }   // 13 ошибка
		};
//  Описание элемента автомата
//  +---< состояние в которое следует перейти (2 цифры)
//  |
//  | +---< следует-ли добавлять текущий символ к буферу
//  | |
//  | | +---< следует-ли перейти к следующей строке
//  v v v
// 32 1 0

// Расположение в строке элементов, соответствующих различным состояниям
static int posElementsInString[NStates] = {
			posOneSpaces,    // 0  начало строки (пробелы)
			posKey,          // 1  ключ (обычные символы)
			posKey,          // 2  экранирование (ключ)
			posTwoSpaces,    // 3  разделитель между ключем и значением параметра
			posValue,        // 4  значение параметра
			posValue,        // 5  экранирование (значение параметра)
			posThreeSpaces,  // 6  разделитель между ключем и комментарием
			posRemChars,     // 7  начало комментария
			posRemOneSpaces, // 8  пробелы перед текстом комментария
			posRemValue,     // 9  текст комментария
			posRemTwoSpaces, // 10 пробелы после текста комментария
			posEndOfLine,    // 11 конец строки
			posEndOfLine,    // 12 конец файла (чтобы не писать 0)
			posEndOfLine     // 13 ошибка (чтобы не писать 0)
		};

TCfg::TCfg() {
	fileName[0] = 0;
	confString = new TSimpleList<ConfString>();
	cursor = confString->createCursorPtr();
}

static int getTChar( void* );

TCfg::TCfg( const char * file_name ) throw( TAccessExc, TRequestExc ) {
	confString = new TSimpleList<ConfString>();
	cursor = confString->createCursorPtr();

	strcpy( fileName, file_name );

	FILE * f = fopen( fileName, "rb" );
	if( f == NULL ) {
		char msg[200+MAX_PATH];
		sprintf( msg, "TCfg::TCfg: ошибка открытия файла конфигурации %s", fileName );
		throw TAccessExc( 1, msg );
	}

	try {
		parse( getTChar, f );
	} catch( TRequestExc ) {   // синтаксическая ошибка в файле
		throw;
	}

	fclose( f );

	confToParams();
	cursor->invalidate();        // Здесь тоже необходимо установить курсор
}


TCfg::~TCfg() {
	delete cursor;
	delete confString;
}


const char * TCfg::cfgName() const {
	static char fname[ MAX_FNAME ];
	splitpath( (char*)fileName, 0, 0, fname, 0 );
	return fname;
}


void TCfg::confToParams() {
	for( cursor->setToFirst(); cursor->isValid(); cursor->setToNext() ) {
		ConfString & b = *(cursor->elementAt());
		if( b[posKey].length() != 0 )
			b.setIsKey();
	}
}

static void protect( const TCharString &, TCharString & );
static void strwrite(  FILE*, const TCharString& );

void TCfg::saveCfg( const char * file_name ) throw (TParamExc, TAccessExc) {
	if( file_name ) {
		strcpy( fileName, file_name );
	} else {
		if( fileName[0] == '\0' ) {
			throw TParamExc( 1, "TCfg::saveCfg: не задано имя записываемого файла" );
		}
	}

	FILE * f = fopen( fileName, "wb" );
	if( f == NULL ) {
		throw TAccessExc( 1, "TCfg::saveCfg: ошибка открытия файла" );
	}

	TCharString sKey;     // Предоставляем экранированные версии двух полей
	TCharString sValue;
	for( cursor->setToFirst(); cursor->isValid(); cursor->setToNext()) {
		ConfString &c = *(cursor->elementAt());
		protect( c[posKey], sKey );
		protect( c[posValue], sValue );

		strwrite( f, c[posOneSpaces] );
		strwrite( f, sKey );
		strwrite( f, c[posTwoSpaces] );
		strwrite( f, sValue );
		strwrite( f, c[posThreeSpaces] );
		strwrite( f, c[posRemChars] );
		strwrite( f, c[posRemOneSpaces] );
		strwrite( f, c[posRemValue] );
		strwrite( f, c[posRemTwoSpaces] );
		strwrite( f, c[posEndOfLine] );
	}

	fclose( f );
}

static int cmpKey( TCharString& c, const char * pd ) {
#if defined STRICMP
	return stricmp( (const char *)c, pd );
#elif defined STRCASECMP

	return strcasecmp( (const char *)c, pd );
#else
#error "see tc_config.h"
#endif
}


int TCfg::containsParamWithKey( const char * keyName ) throw (TParamExc) {
	if( keyName == 0 || keyName[0] == 0 ) {
		throw TParamExc(1, "TCfg::containsParamWithKey: не задано имя параметра" );
	}

	for( cursor->setToFirst(); cursor->isValid(); cursor->setToNext()) {
		ConfString &c = *(cursor->elementAt());
		if( cmpKey(c[posKey], keyName) == 0 ) {
			return 1;
		}
	}

	return 0;
}


const char * TCfg::getValue( const char * key ) throw (TParamExc, TRequestExc) {
	if( key == 0 || key[0] == 0 ) {
		throw TParamExc(1, "TCfg::getValue: не задано имя параметра" );
	}

	for( cursor->setToFirst(); cursor->isValid(); cursor->setToNext()) {
		ConfString &c = *(cursor->elementAt());
		if( cmpKey( c[posKey], key ) == 0 ) {
			return c[posValue];
		}
	}

	char msg[200];
	sprintf( msg, "TCfg::getValue: отсутствует параметр %s", strlen( key ) < 150 ? key : "с указанным именем" );
	throw TRequestExc( 1, msg );
}


void TCfg::addOrReplaceParam( const char* keyName, const char* value, const char* comment ) throw (TParamExc) {
	if( keyName == 0 || keyName[0] == 0 ) {
		throw TParamExc(1, "TCfg::addOrReplaceParam: не задано имя параметра" );
	}

	for( cursor->setToFirst(); cursor->isValid(); cursor->setToNext()) {
		ConfString &c = *(cursor->elementAt());
		if( cmpKey( c[posKey], keyName ) == 0 ) {
			if( value != 0 ) {
				if( c[posTwoSpaces].length() == 0 ) {
					c[posTwoSpaces] = TCharString( ' ' );
				}
				c[posValue] = value;
			}    // в противном случае значение параметра не изменяется
			if( comment != 0 ) {
				if( c[posRemChars].length() == 0 ) {
					c[posThreeSpaces]  = TCharString( ' ' );
					c[posRemChars]     = TCharString( '#' );
					c[posRemOneSpaces] = TCharString( ' ' );
				}
				c[posRemValue] = comment;
			}
			cursor->invalidate();
			return;
		}
	}

	// Строки с заданным ключом нет - добавляем новую
	if( !confString->isEmpty() ) {
		cursor->setToLast();
		ConfString &c = *(cursor->elementAt());
		if( c[posEndOfLine].length() == 0 ) {    // В случае необходимости добавляем перевод каретки к последней строке
			c[posEndOfLine] += '\n';
		}
	}

	// Генерируем строку, которую будем вставлять
	ConfString * pCfg = new ConfString;
	ConfString & cfg = *pCfg;
	cfg[posKey] = TCharString( keyName );
	cfg[posTwoSpaces] = TCharString( ' ' );
	if( value != 0 ) {
		cfg[posValue] = TCharString( value );
	}    // в противном случае значение параметра остается пустым
	if( comment != 0 ) {
		cfg[posThreeSpaces] = TCharString( ' ' );
		cfg[posRemChars] = TCharString( '#' );
		cfg[posRemOneSpaces] = TCharString( ' ' );
		cfg[posRemValue] = TCharString( comment );
		cfg[posEndOfLine] = TCharString( "\n" );
	}
	cfg.setIsKey();

	confString->addAsLast( pCfg );
}


void TCfg::setToFirst() {
	cursor->setToFirst();
}


void TCfg::setToNext() throw (TRequestExc) {
	if( !cursor->isValid() ) {
		throw TRequestExc( 1, "TCfg::setToNext: курсор недействителен" );
	}

	ConfString * c;
	do {
		cursor->setToNext();
		if( !cursor->isValid() )
			return;
		c = cursor->elementAt();
	} while( !(c->isKey()) );
}


int TCfg::isValid() {
	return cursor->isValid();
}


void TCfg::setTo( const char * key ) throw (TParamExc, TRequestExc) {
	if( key == 0 || key[0] == 0 ) {
		throw TParamExc( 1, "TCfg::setTo: не задано имя параметра" );
	}

	for( cursor->setToFirst(); cursor->isValid(); cursor->setToNext()) {
		ConfString &c = *(cursor->elementAt());
		if( cmpKey( c[posKey], key ) == 0 ) {
			return;     // выходим, курсор установлен нормально
		}
	}

	// сейчас курсор недействителен
	throw TRequestExc( 1, "TCfg::setTo: отсутствует параметр с указанным именем" );
}


const char * TCfg::getCurrentKey() throw( TRequestExc ) {
	if( !isValid() ) {
		throw TRequestExc( 1, "TCfg::getCurrentKey: курсор недействителен" );
	}

	return (*cursor->elementAt())[posKey];
}


const char * TCfg::getCurrentValue() throw( TRequestExc ) {
	if( !isValid() ) {
		throw TRequestExc( 1, "TCfg::getCurrentValue: курсор недействителен" );
	}

	return (*cursor->elementAt())[posValue];
}


const char * TCfg::getCurrentComment() throw( TRequestExc ) {
	if( !isValid() ) {
		throw TRequestExc( 1, "TCfg::getCurrentComment: курсор недействителен" );
	}

	return (*cursor->elementAt())[posRemValue];
}


void TCfg::setCurrentKey( const char * keyName ) throw( TParamExc, TRequestExc ) {
	if( !isValid() ) {
		throw TRequestExc( 1, "TCfg::setCurrentKey: курсор недействителен" );
	}

	if( keyName == 0 || keyName[0] == 0 ) {
		throw TParamExc( 1, "TCfg::setCurrentKey: не задано имя параметра" );
	}

	(*cursor->elementAt())[posKey] = keyName;
}


void TCfg::setCurrentValue( const char * value ) throw( TRequestExc ) {
	if( !isValid() ) {
		throw TRequestExc( 1, "TCfg::setCurrentValue: курсор недействителен" );
	}

	(*cursor->elementAt())[posValue] = value;
}


void TCfg::setCurrentComment( const char * comment ) throw( TRequestExc ) {
	if( !isValid() ) {
		throw TRequestExc( 1, "TCfg::setCurrentComment: курсор недействителен" );
	}

	ConfString & c = *(cursor->elementAt());

	if( c[posRemChars].length() != 1 ) {
		c[posThreeSpaces] = TCharString( ' ' );
		c[posRemChars] = TCharString( '#' );
		c[posRemOneSpaces] = TCharString( ' ' );
	}

	c[posRemValue] = TCharString( comment );
	if( c[posEndOfLine].length() == 0 ) {
		c[posEndOfLine] += '\n';
	}

}

static int classOfSymbol( int );

// Разбор массива исходных данных
void TCfg::parse( int (*getChar)(void*), void* charSource ) throw (TRequestExc) {
	int line = 1;       // счетчик строк
	//int nl_flag = 1;   // флаг, говорящий о том, что при появлении следующего символа нужно перевести строку

	// Считаем, что компилятор соответвствет третьему стандарту языка,
	// а инициализация нулями нас устраивает
	confString->addAsLast( new ConfString );
	cursor->setToLast();

	int S = 0;    // Текущее состояние автомата
	while( S != 12 && S != 13 ) {    // Пока не встретится ошибка или конец файла
		int c = getChar( charSource );
		if( c == '\n' )
			line++;   // увели
		int cl = classOfSymbol( c );
		int a = automat[S][cl];
		int nextLineFlag = a % 10;
		a /= 10;
		int addCharToBufferFlag = a % 10;
		int newS = a / 10;
		if( nextLineFlag ) {
			confString->addAsLast( new ConfString );
			cursor->setToLast();
		}
		if( addCharToBufferFlag ) {
			ConfString *b = cursor->elementAt();
			ConfString &b1 = *b;
			int posElement = posElementsInString[newS];
			if( S == 2 || S == 5 ) {
				if( c == 'n' )
					b1[posElement] += '\n';
				else if( c == 'r' )
					b1[posElement] += '\r';
				else
					b1[posElement] += (char)c; // из-за наличия TCharString & operator += ( int ); указываем явно тип char
			} else {
				b1[posElement] += (char)c; // см. выше.
			}
		}
		S = newS;
	}

	if( S == 13 ) {     // Если мы вышли из цикла в результате ошибки
		char fname[MAX_FNAME], ext[MAX_EXT], msg[MAX_PATH + 80];

		// печатаем только имя файла - без пути к нему
		splitpath( fileName, 0, 0, fname, ext );
		sprintf( msg, "TCfg::parse: ошибка в строке %d файла конфигурации %s%s", line, fname, ext );

		throw TRequestExc( 1, msg );
	}

}


// Набор функций, определяющих классы символов
static int charIsEOF( int schar ) {
	if( schar == EOF )
		return 1;
	return 0;
}

static int charIsSpace( int schar ) {
	if( schar == ' ' || schar == '\t' )
		return 1;
	return 0;
}

static int charIsSlash( int schar ) {
	if( schar == '\\' )
		return 1;
	return 0;
}

static int charIsSharp( int schar ) {
	if ( schar == '#' )
		return 1;
	return 0;
}

static int charIsEOL( int schar ) {
	if( schar == '\n' || schar == '\r' )
		return 1;
	return 0;
}


// Функция определяет класс символа
// Возвращаемые классы:
// 0 --- конец файла
// 1 --- пробельный символ
// 2 --- слеш (экранирование)
// 3 --- решетка '#' --- начало комментария
// 4 --- конец строки
// 5 --- обычный символ (то, что не относится к вышеопределенному)
static int classOfSymbol( int schar ) {
	if( charIsEOF( schar ) ) {
		return 0;
	} else if( charIsSpace( schar ) ) {
		return 1;
	} else if( charIsSlash( schar ) ) {
		return 2;
	} else if( charIsSharp( schar ) ) {
		return 3;
	} else if( charIsEOL( schar ) ) {
		return 4;
	}
	return 5;
}


static int getTChar(void * f) {
	return getc( (FILE*)f );
}


static void protect( const TCharString & source, TCharString & dest ) {
	dest.clear();

	const char * s = source;
	char a;
	while( (a = *s++) ) {
		if( a == ' ' || a == '\t' || a == '#' || a == '\\' ||  a == '\r' ||  a == '\n' )
			dest += '\\';
		if( a == '\n' )
			dest += 'n' ;
		else if( a == '\r' )
			dest += 'r' ;
		else
			dest += a;      // все остальные символы
	}
}


static void strwrite(  FILE* f, const TCharString& c ) {
	fwrite( (const char *)c, 1, c.length(), f );
}


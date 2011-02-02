/*
*/

#include <cmath>
#include <stdarg.h>
#include <cstdarg>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <tc_config.h>

#include <../c_lib/include/c_lib/c_types.hpp>

using namespace std;
#define NUL '\0'
#define NUM(c) (c - '0')

ostream& _ostrformat( ostream& out, string& format, va_list ap );
int _istrscanf( istream& ins, string& format, va_list ap );

string strformat( char const * format, ... ) {
	va_list ap;
	va_start( ap, format );
	ostringstream out;
	string s( format );
	_ostrformat( out, s, ap );
	va_end( ap );
	return out.str();
}

string strformat( string& format, ... ) {
	va_list ap;
	va_start( ap, format );
	ostringstream out;
	_ostrformat( out, format, ap );
	va_end( ap );
	return out.str();
}

ostream& streamformat( ostream& out, string& format , ... ) {
	va_list ap;
	va_start( ap, format );
	_ostrformat( out, format, ap );
	va_end( ap );
	return out;
}

ostream& streamformat( ostream& out, char const * format , ... ) {
	va_list ap;
	va_start( ap, format );
	string s( format );
	_ostrformat( out, s, ap );
	va_end( ap );
	return out;
}

std::string& strformat2( std::string& dst, std::string& format , ... ) {
	va_list ap;
	va_start( ap, format );
	ostringstream out;
	_ostrformat( out, format, ap );
	va_end( ap );
	dst.assign( out.str() );
	return dst;
}

std::string& strformat2( std::string& dst, char const * format , ... ) {
	va_list ap;
	va_start( ap, format );
	ostringstream out;
	string s( format );
	_ostrformat( out, s, ap );
	va_end( ap );
	dst.assign( out.str() );
	return dst;
}

int strscanf( std::string& src, std::string& format , ... ) {
	va_list ap;
	va_start( ap, format );
	istringstream ins( src );
	int res = _istrscanf( ins, format, ap );
	va_end( ap );
	return res;
}

int strscanf( std::string& src, char const * format , ... ) {
	va_list ap;
	va_start( ap, format );
	istringstream ins( src );
	string s( format );
	int res = _istrscanf( ins, s, ap );
	va_end( ap );
	return res;
}

ostream& _ostrformat( ostream& out, string& format, va_list ap ) {
	static string form( " .-+#*0123456789" );
	enum int_type_enum {
		IS_LONG, IS_INT, IS_SHORT, IS_BYTE //64,32,16,8
	};

	enum int_type_enum int_type = IS_INT;

	enum float_type_enum {
		IS_LONGD, IS_DOUBLE // 80, 64
	};

	enum float_type_enum float_type = IS_DOUBLE;

	bool printBlank = false;
	ostringstream old;
	//ostringstream tmp;
	string::iterator fPtr;
	string::iterator uPtr;

	fPtr = format.begin();
	old.copyfmt( out );

	while ( fPtr != format.end() ) {
		if ( *fPtr != '%' ) {
			out << *fPtr;
		} else {
			printBlank = false;
			/*
			* Default variable settings
			*/
			out.setf( ios::right, ios::fmtflags(-1L) ); //Default adjustment to right
			out.width(0);
			out.precision(6);
			out.fill( ' ' );
			
			fPtr++;
			uPtr = fPtr;
			/*
			* Checking for flags, width or precision
			*/
			if ( form.find( *fPtr ) != string::npos ) {
				for ( bool breakout = false; !breakout; fPtr++ ) {
					switch ( *fPtr ) {
						case '-':
							out.setf( ios::left );
							break;
						case '+':
							out.setf( ios::showpos );
							break;
						case '#':
							out.setf( ios::showbase|ios::showpoint );
							break;
						case ' ':
							printBlank = true;
							//out.fill( ' ' );
							break;
						case '0':
							out.fill( '0' );
							break;
						default:
							breakout = true;
							break;
					}
				}
				/*
				* Check if a width was specified
				*/
				if ( isdigit( *fPtr ) ) {
					printBlank = false;
					int num = NUM( *fPtr++ );
					while ( isdigit( *fPtr ) ) {
						num *= 10 ;
						num += NUM( *fPtr++ );
					}
					out.width( num );
				} else if ( *fPtr == '*' ) {
					printBlank = false;
					int v = va_arg( ap, int );
					fPtr++;
					if ( v < 0 ) {
						out.setf( ios::left, ios::adjustfield );
						out.width( -v );
					} else
						out.width( v );
				}
				/*
				* Check if a precision was specified
				*/
				if ( *fPtr == '.' ) {
					fPtr++;
					if ( isdigit( *fPtr ) ) {
						int num = NUM( *fPtr++ );
						while ( isdigit( *fPtr ) ) {
							num *= 10 ;
							num += NUM( *fPtr++ );
						}
						out.precision( num );
					} else if ( *fPtr == '*' ) {
						int v = va_arg( ap, int );
						if ( v < 0 ) v = 0;
						out.precision( v );
						fPtr++;
					}
				}
			}
			switch ( *fPtr ) {
				case 'l':
					int_type = IS_LONG;
					fPtr++;
					if ( *fPtr == 'l' ) {
						float_type = IS_LONGD;
						fPtr++;
					}
					break;
				case 'h':
					int_type = IS_SHORT;
					fPtr++;
					if ( *fPtr == 'h' ) {
						int_type = IS_BYTE;
						fPtr++;
					}
					break;
				case 'L':
					int_type = IS_LONG;
					float_type = IS_LONGD;
					break;
				default:
					int_type = IS_INT;
					break;
			}
			/*
			* Argument extraction and printing.
			* First we determine the argument type.
			* Then, we convert the argument to a string.
			*/
			switch ( *fPtr ) {
				case 'u': {
					switch ( int_type ) {
						case IS_LONG:
							out << ( uint64_t ) va_arg( ap, uint64_t );
							break;
						case IS_INT:
							out << ( uint32_t ) va_arg( ap, uint32_t );
							break;
						case IS_SHORT:
							out << ( uint16_t ) va_arg( ap, unsigned int );
							break;
						case IS_BYTE:
							out << ( uint16_t )( 0xff & va_arg( ap, unsigned int ) );
							break;
					}
					break;
				}
				case 'd':
				case 'i':
					switch ( int_type ) {
						case IS_LONG: {
							int64_t i64 = va_arg( ap, int64_t );
							if ( printBlank && i64 >= 0 ) out << ' ';
							out << i64;
							break;
						}
						case IS_INT: {
							int32_t i32 =  va_arg( ap, int32_t );
							if ( printBlank && i32 >= 0 ) out << ' ';
							out << i32;
							break;
						}
						case IS_SHORT: {
							int16_t i16 =  va_arg( ap, int );
							if ( printBlank && i16 >= 0 ) out << ' ';
							out << i16;
							break;
						}
						case IS_BYTE: {
							//out << ( int8_t ) va_arg ( ap, int );
							int16_t i8 = ( int16_t )(( int8_t ) va_arg( ap, int ) );
							if ( printBlank && i8 >= 0 ) out << ' ';
							out << i8;
							break;
						}
					}
					break;
				case 'o':
					out.setf( ios::oct, ios::basefield );
					switch ( int_type ) {
						case IS_LONG:
							out << ( uint64_t ) va_arg( ap, uint64_t );
							break;
						case IS_INT:
							out << ( uint32_t ) va_arg( ap, uint32_t );
							break;
						case IS_SHORT:
							out << ( uint16_t ) va_arg( ap, unsigned int );
							break;
						case IS_BYTE:
							out << ( uint16_t )( 0xff & va_arg( ap, unsigned int ) );
							break;
					}
					break;
				case 'X':
					out << uppercase;
				case 'x':
					out << hex;
					switch ( int_type ) {
						case IS_LONG:
							out << ( uint64_t ) va_arg( ap, uint64_t );
							break;
						case IS_INT:
							out << ( uint32_t ) va_arg( ap, uint32_t );
							break;
						case IS_SHORT:
							out << ( uint16_t ) va_arg( ap, unsigned int );
							break;
						case IS_BYTE:
							out << ( uint16_t )( 0xff & va_arg( ap, unsigned int ) );
							break;
					}
					break;
				case 'S':
					out.setf( ios::uppercase );
				case 's':
					out << ( char* ) va_arg( ap, char * );
					break;
				case 'F':
					out.setf( ios::uppercase );
				case 'f': {
					out.setf( ios::fixed, ios::floatfield );
					switch ( float_type ) {
						case IS_DOUBLE: {
							double dt = va_arg( ap, double );
							if ( printBlank && dt >= 0 ) out << ' ';
							out << dt;
							break;
						}
						case IS_LONGD: {
							long double dt = va_arg( ap, long double );
							if ( printBlank && dt >= 0 ) out << ' ';
							out << dt;
							break;
						}
					}
					break;
				}
				case 'E':
					out.setf( ios::uppercase );
				case 'e': {
					out.setf( ios::scientific, ios::floatfield );
					switch ( float_type ) {
						case IS_DOUBLE: {
							double dt = va_arg( ap, double );
							if ( printBlank && dt >= 0 ) out << ' ';
							out << dt;
							break;
						}
						case IS_LONGD: {
							long double dt = va_arg( ap, long double );
							if ( printBlank && dt >= 0 ) out << ' ';
							out << dt;
							break;
						}
					}
					break;
				}
				case 'G':
					out.setf( ios::uppercase );
				case 'g': {
					switch ( float_type ) {
						case IS_DOUBLE: {
							double dt = va_arg( ap, double );
							if ( printBlank && dt >= 0 ) out << ' ';
							out << dt;
							break;
						}
						case IS_LONGD: {
							long double dt = va_arg( ap, long double );
							if ( printBlank && dt >= 0 ) out << ' ';
							out << dt;
							break;
						}
					}
					break;
				}
				case 'C':
					out.setf( ios::uppercase );
				case 'c':
					out << ( char )( va_arg( ap, int ) );
					break;
				case '%':
					out << '%';
					break;
					//case 'n':
					//  break;
				case 'P':
					out.setf( ios::uppercase );
				case 'p':
					out << ( void* ) va_arg( ap, void * );
					break;
				case NUL:
					/*
					* The last character of the format string was %.
					* We ignore it.
					*/
					continue;
				default:
					/*
					* The default case is for unrecognized %'s.
					* We print %<char> to help the user identify what
					* option is not understood.
					* This is also useful in case the user wants to pass
					* the output to another function that understands
					* some other %<char> (like syslog).
					*/
					out << '%';
					//out << format.substr( uPtr - format.begin(), fPtr - uPtr );
					while ( uPtr <= fPtr ) out << *uPtr++;
					break;
			}
		}
		fPtr++;
	}
	out.copyfmt( old );
	return out;
}

int _istrscanf( istream& ins, string& format, va_list ap ) {
	enum int_type_enum {
		IS_LONG, IS_INT, IS_SHORT, IS_BYTE //64,32,16,8
	};

	enum int_type_enum int_type = IS_INT;

	enum float_type_enum {
		IS_LONGD, IS_DOUBLE, IS_FLOAT //80,64,32
	};

	enum float_type_enum float_type = IS_FLOAT;

	int res = 0;
	char c;
	uint16_t u16t;
	int16_t  i16t;


	istringstream tmp;
	tmp.copyfmt( ins );
	ins.setf( ios::skipws, ios::fmtflags(-1L) );

	string::iterator fPtr = format.begin();
	bool breakout = false;

	while ( !breakout && fPtr != format.end() ) {
		if ( *fPtr != '%' ) {
			if ( !isspace( *fPtr ) ) {
				ins >> c;
				if ( c != *fPtr ) return res;
			}
		} else {
			/*
			* Default variable settings
			*/
			//ins.copyfmt( tmp );
			fPtr++;

			if ( *fPtr == 'l' ) {
				int_type = IS_LONG;
				float_type = IS_DOUBLE;
				fPtr++;
				if ( *fPtr == 'l' ) {
					float_type = IS_LONGD;
					fPtr++;
				}
			} else if ( *fPtr == 'h' ) {
				int_type = IS_SHORT;
				fPtr++;
				if ( *fPtr == 'h' ) {
					int_type = IS_BYTE;
					fPtr++;
				}
			} else if ( *fPtr == 'L' ) {
				int_type = IS_LONG;
				float_type = IS_LONGD;
			} else {
				int_type = IS_INT;
				float_type = IS_FLOAT;
			}
			/*
			* Argument extraction and printing.
			* First we determine the argument type.
			* Then, we convert the argument to a string.
			*/

			switch ( *fPtr ) {
				case 'u': {
					switch ( int_type ) {
						case IS_LONG:
							if ( ins >> *(( uint64_t* ) va_arg( ap, uint64_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_INT:
							if ( ins >> *(( uint32_t* ) va_arg( ap, uint32_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_SHORT:
							if ( ins >> *(( uint16_t* ) va_arg( ap, uint16_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_BYTE:
							if ( ins >> u16t ) {
								*(( uint8_t* ) va_arg( ap, uint8_t* ) ) = ( uint8_t ) u16t;
								res++;
							}							else breakout = true;
							break;
					}
					break;
				}
				case 'd':
				case 'i':
					switch ( int_type ) {
						case IS_LONG:
							if ( ins >> *(( int64_t* ) va_arg( ap, int64_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_INT:
							if ( ins >> *(( int32_t* ) va_arg( ap, int32_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_SHORT:
							if ( ins >> *(( int16_t* ) va_arg( ap, int16_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_BYTE:
							if ( ins >> i16t ) {
								*(( int8_t* ) va_arg( ap, int8_t* ) ) = ( int8_t ) i16t;
								res++;
							}							else breakout = true;
							break;
					}
					break;
				case 'o':
					ins >> oct;
					switch ( int_type ) {
						case IS_LONG:
							if ( ins >> *(( uint64_t* ) va_arg( ap, uint64_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_INT:
							if ( ins >> *(( uint32_t* ) va_arg( ap, uint32_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_SHORT:
							if ( ins >> *(( uint16_t* ) va_arg( ap, uint16_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_BYTE:
							if ( ins >> u16t ) {
								*(( uint8_t* ) va_arg( ap, uint8_t* ) ) = ( uint8_t ) u16t;
								res++;
							} 							else breakout = true;
							break;
					}
					break;
				case 'X':
				case 'x':
					ins >> hex;
					switch ( int_type ) {
						case IS_LONG:
							if ( ins >> *(( uint64_t* ) va_arg( ap, uint64_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_INT:
							if ( ins >> *(( uint32_t* ) va_arg( ap, uint32_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_SHORT:
							if ( ins >> *(( uint16_t* ) va_arg( ap, uint16_t* ) ) ) res++;
							else breakout = true;
							break;
						case IS_BYTE:
							if ( ins >> u16t ) {
								*(( uint8_t* ) va_arg( ap, uint8_t* ) ) = ( uint8_t ) u16t;
								res++;
							}							else breakout = true;
							break;
					}
					break;
				case 'S':
				case 's':
					if ( ins >> *(( string * )va_arg( ap, string* ) ) ) res++;
					else breakout = true;
					break;
				case 'F':
				case 'f':
				case 'E':
				case 'e':
				case 'g':
				case 'G': {
					switch ( float_type ) {
						case IS_LONGD:
							if ( ins >> *(( long double* ) va_arg( ap, long double* ) ) ) res++;
							else breakout = true;
							break;
						case IS_DOUBLE:
							if ( ins >> *(( double* ) va_arg( ap, double* ) ) ) res++;
							else breakout = true;
							break;
						case IS_FLOAT:
							if ( ins >> *(( float* ) va_arg( ap, float* ) ) ) res++;
							else breakout = true;
							break;
						default:
							breakout = true;
							break;
					}
					break;
				}
				//  break;
				case 'C':
				case 'c':
					if( ins >> *(( char* )( va_arg( ap, int* ) ) ) ) res ++;
					else breakout = true;
					break;
				case '%': {
					if ( !(( ins >> c ) && ( c == '%' ) ) ) breakout = true;
					break;
				}
				case 'P':
				case 'p':
					if ( ins >> *(( void** ) va_arg( ap, void ** ) ) ) res ++;
					else breakout = true;
					break;
				case NUL:
					/*
					* The last character of the format string was %.
					* We ignore it.
					*/
					continue;
				default:
					return res;
					break;
			}
		}
		fPtr++;
	}
	ins.copyfmt( tmp );
	return res;
}

// kate: indent-mode cstyle; replace-tabs off; tab-width 4;  replace-tabs off;  replace-tabs off;  replace-tabs off;  replace-tabs off;

#include <tc_config.h>
#include <stdio.h>
#include <c_lib.hpp>

class XML {
private:
	static const int max_number_tags = 10;

	char file_name[MAX_PATH];
	FILE* file_header;
	std::string tags[max_number_tags];      /* Встреченные теги */
	std::string btags[max_number_tags];     /* Основная часть встреченных тегов */
	int number_tags;                        /* Число незавершенных тегов */
	std::string text;                       /* текст, идущий в составе тега */
	std::string tag;                        /* основная часть имени тега */
	int line;
	char ahead_char;
	bool ahead_flag;

	char* buffer;
	int buffer_length;
	int current_char;
	std::string result;
public:
	XML( char* file_name ) throw (TAccessExc);
	XML( char* buffer, int length ) throw (TRequestExc);
	~XML();
	char get_next_char();
	char get_ahead_char();
	int get_lexem() throw (TRequestExc);
	void file_close( char* file_name );
	std::string & get_text( int n, char** tags ) throw (TRequestExc);
	//    string toString(){ return string(buffer); }
	void toBegin() {
		current_char = -1;
		number_tags = 0;
	};
};

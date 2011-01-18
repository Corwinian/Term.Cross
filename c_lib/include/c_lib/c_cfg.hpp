/*-----------------------------------------------------------------------------
	c_cfg.hpp
 
	Класс TCfg - реализует доступ к файлам конфигурации.
 
	Формат конфигурационного файла.
	Конфигурационный файл является текстовым файлом.  Текстовый файл в
	свою очередь, состоит из строк, которые могут разделяться
	следующими комбинациями символов: 0D, 0A, 0D 0A, 0A 0D.
 
	Строки могут:
	- быть пустыми
	- начинаться с символа #, тогда вся строка является комментарием
	- содержать ключ, значение параметра и комментарий
 
	Более подробно, последний случай может быть описан как
	<ключ>[<промежуток>[<значение>]][<промежуток>][#<комментарий>]
	<ключ>				> {<символ>}*
	<промежуток>		  > {<пробельный символ>}*
	<значение>			> {<символ>}*
	<комментарий>		 > {<символ>}*
	<символ>			  > любой символ кроме 00, 0D и 0A
	<пробельный символ>   > пробел (20) или табуляция (09)
	Символам пробела, табуляции, '#', перевода каретки и возврата строки,
	'\' внутри ключа или значения должен предшествовать символ '\'.
-----------------------------------------------------------------------------*/
#ifndef _C_CFG_HPP_
#define _C_CFG_HPP_

#include <stdlib.h>
#include <tc_config.h>
#include <c_lib/c_types.hpp>
#include <c_lib/c_except.hpp>

#include <c_lib/c_slst.hpp>
#include <c_lib/c_misc.hpp>
#include "string"
#warning подумать могет перенести в другое место
#include <memory.h>
using namespace std;
// Внутр. константа.  Вынесена наружу из-за ошибки в VisuaAge C++
const int NElementsInString = 11;

class TCfg {
public:
	/*
		Cоздание пустой конфигурации.
	*/
	TCfg();

	/*
		Cоздание конфигурации из файла.
		Файл должен соответствовать описанному выше формату.
		Исключения:
		TAccessExc  --- ошибка работы с файлом;
		TRequestExc --- ошибка разбора файла.
	*/
	TCfg( const char* fileName ) throw (TAccessExc, TRequestExc);

	virtual ~TCfg();

	const char * fullCfgPath() const {
		return fileName;
	}
	const char * cfgName() const;
	/*
		Сохранение конфигурации в файл.  Если имя файла не указано, использеется
		иия, указанное при создании объекта, или при последнем сохранении конфигурации.
		Исключения:
		TAccessExc ---  ошибка работы с файлом;
		TParamExc  ---  ни при вызове этой функции, ни при вызове конструктора не было
						указано имя файла конфигурации
	*/
	void saveCfg( const char * fileName = 0 ) throw (TParamExc, TAccessExc);

	/*
		Проверка существования параметра с указанным значение ключа.
		Исключения:
		TParamExc --- задан нулевой ключ.
	*/
	int containsParamWithKey( const char * keyName ) throw (TParamExc);

	/*
		Получение значения параметра по заданному ключу.
		Исключения:
		TParamExc   --- задан нулевой ключ;
		TRequestExc --- параметр с указанным ключем не найден.
	*/
	const char * getValue( const char * keyName ) throw (TParamExc, TRequestExc);

	/*
		Добавление или замена параметра.
		Добавление параметра к конфигурации, или, если параметр с таким
		ключом существует, замена существующего значения параметра и
		комментария, на новые.
		Если значение параметра не указано, оно не изменяется, или, в случае
		добавления, остается пустым.
		Если комментарий не указан, его значение не изменяется, или, в случае
		добавления, остается пустым.
		Исключения:
		TParamExc --- задан нулевой ключ.
	*/
	void addOrReplaceParam( const char* keyName, const char* value = 0, const char* comment = 0 ) throw (TParamExc);

	/*
	 *  Работа с конфигурацией с помощью курсора
	 *  (для графической оболочки).
	 *  Вызов любой из ранее перечисленных фуккций
	 *  (saveCfg, paramExist, getValue, addOrReplaceParam)
	 *  делает курсор недействительным.
	 *  Также курсор недействителен сразу после создания конфигурации.
	 */

	/*
		Перемещение курсора в начало конфигурации.
	*/
	void setToFirst( void );

	/*
		Перемещение курсора к параметру с заданным ключем.
		TParamExc   --- задан нулевой ключ;
		TRequestExc --- параметра с заданным ключем в конфигурации нет.
	*/
	void setTo( const char* key ) throw (TParamExc, TRequestExc);

	/*
		Проверка действительности ключа.
	*/
	int isValid( void );

	/*
		Переход к следующему параметру.
		После последнего параметра курсор становится недействительным.
		Исключения:
		TRequestExc --- курсор недействителен.
	*/
	void setToNext(void) throw (TRequestExc);

	/*
		Функции получения ключа, значения и комментария текущего параметра
		(параметра, на который указывает курсор).
		Исключения:
		TRequestExc --- курсор недействителен.
	*/
	const char* getCurrentKey(void) throw(TRequestExc);
	const char* getCurrentValue(void) throw(TRequestExc);
	const char* getCurrentComment(void) throw(TRequestExc);

	/*
		Модификация значений ключа, значения и комментария текущего параметра.
		(параметра, на который указывает курсор).
		Исключения:
		TParamExc   --- указан нулевой ключ;
		TRequestExc --- курсор недействителен.
	*/
	void setCurrentKey( const char* keyName ) throw(TParamExc, TRequestExc);
	void setCurrentValue( const char* value ) throw(TRequestExc);
	void setCurrentComment( const char* comment ) throw(TRequestExc);

	/*   Примечание к функциям работы с курсором:
	 *	курсор должен использоваться следующим образом:
	 *	 for( cfg.setToFirst(); cfg.isValid(); cfg.setToNext() ){
	 *		 // Вызовы функций get и set...
	 *	 }
	 */

private:
	// Класс, описывающий одну строку файла конфигурации
	class ConfString {
	public:
		ConfString(): keyFlag(0) {}
		;
		string& operator[] (int i ) {
			return confString[i];
		};
		void setIsKey() {
			keyFlag = 1;
		}
		;	// функция, вызываемая для установки флага
		int isKey() {
			return keyFlag;
		};
	private:
		string confString[NElementsInString];
		int keyFlag;	// 1 - строка содержит ключ, 0 - строка пустая или содержит только комментарий
	};

	// Список строк конфигурационного файла (двунаправленный список)
	TSimpleList<ConfString>* confString;
	TSimpleListCursor<ConfString>* cursor;

	// Имя файла конфигурации
	char fileName[MAX_PATH];
	//char* fileName;

	void parse( int (*getChar)(void*), void* charSource ) throw (TRequestExc);
	void confToParams();
	void setToParam(void);
};

#endif


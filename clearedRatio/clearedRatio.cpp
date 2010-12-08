/***********************************************
 clearedRatio.cpp
 Программа, показывающая, какая часть спутникового изображения является
 действительной, т.е. >= 0 для двухбайтовых данных и != 0
 для однобайтовых.
 Программа не определяет формат данных, и последний должен
 указываться пользователем.
 Формат вызова прогрммы:
    clearedRatio [опции] <имя исходного файла>

 Для справки о опциях - запустить без параметров.
    clearedRatio

 Возможные опции:
    -1 - исходный файл является файлом однобайтовых данных 
    -2 - исходный файл является файлом двубайтовых данных (по-умолчанию)
    -p - результат будет возвращен в целых процентах  
    -P - резулат будет выведен в дробных процентах (по-умолчанию)
    -f - результат будет выведен в виде несокращенной дроби
    -v - результат будет снабжен текстовыми комментариями
    -q - на экран не выводится ничего. Результат выводится в целых
         процентах в коде возврата программы. прим. данная опция не 
         совместима с опциями -P, -f, -v
 Возвращаемое значение:
    в случае, если не указана опция -q, то 0 - в случае удачного
    завершения программы, 1 - ошибка открытия файла, 2 - слишком 
    короткий файл, 3 - ошибка опций ком. строки.
    если опция -q не указана, 0-100 - удачное завершение программы,
    255 - ошибка открытия файла, 254 - слишком короткий файл,
    253 - ошибка опций ком. строки.
***********************************************/
#include <stdio.h>
#include <math.h>
// #include <getopt.h>
#include <unistd.h>

int option_1 = 0;
int option_2 = 0;
int option_p = 0;
int option_P = 0;
int option_f = 0;
int option_v = 0;
int option_q = 0;
int option_err = 0;  // Ошибка командной строки

long n_pixel;
long good_pixel;
const char * file_name;

int main(int argc, char**argv ){
   if( argc <= 1){
       printf( "There are no args!\n" );
       return 0;
   }

   int c;
   opterr = 0;
   while ( (c=getopt( argc, argv, "12pPfvq" )) != -1 ){
      switch(c){
         case '1': option_1 = 1; break;
         case '2': option_2 = 1; break;
         case 'p': option_p = 1; break;
         case 'P': option_P = 1; break;
         case 'f': option_f = 1; break;
         case 'v': option_v = 1; break;
         case 'q': option_q = 1; break;
         case '?': option_err = 1; break;
      }
   } 
   if( argc - 1 == optind ){
       file_name = argv[optind];
   }
   else {
       option_err = 1;
   }
   if( option_1 == 1 && option_2 == 1 ) option_err = 1;
   if( option_p == 1 && option_P == 1 ) option_err = 1;
   if( option_p == 1 && option_f == 1 ) option_err = 1;
   if( option_P == 1 && option_f == 1 ) option_err = 1;
   if( option_q == 1 && option_v == 1 ) option_err = 1;
   if( option_q == 1 && option_p == 1 ) option_err = 1;
   if( option_q == 1 && option_P == 1 ) option_err = 1;
   if( option_q == 1 && option_f == 1 ) option_err = 1;

   if( option_err ){
       if( option_q == 1 ){
          return 256;
       }
       fprintf( stderr, "Command line error\n" );
       return 3;
   }

   if( option_1 == 0 && option_2 == 0 ) option_2 = 1;
   if( option_p == 0 && option_P == 0 && option_f == 0 ) option_P = 1;

   FILE *f = fopen( file_name, "rb" );
   if( option_v ) printf( "Input file name: %s\n", file_name );
   if( f == 0 ){
       if( option_q ) return 255;
       else {
          perror( file_name );
          return 1;
       } 
   }

   char b0[256];
   if( fread( b0, 1, 256, f ) != 256 ){
       if( option_q ) return 254;
       else {
          if( option_v ) fprintf( stderr, "Too short file %s\n", file_name );
          return 2;
       } 
   }

   if( option_1 ){
       unsigned char p;
       while( fread( &p, 1, 1, f) == 1 ){
          if( p != 0 ) good_pixel++;
           n_pixel++;
       }
   }
   else { // if( option_2 )
       signed short int p;
       while( fread( &p, 2, 1, f) == 1 ){
          if( p >= 0 ) good_pixel++;
           n_pixel++;
       }
   }

   fclose(f);
   if( n_pixel == 0 ){
       if( option_q ) return 254;
       else {
          if( option_v ) fprintf( stderr, "Too short file %s\n", file_name );
          return 2;
       } 
   }

   double percent_d = (good_pixel*100.0)/n_pixel;
   int percent_i = int(floor( percent_d + 0.5 ));

   if( option_q ){
       return percent_i;
   }
   if( option_p ){
       if( option_v) printf( "percent of good pixels = %d\n", percent_i );
       else printf( "%d\n", percent_i );
   }
   else if( option_P ){
       if( option_v) printf( "percent of good pixels = %f\n", percent_d );
       else printf( "%f\n", percent_d );
   }
   else { // if( option_f ){
       if( option_v) printf( "good pixels: %ld of %ld\n", good_pixel, n_pixel );
       else printf( "%ld/%ld\n", good_pixel, n_pixel );
   }
   return 0;
 
}

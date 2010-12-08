/* demo_use.c -- demonstrate direct use of the "hello" routine */

//#include "libhello.hpp"
#include <c_lib.hpp>
#include <iostream>

using namespace std;

int main(void) {
    cout<<"Test0\n";
    try{
	cout<<"Test1\n";
//    cout.flush();
	throw TException(0,"SUXX");
    }catch (TException ex){
	cout<<"Test2\n";
	cout<<ex.text()<<"\n";
    }
    cout<<"Test3\n";
	cout<<check_dir( "C:/CONFIG.SYS" );
    return 0;
}

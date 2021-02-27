#include<iostream>
using namespace std;

void func(){
    throw exception();
}

int main(){
    cout<<"before"<<endl;
    func();
    cout<<"after"<<endl;
    return 0;
}
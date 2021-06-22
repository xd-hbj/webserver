#include<iostream>
#include<queue>
#include<set>
#include<forward_list>
using namespace std;

class A{
    public:
        static int x;
};

int A::x=5;

void init(){
    A::x = 11;
    static int y = 1;
    
}

void init1(){
    A::x = 20;
}
    
int main(){
    cout<<A::x<<endl;
    init();
    cout<<A::x<<endl;
    init1();
    cout<<A::x<<endl;
    return 0;
}
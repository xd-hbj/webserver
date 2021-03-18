#include<iostream>
#include<queue>
#include<set>
#include<forward_list>
using namespace std;

class ClxBase{
public:
    ClxBase() {};
    virtual ~ClxBase() {cout << "Output from the destructor of class ClxBase!" << endl;};

    void DoSomething() { cout << "Do something in class ClxBase!" << endl; };
    int x;
};

class ClxDerived : public ClxBase{
public:
    ClxDerived() {};
    ~ClxDerived() { cout << "Output from the destructor of class ClxDerived!" << endl; };

    void DoSomething() { cout << "Do something in class ClxDerived!" << endl; };
    int y;
};

    
int main(){
    ClxBase* ptr = new ClxDerived[5];
    
    delete ptr;
    return 0;
}
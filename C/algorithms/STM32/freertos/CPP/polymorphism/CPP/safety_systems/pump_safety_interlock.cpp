#include <iostream>
#include <string>
using namespace std;

#define MAX_PRESSURE 300.0 
#define MAX_BUBBLE_COUNT 3.0  
#define MIN_DOSE 0.1
#define MAX_DOSE 50.0 
#define MAX_CHECKER 50

class SafetyChecker{
    protected:
        float value;
    public:
        SafetyChecker(float a){
            value=a;
        }
        virtual bool runCheck() =0;
        virtual string getFaultDescription() const=0;
        virtual string getName() const=0;
        virtual ~SafetyChecker()=default;
};

class OcclusionDetector:public SafetyChecker{
    public:
        OcclusionDetector(float a):SafetyChecker(a){}
        bool runCheck()override{
            if(value<MAX_PRESSURE && value>0){
                return true;
            }
            else if(value>MAX_PRESSURE && value<500){
                return false;
            }
            else{
                cout<<"Invalid Pressure Detected"<<endl;
                return false;
            }
        }
        string getFaultDescription() const override{
            return "HIGH LINE PRESSURE OCCLUSION DETECTED";
        }
        string getName()const override{
            return "Occlusion Detector";
        }
};

class AirBubbleDetector:public SafetyChecker{
    public:
        AirBubbleDetector(float a):SafetyChecker(a){}
        bool runCheck()override{
            if(value>0 && value<MAX_BUBBLE_COUNT){
                return true;
            }
            else if(value>MAX_BUBBLE_COUNT && value<10){
                return false;
            }
            else{
                cout<<"Invalid bubble count detected"<<endl;
                return false;
            }
        }
        string getFaultDescription() const override{
            return "CRITICAL AIR BUBBLES IN DELIVERY LINE";
        }
        string getName() const override{
            return "Air Bubble Detector";
        }
};

class DoseRangeChecker:public SafetyChecker{
    public:
        DoseRangeChecker(float a):SafetyChecker(a){}
        bool runCheck()override{
            if(value>0.1 && value<50.0){
                return true;
            }
            else if(value>0 && value<75){
                return false;
            }
            else{
                cout<<"Invalid Dose Range"<<endl;
                return false;
            }
        }
        string getFaultDescription() const override{
            return "OUT OF BOUNDS INFUSION DOSE REQUESTED";
        }
        string getName()const override{
            return "Dose Range Checker";
        }
};

class SafetyInterLock{
    private:
        SafetyChecker* Checker[MAX_CHECKER];
        int count=0;
    public:
        SafetyInterLock(){
            count=0;
            for(int i=0;i<MAX_CHECKER;i++){
                Checker[i]=NULL;
            }
        }
        void registerChecker(SafetyChecker* checker){
            if(count<MAX_CHECKER){
                Checker[count]=checker;
                count++;
            }
        }
        bool runAllChecks(){
            for(int i=0;i<count;i++){
               if(Checker[i]->runCheck()==false){
                  cout<<"PUMP DELIVERY BLOCKED — ["<<Checker[i]->getFaultDescription()			  <<"]"<<endl;
                  return false;
        }
    }                else{
                    cout<<"Pump Delivery Authorized"<<endl;
                    return true;
                }
            }
        }
        void printStatus(){
            for(int i=0;i<count;i++){
            cout<<Checker[i]->getName()<<" staus: ";
            if(Checker[i]->runCheck()==true){
                cout<<"Pass"<<endl;
            }
            else if(Checker[i]->runCheck()==false){
                cout<<"Fail"<<endl;
            }
            }
        }
};

int main()
{
{
    cout << "==== TEST 1: ALL SENSORS WITHIN SAFE BOUNDS ====" << endl;
    
    SafetyInterLock Interlock;
    OcclusionDetector O(250.0);
    AirBubbleDetector A(2);
    DoseRangeChecker D(30.0);
    
    Interlock.registerChecker(&O);
    Interlock.registerChecker(&A);
    Interlock.registerChecker(&D);
    
    Interlock.runAllChecks();
    Interlock.printStatus();
}
{
    cout << "\n==== TEST 2: UNSAFE OCCLUSION TRIGGERED ====" << endl;
    
    SafetyInterLock Interlock;
    OcclusionDetector O(350.0);
    AirBubbleDetector A(2);
    DoseRangeChecker D(30.0);
    
    Interlock.registerChecker(&O);
    Interlock.registerChecker(&A);
    Interlock.registerChecker(&D);
    
    Interlock.runAllChecks();
    Interlock.printStatus();
}
{
    cout << "\n==== TEST 2: UNSAFE Air Bubble TRIGGERED ====" << endl;
    
    SafetyInterLock Interlock;
    OcclusionDetector O(350.0);
    AirBubbleDetector A(4);
    DoseRangeChecker D(30.0);
    
    Interlock.registerChecker(&O);
    Interlock.registerChecker(&A);
    Interlock.registerChecker(&D);
    
    Interlock.runAllChecks();
    Interlock.printStatus();
}
{
    cout << "\n==== TEST 2: UNSAFE Air Bubble TRIGGERED ====" << endl;
    
    SafetyInterLock Interlock;
    OcclusionDetector O(350.0);
    AirBubbleDetector A(4);
    DoseRangeChecker D(55.0);
    
    Interlock.registerChecker(&O);
    Interlock.registerChecker(&A);
    Interlock.registerChecker(&D);
    
    Interlock.runAllChecks();
    Interlock.printStatus();
}
}

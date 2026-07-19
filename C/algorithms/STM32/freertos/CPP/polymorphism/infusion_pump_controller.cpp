#include <iostream>
#include<string>
using namespace std;
class InfusionPump{
    protected:
        string ModelName;
        float flowRateMlPerLit=0.0,max_SafeRate=0.0;
    public:
        InfusionPump(string a,float b){
            ModelName=a;
            max_SafeRate=b;
        }
        virtual bool setFlowRate(float rate)=0;
        virtual void deliver()=0;
        void status(){
            cout<<ModelName<<" : "<<flowRateMlPerLit<<endl;
        }
};
 class SyringePump:public InfusionPump{
     public:
        SyringePump(string a,float b):InfusionPump(a,b){}
        bool setFlowRate(float rate) override{
            if(rate<=max_SafeRate){
                flowRateMlPerLit=rate;
                return true;
            }
            else{
                return false;
            }
        }
        void deliver() override{
            cout<<"["<<ModelName<<"]"<<"Delivering via Syringe Pump mechanism at"<<flowRateMlPerLit<<"ml/hr"<<endl;
        }
 };
 class VolumetricPump:public InfusionPump{
     public:
        VolumetricPump(string a,float b):InfusionPump(a,b){}
        bool setFlowRate(float rate) override{
            if(rate<=max_SafeRate){
                flowRateMlPerLit=rate;
                return true;
            }
            else{
                return false;
            }
        }
        void deliver() override {
            cout<<"["<<ModelName<<"]"<<" Delivering via Volumetric Pump mechanism at"<<flowRateMlPerLit<<"ml/hr"<<endl;
        }
 };
int main()
{
    InfusionPump *I[2];
    VolumetricPump V("Volumetric Pump",999.0);
    SyringePump S("Syringe Pump",50.0);
    I[0]=&V;
    I[1]=&S;
    float arr1[2]={49.0,55.0};
    float arr2[2]={900.0,1000.0};
    for(int i=0;i<2;i++){
        bool a=S.setFlowRate(arr1[i]);
        if(a==false){
            cout<<" REJECTED: requested rate exceeds safe limit for Syringe Pump"<<endl;
        }
    }
    for(int i=0;i<2;i++){
        bool b=V.setFlowRate(arr2[i]);
        if(b==false){
            cout<<"REJECTED: requested rate exceeds safe limit for Volumetric Pump"<<endl;
        }
    }
    for(int i=0;i<2;i++){
        I[i]->deliver();
        I[i]->status();
    }
}

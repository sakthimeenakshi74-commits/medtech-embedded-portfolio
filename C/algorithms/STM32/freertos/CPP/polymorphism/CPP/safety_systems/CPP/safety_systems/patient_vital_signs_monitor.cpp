#include <iostream>
#include <string>
using namespace std;
#define FAULT_LOW 0
#define ECG_MIN 60
#define ECG_MAX 100
#define ECG_CRITICAL_LOW 50
#define ECG_CRITICAL_HIGH 130
#define ECG_FAULT_HIGH 300
#define SPO2_NORMAL 95
#define SPO2_CRITICAL 90
#define SPO2_FAULT_HIGH 100
#define SYS_MIN 90
#define SYS_MAX 140
#define DAI_MIN 60
#define DIA_MAX 90
#define SYS_CRITICAL 180
#define DIA_CRITICAL 120

class VitalSign{
    protected:
        string vitalName;
        // Bug Fix: Initialized variables to prevent garbage memory data
        bool faultDetected = false;
        bool alarmActive = false;
    public:
        VitalSign(string a){
            vitalName=a;
        }
        virtual bool read_Sensor()=0;
        virtual void checkAlarm()=0;
        virtual void display()=0;
        void reportFault(){
            // Bug Fix: Corrected inverted logic (Trigger print when fault is true)
            if(faultDetected==true){
                cout<<"["<<vitalName<<"] SENSOR FAULT — last reading invalid"<<endl;
            }
        }
};
class ECGVital:public VitalSign{
    private:
        int heartRate = 0; // Bug Fix: Initialized tracking variable
        // Bug Fix: Removed duplicate 'FaultDetection' and 'ECGAlarm' fields 
        // to use inherited base class members (faultDetected and alarmActive)
    public:
        ECGVital(string a):VitalSign(a){}
        bool read_Sensor() override {
            heartRate=64;
            if(heartRate<=FAULT_LOW || heartRate>ECG_FAULT_HIGH){
                // Bug Fix: Fixed inverted logic (True means a fault exists)
                faultDetected=true;
                return faultDetected;
            }
            faultDetected=false;
            return faultDetected;
        }
        void checkAlarm() override {
            // Bug Fix: Reset alarm state before executing checks
            alarmActive = false; 
            if(heartRate<ECG_CRITICAL_LOW || heartRate>ECG_CRITICAL_HIGH){
                alarmActive=true;
            }
            if(heartRate<ECG_CRITICAL_LOW && alarmActive==1){
                cout<<"Bradycardia detected"<<endl;
            }
            else if(heartRate>ECG_CRITICAL_HIGH && alarmActive==1){
                cout<<"Tachycardia detected"<<endl;
            }
        }
        void display() override {
            cout<<"ECG : "<<heartRate<<" bpm";
            if(alarmActive==true){
                cout<<" [WARNING]"<<endl;
            }
            else if(faultDetected==true){ // Bug Fix: Handled fault value correctly
                cout<<" [Fault]"<<endl;
            }
            else if(heartRate>=60 && heartRate<=100){
                cout<<" [Normal]"<<endl;
            }
            else {
                cout << endl;
            }
        }
};
class SpO2Vital:public VitalSign{
    private:
        int SpO2 = 0; // Bug Fix: Initialized tracking variable
        // Bug Fix: Removed duplicate fields to use inherited base class members
    public:
        SpO2Vital(string a):VitalSign(a){}
        bool read_Sensor() override {
            SpO2=85;
            if(SpO2<=FAULT_LOW || SpO2>SPO2_FAULT_HIGH){
                // Bug Fix: Fixed inverted logic (True means a fault exists)
                faultDetected=true;
                return faultDetected;
            }
            faultDetected=false;
            return faultDetected;
        }
        void checkAlarm() override {
            alarmActive = false; // Bug Fix: Reset alarm state
            if(SpO2<SPO2_CRITICAL){
                alarmActive=true;
                cout<<"SpO2 is dropping"<<endl;
            }
        }
        void display() override {
            cout<<"SpO2 : "<<SpO2<<" %";
            if(alarmActive==true){
                cout<<" [WARNING]"<<endl;
            }
            else if(faultDetected==true){ // Bug Fix: Handled fault value correctly
                cout<<" [Fault]"<<endl;
            }
            else if(SpO2>=50 && SpO2<SPO2_FAULT_HIGH){
                cout<<" [Normal]"<<endl;
            }
            else {
                cout << endl;
            }
        }
};
class NIBPVital:public VitalSign{
    private:
        int systolic=0,diastolic=64;
        // Bug Fix: Removed duplicate fields to use inherited base class members
    public:
        NIBPVital(string a):VitalSign(a){}
        bool read_Sensor() override {
            systolic=0,diastolic=63;
            if(systolic<=FAULT_LOW || diastolic<=FAULT_LOW ||systolic<diastolic){
                // Bug Fix: Fixed inverted logic (True means a fault exists)
                faultDetected=true;
                return faultDetected;
            }
            faultDetected=false;
            return faultDetected;
        }
        void checkAlarm() override {
            alarmActive = false; // Bug Fix: Reset alarm state
            if(systolic>SYS_CRITICAL || diastolic >DIA_CRITICAL){
                alarmActive=true;
                cout<<"hypertensive crisis"<<endl;
            }
        }
        void display() override {
            cout<<"Systolic: "<<systolic<<"mmHg   Diastolic: "<<diastolic<<"mmHg "; // Cleaned spacing
            if(alarmActive==true){
                cout<<"[WARNING]"<<endl;
            }
            else if(faultDetected==true){ // Bug Fix: Handled fault value correctly
                cout<<"[Fault]"<<endl;
            }
            else if(systolic>90 && systolic <140 && diastolic>60 && diastolic<90){
                cout<<"[Normal]"<<endl;
            }
            else {
                cout << endl;
            }
        }
};
int main()
{
    VitalSign *V[3];
    ECGVital E("ECG");
    SpO2Vital S("SpO2");
    NIBPVital N("NIBP");
    V[0]=&E;
    V[1]=&S;
    V[2]=&N;
    for(int i=0;i<3;i++){
        bool fault=V[i]->read_Sensor();
        // Bug Fix: Updated conditional branch logic to reflect fixed boolean meanings
        if(fault==true){
            V[i]->reportFault();
        }
        else{
            V[i]->checkAlarm();
        }
        V[i]->display();
    }
}

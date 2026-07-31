#include <iostream>
#include <cstdint> // Required for fixed-width integer types
using namespace std;

// Maximum number of readings to store in the circular buffer
#define BUFFERSIZE 5

class DataLogger{
    private:
        uint16_t DataBuffer[BUFFERSIZE]; // Array to hold the sensor data
        uint8_t head;                     // Pointer for the current insertion position
        uint8_t count;                    // Current number of items in the buffer
        uint32_t totalSum;                // Running sum to calculate average efficiently

    public:
        DataLogger(){
            reset(); // Initialize the buffer on creation
        }
        
        // Function to add a new reading, overwriting old data if the buffer is full
        void AddReading(uint16_t newReading){
            if(count==BUFFERSIZE){
                // If buffer is full, subtract the oldest value from the total
                totalSum -=DataBuffer[head];
            }
            else{
                // If not full, increment the count of items
                count++;
            }
            // Add the new reading to the current position
            DataBuffer[head]=newReading;
            totalSum+=newReading;
            
            // Move the head forward and wrap around to 0 if necessary
            head=(head+1)%BUFFERSIZE;
        }
        
        // Function to calculate and return the average of the stored readings
        const uint16_t GetAverage(){
            if (count==0){
                return 0; // Prevent division by zero
            }
            else{
                return totalSum/count; // Return the average
            }
        }
        
        // Function to reset all buffer parameters to zero
        void reset(){
            head=0;
            count=0;
            totalSum=0;
            for(int i=0;i<BUFFERSIZE;i++){
                DataBuffer[i]=0;
            }
        }
};

int main()
{
    DataLogger D;
    // Input data for testing
    int Data[BUFFERSIZE]={190,389,38,29,48};
    
    // Process each reading and display the average
    for(int i=0;i<BUFFERSIZE;i++){
        D.AddReading(Data[i]);
        cout<<"Average of the buffer is "<<D.GetAverage()<<endl;
    }
    
    D.reset(); // Clear data for next use
    return 0;
}

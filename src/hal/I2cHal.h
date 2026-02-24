#pragma once
class I2cHal { 
    public: static void begin(int sda, int scl); 
    bool recoverBus();    
};

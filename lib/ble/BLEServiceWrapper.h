#ifndef BLE_SERVICE_WRAPPER_H
#define BLE_SERVICE_WRAPPER_H

#include <BLEService.h>
#include <vector>
#include "BLECharWrapper.h"

class BLEServiceWrapper {
public:
    BLEServiceWrapper(BLEService* service);

    // Create a characteristic with builder pattern
    BLECharBuilder addCharacteristic(const char* uuid);

    // Service lifecycle
    void start();
    void stop();

    // Get service UUID
    const char* getUUID();

    // Access underlying service if needed
    BLEService* raw();

private:
    BLEService* _service;
    std::vector<BLECharWrapper*> _characteristics;
};

#endif

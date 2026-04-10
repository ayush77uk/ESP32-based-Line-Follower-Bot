#ifndef BLE_MANAGER_H
#define BLE_MANAGER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>
#include <functional>
#include <vector>
#include "BLEServiceWrapper.h"

using ConnectionCallback = std::function<void()>;

class BLEManager {
public:
    // Singleton access
    static BLEManager& getInstance();

    // Delete copy constructor and assignment operator
    BLEManager(const BLEManager&) = delete;
    BLEManager& operator=(const BLEManager&) = delete;

    // Initialization
    void init(const char* deviceName);

    // Connection callbacks
    void onConnect(ConnectionCallback callback);
    void onDisconnect(ConnectionCallback callback);

    // Service management
    BLEServiceWrapper* createService(const char* uuid);

    // Advertising control
    void startAdvertising();
    void stopAdvertising();
    
    // Configure advertising
    void setAdvertisingServiceUUID(const char* uuid);
    void setScanResponse(bool enable);

    // Connection state
    bool isConnected();

    // Access underlying objects if needed
    BLEServer* getServer();
    BLEAdvertising* getAdvertising();

private:
    BLEManager();
    ~BLEManager();

    friend class BLEManagerServerCallbacks;

    BLEServer* _server;
    BLEAdvertising* _advertising;
    std::vector<BLEServiceWrapper*> _services;
    
    bool _connected;
    ConnectionCallback _connectCallback;
    ConnectionCallback _disconnectCallback;
};


class BLEManagerServerCallbacks : public BLEServerCallbacks {
public:
    BLEManagerServerCallbacks(BLEManager* manager);
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;

private:
    BLEManager* _manager;
};

#endif

#include "BLEManager.h"

// ============== BLEManagerServerCallbacks ==============

BLEManagerServerCallbacks::BLEManagerServerCallbacks(BLEManager* manager)
    : _manager(manager) {}

void BLEManagerServerCallbacks::onConnect(BLEServer* pServer) {
    _manager->_connected = true;
    if (_manager->_connectCallback) {
        _manager->_connectCallback();
    }
}

void BLEManagerServerCallbacks::onDisconnect(BLEServer* pServer) {
    _manager->_connected = false;
    if (_manager->_disconnectCallback) {
        _manager->_disconnectCallback();
    }
}


// ============== BLEManager ==============

BLEManager& BLEManager::getInstance() {
    static BLEManager instance;
    return instance;
}

BLEManager::BLEManager()
    : _server(nullptr),
      _advertising(nullptr),
      _connected(false),
      _connectCallback(nullptr),
      _disconnectCallback(nullptr) {}

BLEManager::~BLEManager() {
    for (auto service : _services) {
        delete service;
    }
}

void BLEManager::init(const char* deviceName) {
    BLEDevice::init(deviceName);
    
    _server = BLEDevice::createServer();
    _server->setCallbacks(new BLEManagerServerCallbacks(this));
    
    _advertising = BLEDevice::getAdvertising();
    _advertising->setScanResponse(false);
    _advertising->setMinPreferred(0x0);
}

void BLEManager::onConnect(ConnectionCallback callback) {
    _connectCallback = callback;
}

void BLEManager::onDisconnect(ConnectionCallback callback) {
    _disconnectCallback = callback;
}

BLEServiceWrapper* BLEManager::createService(const char* uuid) {
    BLEService* service = _server->createService(uuid);
    BLEServiceWrapper* wrapper = new BLEServiceWrapper(service);
    _services.push_back(wrapper);
    
    // Auto-add to advertising
    _advertising->addServiceUUID(uuid);
    
    return wrapper;
}

void BLEManager::startAdvertising() {
    BLEDevice::startAdvertising();
}

void BLEManager::stopAdvertising() {
    _advertising->stop();
}

void BLEManager::setAdvertisingServiceUUID(const char* uuid) {
    _advertising->addServiceUUID(uuid);
}

void BLEManager::setScanResponse(bool enable) {
    _advertising->setScanResponse(enable);
}

bool BLEManager::isConnected() {
    return _connected;
}

BLEServer* BLEManager::getServer() {
    return _server;
}

BLEAdvertising* BLEManager::getAdvertising() {
    return _advertising;
}

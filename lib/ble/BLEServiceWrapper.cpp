#include "BLEServiceWrapper.h"

BLEServiceWrapper::BLEServiceWrapper(BLEService* service)
    : _service(service) {}

BLECharBuilder BLEServiceWrapper::addCharacteristic(const char* uuid) {
    return BLECharBuilder(_service, uuid);
}

void BLEServiceWrapper::start() {
    _service->start();
}

void BLEServiceWrapper::stop() {
    _service->stop();
}

const char* BLEServiceWrapper::getUUID() {
    return _service->getUUID().toString().c_str();
}

BLEService* BLEServiceWrapper::raw() {
    return _service;
}

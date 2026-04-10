#include "BLECharWrapper.h"

// ============== BLECharWrapper ==============

BLECharWrapper::BLECharWrapper(BLECharacteristic* characteristic)
    : _characteristic(characteristic), _writeCallback(nullptr), _readCallback(nullptr) {}

void BLECharWrapper::setValue(const char* value) {
    _characteristic->setValue((uint8_t*)value, strlen(value));
}

void BLECharWrapper::setValue(uint8_t* data, size_t len) {
    _characteristic->setValue(data, len);
}

void BLECharWrapper::setValue(uint32_t value) {
    _characteristic->setValue(value);
}

void BLECharWrapper::setValue(int value) {
    _characteristic->setValue(value);
}

void BLECharWrapper::setValue(float value) {
    _characteristic->setValue(value);
}

void BLECharWrapper::setValue(double value) {
    _characteristic->setValue(value);
}

std::string BLECharWrapper::getValue() {
    return _characteristic->getValue();
}

uint8_t* BLECharWrapper::getData() {
    return _characteristic->getData();
}

void BLECharWrapper::notify() {
    _characteristic->notify();
}

void BLECharWrapper::indicate() {
    _characteristic->indicate();
}

void BLECharWrapper::onWrite(WriteCallback callback) {
    _writeCallback = callback;
}

void BLECharWrapper::onRead(ReadCallback callback) {
    _readCallback = callback;
}

BLECharacteristic* BLECharWrapper::raw() {
    return _characteristic;
}


// ============== BLECharBuilderCallbacks ==============

BLECharBuilderCallbacks::BLECharBuilderCallbacks(BLECharWrapper* wrapper)
    : _wrapper(wrapper) {}

void BLECharBuilderCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    if (_wrapper->_writeCallback) {
        _wrapper->_writeCallback(pCharacteristic->getData(), pCharacteristic->getLength());
    }
}

void BLECharBuilderCallbacks::onRead(BLECharacteristic* pCharacteristic) {
    if (_wrapper->_readCallback) {
        _wrapper->_readCallback();
    }
}


// ============== BLECharBuilder ==============

BLECharBuilder::BLECharBuilder(BLEService* service, const char* uuid)
    : _service(service), _uuid(uuid) {}

BLECharBuilder& BLECharBuilder::withRead() {
    _properties |= BLECharacteristic::PROPERTY_READ;
    return *this;
}

BLECharBuilder& BLECharBuilder::withWrite() {
    _properties |= BLECharacteristic::PROPERTY_WRITE;
    return *this;
}

BLECharBuilder& BLECharBuilder::withWriteNR() {
    _properties |= BLECharacteristic::PROPERTY_WRITE_NR;
    return *this;
}

BLECharBuilder& BLECharBuilder::withNotify() {
    _properties |= BLECharacteristic::PROPERTY_NOTIFY;
    return *this;
}

BLECharBuilder& BLECharBuilder::withIndicate() {
    _properties |= BLECharacteristic::PROPERTY_INDICATE;
    return *this;
}

BLECharBuilder& BLECharBuilder::onWrite(WriteCallback callback) {
    _writeCallback = callback;
    return *this;
}

BLECharBuilder& BLECharBuilder::onRead(ReadCallback callback) {
    _readCallback = callback;
    return *this;
}

BLECharWrapper* BLECharBuilder::build() {
    BLECharacteristic* characteristic = _service->createCharacteristic(_uuid, _properties);
    
    // Add descriptor for notify/indicate
    if (_properties & (BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE)) {
        characteristic->addDescriptor(new BLE2902());
    }

    BLECharWrapper* wrapper = new BLECharWrapper(characteristic);
    wrapper->_writeCallback = _writeCallback;
    wrapper->_readCallback = _readCallback;

    // Set callbacks if any were registered
    if (_writeCallback || _readCallback) {
        characteristic->setCallbacks(new BLECharBuilderCallbacks(wrapper));
    }

    return wrapper;
}

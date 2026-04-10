#ifndef BLE_CHAR_WRAPPER_H
#define BLE_CHAR_WRAPPER_H

#include <BLEService.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>
#include <functional>
#include <cstring>

class BLEServiceWrapper;

using WriteCallback = std::function<void(uint8_t* data, size_t len)>;
using ReadCallback = std::function<void()>;

class BLECharBuilder;

class BLECharWrapper {
public:
    BLECharWrapper(BLECharacteristic* characteristic);

    // Value operations
    void setValue(const char* value);
    void setValue(uint8_t* data, size_t len);
    void setValue(uint32_t value);
    void setValue(int value);
    void setValue(float value);
    void setValue(double value);
    
    std::string getValue();
    uint8_t* getData();

    // Notification/Indication
    void notify();
    void indicate();

    // Set callbacks after creation
    void onWrite(WriteCallback callback);
    void onRead(ReadCallback callback);

    // Access underlying characteristic if needed
    BLECharacteristic* raw();

private:
    friend class BLECharBuilder;
    friend class BLECharBuilderCallbacks;
    BLECharacteristic* _characteristic;
    WriteCallback _writeCallback;
    ReadCallback _readCallback;
};


class BLECharBuilderCallbacks : public BLECharacteristicCallbacks {
public:
    BLECharBuilderCallbacks(BLECharWrapper* wrapper);
    void onWrite(BLECharacteristic* pCharacteristic) override;
    void onRead(BLECharacteristic* pCharacteristic) override;

private:
    BLECharWrapper* _wrapper;
};


class BLECharBuilder {
public:
    BLECharBuilder(BLEService* service, const char* uuid);

    BLECharBuilder& withRead();
    BLECharBuilder& withWrite();
    BLECharBuilder& withWriteNR();  // Write without response
    BLECharBuilder& withNotify();
    BLECharBuilder& withIndicate();

    BLECharBuilder& onWrite(WriteCallback callback);
    BLECharBuilder& onRead(ReadCallback callback);

    BLECharWrapper* build();

private:
    BLEService* _service;
    const char* _uuid;
    uint32_t _properties = 0;
    WriteCallback _writeCallback = nullptr;
    ReadCallback _readCallback = nullptr;
};

#endif

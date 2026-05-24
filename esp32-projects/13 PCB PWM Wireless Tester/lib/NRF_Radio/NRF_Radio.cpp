// This file implements the NRF Radio Class Method

#include "NRF_Radio.h" // Uses the .h file that holds the class defintion 
#include <SPI.h>

// Define the Constructor
// ---------------------- Constructor ----------------------
NrfRadio::NrfRadio(uint8_t cePin, uint8_t csnPin, const uint8_t* rxAddress, const uint8_t* txAddress, uint8_t nodeId) 
    :
    // Assigns the pins and radio addresses 
    _radio(cePin, csnPin),
    _rxAddress(rxAddress),
    _txAddress(txAddress),
    _nodeId(nodeId),
    _connected(false),
    _lastRxMs(0),
    _lastHeartbeatTxMs(0),
    _counter(0)
    
    {

    }
        
    
// ------------------- METHOD 1 Begin() ----------------------
bool NrfRadio::begin() {
  // Start SPI (ESP32 default SPI pins are used unless you pass custom pins)
  SPI.begin();

  // Initialize the NRF24L01
  if (!_radio.begin()) {
    _connected = false;
    return false;
  }

  // Basic radio settings (both boards must match these)
  _radio.setDataRate(RF24_1MBPS);
  _radio.setPALevel(RF24_PA_LOW);
  _radio.setChannel(76);

  // Fixed payload size (RadioPayload must be <= 32 bytes)
  _radio.setPayloadSize(sizeof(RadioPayload));

  // We are using heartbeat/timeout for "connection", not hardware ACKs
  _radio.setAutoAck(false);

  // Configure pipes (listen on RX pipe, send to TX pipe)
  _radio.openReadingPipe(1, _rxAddress);
  _radio.openWritingPipe(_txAddress);

  // Start in receive mode
  _radio.startListening();

  // Initialize connection timers/state
  unsigned long now = millis();
  _lastRxMs = now;
  _lastHeartbeatTxMs = now;
  _connected = false;

  return true;
}

// ------------------- METHOD 2 sendPackage() ----------------------
bool NrfRadio::sendPackage(const RadioPayload& p) {
  _radio.stopListening();                              // switch to TX mode
  bool ok = _radio.write(&p, sizeof(RadioPayload));    // transmit packet
  _radio.startListening();                             // back to RX mode

  if (ok) {
    if (p.type != PacketType::HEARTBEAT) {
        _counter++;
    }
  }

  return ok;
}

// ------------------- METHOD 3 receivePackage() ----------------------
bool NrfRadio::receivePackage(RadioPayload& out) {
  if (!_radio.available()) {
    return false;
  }

  _radio.read(&out, sizeof(RadioPayload));

  // We received something -> update connection timing/state
  _lastRxMs = millis();
  _connected = true;

  return true;
}

// ------------------- METHOD 4 serviceConnection() ----------------------
void NrfRadio::serviceConnection() {
  unsigned long now = millis();

  // 1) Send heartbeat periodically
  if (now - _lastHeartbeatTxMs >= _heartbeatIntervalMs) {
    RadioPayload hb{};
    hb.type = PacketType::HEARTBEAT;
    hb.counter = _counter;     // optional but fine
    hb.pot1Raw = 0;
    hb.pot2Raw = 0;
    
    sendPackage(hb);

    _lastHeartbeatTxMs = now;
  }

  // 2) Disconnect timeout check (based on last receive time)
  if (_connected && (now - _lastRxMs >= _disconnectTimeoutMs)) {
    _connected = false;
  }
}

bool NrfRadio::isConnected() const {
  return _connected;
}
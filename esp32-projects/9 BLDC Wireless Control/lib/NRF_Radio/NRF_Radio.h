// This file goes over the NRF Radio Class

#pragma once

#include <Arduino.h>
#include <RF24.h>
#include "RadioPayload.h"

class NrfRadio {
public:
  // Constructor: pass pins, addresses, and node id (1 or 2)
  NrfRadio(uint8_t cePin, uint8_t csnPin, const uint8_t* rxAddress, const uint8_t* txAddress, uint8_t nodeId);

  // METHOD 1: Initialize radio settings (call once in setup)
  // Returns true if radio initialized successfully
  bool begin();

  // METHOD 2: Send the datapackage (TEXT / HEARTBEAT / future CONTROL)
  // Returns true if the write succeeded
  bool sendPackage(const RadioPayload& p);

  // METHOD 3: Receive the datapackage (bidirectional)
  // Returns true only when a NEW packet was read into 'out'
  bool receivePackage(RadioPayload& out);

  // METHOD 4: Connection service (call every loop)
  // - Sends heartbeat when the interval expires
  // - Checks timeout and updates connection state
  void serviceConnection();

  // Connection status
  bool isConnected() const;

private:
  // --- RF24 driver ---
  RF24 _radio; // 

  // --- addressing / identity ---
  const uint8_t* _rxAddress;
  const uint8_t* _txAddress;
  uint8_t _nodeId;

  // --- timing / connection ---
  bool _connected;
  unsigned long _lastRxMs;
  unsigned long _lastHeartbeatTxMs;

  // Heartbeat + timeout settings (tune later)
  unsigned long _heartbeatIntervalMs = 500;
  unsigned long _disconnectTimeoutMs = 1500;

  // --- payload tracking ---
  uint16_t _counter;
  char _lastSent[24]; //stores the last text sent
  char _lastReceived[24]; //stores the last text recieved

  // --- internal helpers ---
  void sendHeartbeat();
  void handleReceived(const RadioPayload& p);
};
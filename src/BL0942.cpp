#include "BL0942.h"

void BL0942::readPacket_() {
  serial_.readBytes(reinterpret_cast<byte*>(&buffer_), sizeof(buffer_));
}

byte BL0942::calcChecksum_() const {
  byte checksum = READ_COMMAND + PACKET_HEADER;
  // Whole package but checksum
  auto *raw = reinterpret_cast<const byte *>(&buffer_);
  for (size_t i = 0; i < sizeof(buffer_) - 1; i++) {
      checksum += raw[i];
  }
  checksum ^= 0xFF;
  return checksum;
}

BL0942::BL0942(HardwareSerial &serial)
  : serial_(serial) {
}

void BL0942::begin() {
    serial_.begin(4800);
}

void BL0942::startMeasurement() {
  serial_.write(READ_COMMAND);
  serial_.write(FULL_PACKET);
}

bool BL0942::update() {
  if (serial_.available() < sizeof(buffer_) + 1) {
    return false;
  }
  if (serial_.read() != PACKET_HEADER) {
    //TODO: logging
    return false;
  }
  readPacket_();
  byte checksum = calcChecksum_();
  if (checksum != buffer_.checksum) {
    //TODO: logging
    return false;
  }
  i_rms_ = buffer_.i_rms * V_REF / I_COEFF;
  v_rms_ = buffer_.v_rms * V_REF / V_COEFF;
  p_rms_ = buffer_.watt * V_REF * V_REF / P_COEFF;
  return true;
}

float BL0942::getIRms() const {
  return i_rms_;
}

float BL0942::getVRms() const {
  return v_rms_;
}

float BL0942::getPRms() const {
  return p_rms_;
}

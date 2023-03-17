#ifndef BL0942_h
#define BL0942_h

#include <Arduino.h>

struct DataPacket {
  uint32_t i_rms : 24;
  uint32_t v_rms : 24;
  uint32_t i_fast_rms : 24;
  int32_t watt : 24;
  uint32_t cf_cnt : 24;
  uint16_t frequency;
  uint8_t reserved1;
  uint8_t status;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t checksum;
} __attribute__((packed)) __attribute__((scalar_storage_order("little-endian")));

class BL0942 {
  static constexpr byte READ_COMMAND = 0x58;
  static constexpr byte FULL_PACKET = 0xAA;
  static constexpr byte PACKET_HEADER = 0x55;
  static constexpr float V_REF = 1.218;
  static constexpr int I_COEFF = 305978;
  static constexpr int V_COEFF = 73989;
  static constexpr int P_COEFF = 3537;
  HardwareSerial &serial_;
  float i_rms_ = 0;
  float v_rms_ = 0;
  float p_rms_ = 0;
  void readPacket_();
  byte calcChecksum_() const;
public:
  DataPacket buffer_;
  BL0942(HardwareSerial &serial);
  void begin();
  void startMeasurement();
  bool update();
  float getIRms() const;
  float getVRms() const;
  float getPRms() const;
};

#endif //BL0942_h

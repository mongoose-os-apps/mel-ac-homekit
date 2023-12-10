// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef DB_H
#define DB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Total number of services and characteristics contained in the accessory.
 */
// #define kAttributeCount ((size_t) 26)

/**
 * Services
 */
extern HAPService ThermostatService;
extern const HAPUInt8Characteristic ThermostatCurrentHCstateCharacteristic;
extern const HAPUInt8Characteristic ThermostatTargetHCstateCharacteristic;
extern const HAPFloatCharacteristic ThermostatCurrentTempCharacteristic;
extern const HAPFloatCharacteristic ThermostatTargetTempCharacteristic;
extern const HAPBoolCharacteristic ThermostatStatusActiveCharacteristic;

extern HAPService VaneVertService;
extern const HAPUInt8Characteristic VaneVertCurrentSateCharacteristic;
extern const HAPIntCharacteristic VaneVertCurrentTiltAngleCharacteristic;
extern const HAPIntCharacteristic VaneVertTargetTiltAngleCharacteristic;
extern const HAPUInt8Characteristic VaneVertSwingModeCharacteristic;
extern const HAPBoolCharacteristic VaneVertStatusActiveCharacteristic;

extern HAPService VaneHorizService;
extern const HAPUInt8Characteristic VaneHorizCurrentSateCharacteristic;
extern const HAPIntCharacteristic VaneHorizCurrentTiltAngleCharacteristic;
extern const HAPIntCharacteristic VaneHorizTargetTiltAngleCharacteristic;
extern const HAPUInt8Characteristic VaneHorizSwingModeCharacteristic;
extern const HAPBoolCharacteristic VaneHorizStatusActiveCharacteristic;

extern HAPService FanService;
extern const HAPUInt8Characteristic FanActiveCharacteristic;
extern const HAPUInt8Characteristic FanCurrentSateCharacteristic;
extern const HAPUInt8Characteristic FanTargetSateCharacteristic;
extern const HAPFloatCharacteristic FanRotationSpeedCharacteristic;
extern const HAPBoolCharacteristic FanStatusActiveCharacteristic;

extern HAPService ModeFanService;
extern const HAPBoolCharacteristic ModeFanStatusActiveCharacteristic;
extern const HAPBoolCharacteristic ModeFanOnCharacteristic;

extern HAPService ModeDryService;
extern const HAPBoolCharacteristic ModeDryStatusActiveCharacteristic;
extern const HAPBoolCharacteristic ModeDryOnCharacteristic;

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif

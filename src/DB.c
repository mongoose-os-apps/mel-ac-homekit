// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "DB.h"

#include "App.h"
#include "mgos.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * IID constants.
 */

#define kIID_Thermostat ((uint64_t) 0x0130)
#define kIID_ThermostatServiceSignature ((uint64_t) 0x0131)
#define kIID_ThermostatName ((uint64_t) 0x0132)
#define kIID_ThermostatCurrentTemp ((uint64_t) 0x0134)
#define kIID_ThermostatTargetTemp ((uint64_t) 0x0135)
#define kIID_ThermostatCurrentHCstate ((uint64_t) 0x0136)
#define kIID_ThermostatTargetHCstate ((uint64_t) 0x0137)
#define kIID_ThermostatHeatingThreshold ((uint64_t) 0x0138)
#define kIID_ThermostatCoolingThreshold ((uint64_t) 0x0139)
#define kIID_ThermostatTemperatureDisplayUnits ((uint64_t) 0x013A)
#define kIID_ThermostatStatusActive ((uint64_t) 0x013B)

#define kIID_VaneVert ((uint64_t) 0x0230)
#define kIID_VaneVertServiceSignature ((uint64_t) 0x0231)
#define kIID_VaneVertName ((uint64_t) 0x0232)
#define kIID_VaneVertCurrentState ((uint64_t) 0x0233)
#define kIID_VaneVertType ((uint64_t) 0x0234)
#define kIID_VaneVertCurrentTiltAngle ((uint64_t) 0x0235)
#define kIID_VaneVertTargetTiltAngle ((uint64_t) 0x0236)
#define kIID_VaneVertSwingMode ((uint64_t) 0x0237)
#define kIID_VaneVertStatusActive ((uint64_t) 0x0238)

#define kIID_VaneHoriz ((uint64_t) 0x0330)
#define kIID_VaneHorizServiceSignature ((uint64_t) 0x0331)
#define kIID_VaneHorizName ((uint64_t) 0x0332)
#define kIID_VaneHorizCurrentState ((uint64_t) 0x0333)
#define kIID_VaneHorizType ((uint64_t) 0x0334)
#define kIID_VaneHorizCurrentTiltAngle ((uint64_t) 0x0335)
#define kIID_VaneHorizTargetTiltAngle ((uint64_t) 0x0336)
#define kIID_VaneHorizSwingMode ((uint64_t) 0x0337)
#define kIID_VaneHorizStatusActive ((uint64_t) 0x0338)

#define kIID_Fan ((uint64_t) 0x0430)
#define kIID_FanServiceSignature ((uint64_t) 0x0431)
#define kIID_FanName ((uint64_t) 0x0432)
#define kIID_FanActive ((uint64_t) 0x0433)
#define kIID_FanCurrentState ((uint64_t) 0x0434)
#define kIID_FanTargetState ((uint64_t) 0x0435)
#define kIID_FanRotationSpeed ((uint64_t) 0x0436)
#define kIID_FanStatusActive ((uint64_t) 0x0437)

#define kIID_ModeFan ((uint64_t) 0x0530)
#define kIID_ModeFanServiceSignature ((uint64_t) 0x0531)
#define kIID_ModeFanName ((uint64_t) 0x0532)
#define kIID_ModeFanOn ((uint64_t) 0x0533)
#define kIID_ModeFanStatusActive ((uint64_t) 0x0534)

#define kIID_ModeDry ((uint64_t) 0x0630)
#define kIID_ModeDryServiceSignature ((uint64_t) 0x0631)
#define kIID_ModeDryName ((uint64_t) 0x0632)
#define kIID_ModeDryOn ((uint64_t) 0x0633)
#define kIID_ModeDryStatusActive ((uint64_t) 0x0634)

// HAP_STATIC_ASSERT(kAttributeCount == 9 + 3 + 5 + 9, AttributeCount_mismatch);

/**
 * The 'Service Signature' characteristic of the Light Bulb service.
 */

static const HAPDataCharacteristic ThermostatServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_ThermostatServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

// static const HAPStringCharacteristic ThermostatNameCharacteristic = {
//     .format = kHAPCharacteristicFormat_String,
//     .iid = kIID_ThermostatName,
//     .characteristicType = &kHAPCharacteristicType_Name,
//     .debugDescription = kHAPCharacteristicDebugDescription_Name,
//     .manufacturerDescription = NULL,
//     .properties = {.readable = true,
//                    .writable = false,
//                    .supportsEventNotification = false,
//                    .hidden = false,
//                    .requiresTimedWrite = false,
//                    .supportsAuthorizationData = false,
//                    .ip = {.controlPoint = false,
//                           .supportsWriteResponse = false},
//                    .ble = {.supportsBroadcastNotification = false,
//                            .supportsDisconnectedNotification = false,
//                            .readableWithoutSecurity = false,
//                            .writableWithoutSecurity = false}},
//     .constraints = {.maxLength = 64},
//     .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

const HAPFloatCharacteristic ThermostatCurrentTempCharacteristic = {
    .format = kHAPCharacteristicFormat_Float,
    .iid = kIID_ThermostatCurrentTemp,
    .characteristicType = &kHAPCharacteristicType_CurrentTemperature,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentTemperature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_Celsius,
    .constraints =
        {
            .minimumValue = -50.0,
            .maximumValue = +50.0,
            .stepValue = 0.1,
        },
    .callbacks = {.handleRead = HandleThermostatCurrentTempRead,
                  .handleWrite = NULL}};

const HAPFloatCharacteristic ThermostatTargetTempCharacteristic = {
    .format = kHAPCharacteristicFormat_Float,
    .iid = kIID_ThermostatTargetTemp,
    .characteristicType = &kHAPCharacteristicType_TargetTemperature,
    .debugDescription = kHAPCharacteristicDebugDescription_TargetTemperature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_Celsius,
    .constraints = {.minimumValue = 16.0,
                    .maximumValue = 31.0,
                    .stepValue = 0.5},
    .callbacks = {.handleRead = HandleThermostatTargetTempRead,
                  .handleWrite = HandleThermostatTargetTempWrite}};

const HAPUInt8Characteristic ThermostatCurrentHCstateCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_ThermostatCurrentHCstate,
    .characteristicType = &kHAPCharacteristicType_CurrentHeatingCoolingState,
    .debugDescription =
        kHAPCharacteristicDebugDescription_CurrentHeatingCoolingState,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 2,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleThermostatCurrentHCstateRead,
                  .handleWrite = NULL}};

const HAPUInt8Characteristic ThermostatTargetHCstateCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_ThermostatTargetHCstate,
    .characteristicType = &kHAPCharacteristicType_TargetHeatingCoolingState,
    .debugDescription =
        kHAPCharacteristicDebugDescription_TargetHeatingCoolingState,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 3,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleThermostatTargetHCstateRead,
                  .handleWrite = HandleThermostatTargetHCstateWrite}};

const HAPUInt8Characteristic ThermostatTemperatureDisplayUnitsCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_ThermostatTemperatureDisplayUnits,
    .characteristicType = &kHAPCharacteristicType_TemperatureDisplayUnits,
    .debugDescription =
        kHAPCharacteristicDebugDescription_TemperatureDisplayUnits,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 1,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleThermostatTemperatureDisplayUnitsRead,
                  .handleWrite = HandleThermostatTemperatureDisplayUnitsWrite}};

const HAPBoolCharacteristic ThermostatStatusActiveCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_ThermostatStatusActive,
    .characteristicType = &kHAPCharacteristicType_StatusActive,
    .debugDescription = kHAPCharacteristicDebugDescription_StatusActive,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleStatusActiveRead, .handleWrite = NULL}};

HAPService ThermostatService = {
    .iid = kIID_Thermostat,
    .serviceType = &kHAPServiceType_Thermostat,
    .debugDescription = kHAPServiceDebugDescription_Thermostat,
    .name = NULL,  // Set from config.
    .properties = {.primaryService = true,
                   .hidden = false,
                   .ble = {.supportsConfiguration = false}},
    .linkedServices = (const uint16_t[]){kIID_Fan, kIID_VaneHoriz,
                                         kIID_VaneVert, 0},  // NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &ThermostatServiceSignatureCharacteristic,
        //&ThermostatNameCharacteristic,
        &ThermostatCurrentHCstateCharacteristic,
        &ThermostatTargetHCstateCharacteristic,
        &ThermostatCurrentTempCharacteristic,
        &ThermostatTargetTempCharacteristic,
        &ThermostatTemperatureDisplayUnitsCharacteristic,
        &ThermostatStatusActiveCharacteristic, NULL}};

// VaneVert service

static const HAPDataCharacteristic VaneVertServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_VaneVertServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

static const HAPStringCharacteristic VaneVertNameCharacteristic = {
    .format = kHAPCharacteristicFormat_String,
    .iid = kIID_VaneVertName,
    .characteristicType = &kHAPCharacteristicType_Name,
    .debugDescription = kHAPCharacteristicDebugDescription_Name,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 64},
    .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

const HAPUInt8Characteristic VaneVertCurrentSateCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_VaneVertCurrentState,
    .characteristicType = &kHAPCharacteristicType_CurrentSlatState,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentSlatState,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 2,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleVaneVertCurrentStateRead,
                  .handleWrite = NULL}};

const HAPUInt8Characteristic VaneVertTypeCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_VaneVertType,
    .characteristicType = &kHAPCharacteristicType_SlatType,
    .debugDescription = kHAPCharacteristicDebugDescription_SlatType,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 1,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleVaneVertTypeRead, .handleWrite = NULL}};

const HAPIntCharacteristic VaneVertCurrentTiltAngleCharacteristic = {
    .format = kHAPCharacteristicFormat_Int,
    .iid = kIID_VaneVertCurrentTiltAngle,
    .characteristicType = &kHAPCharacteristicType_CurrentTiltAngle,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentTiltAngle,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_ArcDegrees,
    .constraints = {.minimumValue = -90, .maximumValue = 90, .stepValue = 1},
    .callbacks = {.handleRead = HandleVaneVertCurrentTiltAngleRead,
                  .handleWrite = NULL}};

const HAPIntCharacteristic VaneVertTargetTiltAngleCharacteristic = {
    .format = kHAPCharacteristicFormat_Int,
    .iid = kIID_VaneVertTargetTiltAngle,
    .characteristicType = &kHAPCharacteristicType_TargetTiltAngle,
    .debugDescription = kHAPCharacteristicDebugDescription_TargetTiltAngle,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_ArcDegrees,
    .constraints = {.minimumValue = -90, .maximumValue = 90, .stepValue = 45},
    .callbacks = {.handleRead = HandleVaneVertTargetTiltAngleRead,
                  .handleWrite = HandleVaneVertTargetTiltAngleWrite}};

const HAPUInt8Characteristic VaneVertSwingModeCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_VaneVertSwingMode,
    .characteristicType = &kHAPCharacteristicType_SwingMode,
    .debugDescription = kHAPCharacteristicDebugDescription_SwingMode,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 1,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleVaneVertSwingModeRead,
                  .handleWrite = HandleVaneVertSwingModeWrite}};

const HAPBoolCharacteristic VaneVertStatusActiveCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_VaneVertStatusActive,
    .characteristicType = &kHAPCharacteristicType_StatusActive,
    .debugDescription = kHAPCharacteristicDebugDescription_StatusActive,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleStatusActiveRead, .handleWrite = NULL}};

HAPService VaneVertService = {
    .iid = kIID_VaneVert,
    .serviceType = &kHAPServiceType_Slat,
    .debugDescription = kHAPServiceDebugDescription_Slat,
    .name = NULL,  // Set from config.
    .properties = {.primaryService = false,
                   .hidden = false,
                   .ble = {.supportsConfiguration = false}},
    .linkedServices = NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &VaneVertServiceSignatureCharacteristic, &VaneVertNameCharacteristic,
        &VaneVertCurrentSateCharacteristic, &VaneVertTypeCharacteristic,
        &VaneVertCurrentTiltAngleCharacteristic,
        &VaneVertTargetTiltAngleCharacteristic,
        &VaneVertSwingModeCharacteristic, &VaneVertStatusActiveCharacteristic,
        NULL}};

// VaneHoriz service

static const HAPDataCharacteristic VaneHorizServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_VaneHorizServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

static const HAPStringCharacteristic VaneHorizNameCharacteristic = {
    .format = kHAPCharacteristicFormat_String,
    .iid = kIID_VaneHorizName,
    .characteristicType = &kHAPCharacteristicType_Name,
    .debugDescription = kHAPCharacteristicDebugDescription_Name,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 64},
    .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

const HAPUInt8Characteristic VaneHorizCurrentSateCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_VaneHorizCurrentState,
    .characteristicType = &kHAPCharacteristicType_CurrentSlatState,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentSlatState,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 2,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleVaneHorizCurrentStateRead,
                  .handleWrite = NULL}};

const HAPUInt8Characteristic VaneHorizTypeCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_VaneHorizType,
    .characteristicType = &kHAPCharacteristicType_SlatType,
    .debugDescription = kHAPCharacteristicDebugDescription_SlatType,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 1,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleVaneHorizTypeRead, .handleWrite = NULL}};

const HAPIntCharacteristic VaneHorizCurrentTiltAngleCharacteristic = {
    .format = kHAPCharacteristicFormat_Int,
    .iid = kIID_VaneHorizCurrentTiltAngle,
    .characteristicType = &kHAPCharacteristicType_CurrentTiltAngle,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentTiltAngle,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_ArcDegrees,
    .constraints = {.minimumValue = -90, .maximumValue = 90, .stepValue = 1},
    .callbacks = {.handleRead = HandleVaneHorizCurrentTiltAngleRead,
                  .handleWrite = NULL}};

const HAPIntCharacteristic VaneHorizTargetTiltAngleCharacteristic = {
    .format = kHAPCharacteristicFormat_Int,
    .iid = kIID_VaneHorizTargetTiltAngle,
    .characteristicType = &kHAPCharacteristicType_TargetTiltAngle,
    .debugDescription = kHAPCharacteristicDebugDescription_TargetTiltAngle,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_ArcDegrees,
    .constraints = {.minimumValue = -90, .maximumValue = 90, .stepValue = 45},
    .callbacks = {.handleRead = HandleVaneHorizTargetTiltAngleRead,
                  .handleWrite = HandleVaneHorizTargetTiltAngleWrite}};

const HAPUInt8Characteristic VaneHorizSwingModeCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_VaneHorizSwingMode,
    .characteristicType = &kHAPCharacteristicType_SwingMode,
    .debugDescription = kHAPCharacteristicDebugDescription_SwingMode,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 1,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleVaneHorizSwingModeRead,
                  .handleWrite = HandleVaneHorizSwingModeWrite}};

const HAPBoolCharacteristic VaneHorizStatusActiveCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_VaneHorizStatusActive,
    .characteristicType = &kHAPCharacteristicType_StatusActive,
    .debugDescription = kHAPCharacteristicDebugDescription_StatusActive,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleStatusActiveRead, .handleWrite = NULL}};

HAPService VaneHorizService = {
    .iid = kIID_VaneHoriz,
    .serviceType = &kHAPServiceType_Slat,
    .debugDescription = kHAPServiceDebugDescription_Slat,
    .name = NULL,  // Set from config.
    .properties = {.primaryService = false,
                   .hidden = false,
                   .ble = {.supportsConfiguration = false}},
    .linkedServices = NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &VaneHorizServiceSignatureCharacteristic, &VaneHorizNameCharacteristic,
        &VaneHorizCurrentSateCharacteristic, &VaneHorizTypeCharacteristic,
        &VaneHorizCurrentTiltAngleCharacteristic,
        &VaneHorizTargetTiltAngleCharacteristic,
        &VaneHorizSwingModeCharacteristic, &VaneHorizStatusActiveCharacteristic,
        NULL}};

// Fan service

static const HAPDataCharacteristic FanServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_FanServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

// static const HAPStringCharacteristic FanNameCharacteristic = {
//     .format = kHAPCharacteristicFormat_String,
//     .iid = kIID_FanName,
//     .characteristicType = &kHAPCharacteristicType_Name,
//     .debugDescription = kHAPCharacteristicDebugDescription_Name,
//     .manufacturerDescription = NULL,
//     .properties = {.readable = true,
//                    .writable = false,
//                    .supportsEventNotification = false,
//                    .hidden = false,
//                    .requiresTimedWrite = false,
//                    .supportsAuthorizationData = false,
//                    .ip = {.controlPoint = false,
//                           .supportsWriteResponse = false},
//                    .ble = {.supportsBroadcastNotification = false,
//                            .supportsDisconnectedNotification = false,
//                            .readableWithoutSecurity = false,
//                            .writableWithoutSecurity = false}},
//     .constraints = {.maxLength = 64},
//     .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

const HAPUInt8Characteristic FanActiveCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_FanActive,
    .characteristicType = &kHAPCharacteristicType_Active,
    .debugDescription = kHAPCharacteristicDebugDescription_Active,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 1,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleFanActiveRead,
                  .handleWrite = HandleFanActiveWrite}};

const HAPUInt8Characteristic FanCurrentSateCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_FanCurrentState,
    .characteristicType = &kHAPCharacteristicType_CurrentFanState,
    .debugDescription = kHAPCharacteristicDebugDescription_CurrentFanState,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 2,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleFanCurrentStateRead,
                  .handleWrite = NULL}};

const HAPUInt8Characteristic FanTargetSateCharacteristic = {
    .format = kHAPCharacteristicFormat_UInt8,
    .iid = kIID_FanTargetState,
    .characteristicType = &kHAPCharacteristicType_TargetFanState,
    .debugDescription = kHAPCharacteristicDebugDescription_TargetFanState,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_None,
    .constraints = {.minimumValue = 0,
                    .maximumValue = 1,
                    .stepValue = 1,
                    .validValues = NULL,
                    .validValuesRanges = NULL},
    .callbacks = {.handleRead = HandleFanTargetStateRead,
                  .handleWrite = HandleFanTargetStateWrite}};

const HAPFloatCharacteristic FanRotationSpeedCharacteristic = {
    .format = kHAPCharacteristicFormat_Float,
    .iid = kIID_FanRotationSpeed,
    .characteristicType = &kHAPCharacteristicType_RotationSpeed,
    .debugDescription = kHAPCharacteristicDebugDescription_RotationSpeed,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .units = kHAPCharacteristicUnits_Percentage,
    .constraints = {.minimumValue = 0.0,
                    .maximumValue = 100.0,
                    .stepValue = 25.0},
    .callbacks = {.handleRead = HandleFanRotationSpeedRead,
                  .handleWrite = HandleFanRotationSpeedWrite}};

const HAPBoolCharacteristic FanStatusActiveCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_FanStatusActive,
    .characteristicType = &kHAPCharacteristicType_StatusActive,
    .debugDescription = kHAPCharacteristicDebugDescription_StatusActive,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleStatusActiveRead, .handleWrite = NULL}};

HAPService FanService = {
    .iid = kIID_Fan,
    .serviceType = &kHAPServiceType_Fan,
    .debugDescription = kHAPServiceDebugDescription_Fan,
    .name = NULL,  // Set from config.
    .properties = {.primaryService = false,
                   .hidden = false,
                   .ble = {.supportsConfiguration = false}},
    .linkedServices = NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &FanServiceSignatureCharacteristic,
        //&FanNameCharacteristic,
        &FanActiveCharacteristic, &FanCurrentSateCharacteristic,
        &FanTargetSateCharacteristic, &FanRotationSpeedCharacteristic,
        &FanStatusActiveCharacteristic, NULL}};

// ModeFan service

static const HAPDataCharacteristic ModeFanServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_ModeFanServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

static const HAPStringCharacteristic ModeFanNameCharacteristic = {
    .format = kHAPCharacteristicFormat_String,
    .iid = kIID_ModeFanName,
    .characteristicType = &kHAPCharacteristicType_Name,
    .debugDescription = kHAPCharacteristicDebugDescription_Name,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 64},
    .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

const HAPBoolCharacteristic ModeFanOnCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_ModeFanOn,
    .characteristicType = &kHAPCharacteristicType_On,
    .debugDescription = kHAPCharacteristicDebugDescription_On,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleModeFanOnRead,
                  .handleWrite = HandleModeFanOnWrite}};

const HAPBoolCharacteristic ModeFanStatusActiveCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_ModeFanStatusActive,
    .characteristicType = &kHAPCharacteristicType_StatusActive,
    .debugDescription = kHAPCharacteristicDebugDescription_StatusActive,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleStatusActiveRead, .handleWrite = NULL}};

HAPService ModeFanService = {
    .iid = kIID_ModeFan,
    .serviceType = &kHAPServiceType_Switch,
    .debugDescription = kHAPServiceDebugDescription_Switch,
    .name = NULL,
    .properties = {.primaryService = false,
                   .hidden = false,
                   .ble = {.supportsConfiguration = false}},
    .linkedServices = NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &ModeFanServiceSignatureCharacteristic, &ModeFanNameCharacteristic,
        &ModeFanOnCharacteristic, &ModeFanStatusActiveCharacteristic, NULL}};

// ModeDry service

static const HAPDataCharacteristic ModeDryServiceSignatureCharacteristic = {
    .format = kHAPCharacteristicFormat_Data,
    .iid = kIID_ModeDryServiceSignature,
    .characteristicType = &kHAPCharacteristicType_ServiceSignature,
    .debugDescription = kHAPCharacteristicDebugDescription_ServiceSignature,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = true},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 2097152},
    .callbacks = {.handleRead = HAPHandleServiceSignatureRead,
                  .handleWrite = NULL}};

static const HAPStringCharacteristic ModeDryNameCharacteristic = {
    .format = kHAPCharacteristicFormat_String,
    .iid = kIID_ModeDryName,
    .characteristicType = &kHAPCharacteristicType_Name,
    .debugDescription = kHAPCharacteristicDebugDescription_Name,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = false,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = false,
                           .supportsDisconnectedNotification = false,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .constraints = {.maxLength = 64},
    .callbacks = {.handleRead = HAPHandleNameRead, .handleWrite = NULL}};

const HAPBoolCharacteristic ModeDryOnCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_ModeDryOn,
    .characteristicType = &kHAPCharacteristicType_On,
    .debugDescription = kHAPCharacteristicDebugDescription_On,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = true,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleModeDryOnRead,
                  .handleWrite = HandleModeDryOnWrite}};

const HAPBoolCharacteristic ModeDryStatusActiveCharacteristic = {
    .format = kHAPCharacteristicFormat_Bool,
    .iid = kIID_ModeDryStatusActive,
    .characteristicType = &kHAPCharacteristicType_StatusActive,
    .debugDescription = kHAPCharacteristicDebugDescription_StatusActive,
    .manufacturerDescription = NULL,
    .properties = {.readable = true,
                   .writable = false,
                   .supportsEventNotification = true,
                   .hidden = false,
                   .requiresTimedWrite = false,
                   .supportsAuthorizationData = false,
                   .ip = {.controlPoint = false,
                          .supportsWriteResponse = false},
                   .ble = {.supportsBroadcastNotification = true,
                           .supportsDisconnectedNotification = true,
                           .readableWithoutSecurity = false,
                           .writableWithoutSecurity = false}},
    .callbacks = {.handleRead = HandleStatusActiveRead, .handleWrite = NULL}};

HAPService ModeDryService = {
    .iid = kIID_ModeDry,
    .serviceType = &kHAPServiceType_Switch,
    .debugDescription = kHAPServiceDebugDescription_Switch,
    .name = NULL,
    .properties = {.primaryService = false,
                   .hidden = false,
                   .ble = {.supportsConfiguration = false}},
    .linkedServices = NULL,
    .characteristics = (const HAPCharacteristic *const[]){
        &ModeDryServiceSignatureCharacteristic, &ModeDryNameCharacteristic,
        &ModeDryOnCharacteristic, &ModeDryStatusActiveCharacteristic, NULL}};

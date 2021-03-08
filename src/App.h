// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef APP_H
#define APP_H

#include "common/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Identify routine. Used to locate the accessory.
 */
HAP_RESULT_USE_CHECK
HAPError IdentifyAccessory(HAPAccessoryServerRef *server,
                           const HAPAccessoryIdentifyRequest *request,
                           void *_Nullable context);

/**
 * Handle read request to the 'On' characteristic of the Light Bulb service.
 */

HAP_RESULT_USE_CHECK
HAPError HandleThermostatCurrentTempRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPFloatCharacteristicReadRequest *request HAP_UNUSED, float *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetTempRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPFloatCharacteristicReadRequest *request HAP_UNUSED, float *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetTempWrite(
    HAPAccessoryServerRef *server,
    const HAPFloatCharacteristicWriteRequest *request, float value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleThermostatCurrentHCstateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetHCstateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetHCstateWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTemperatureDisplayUnitsRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTemperatureDisplayUnitsWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleStatusActiveRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPBoolCharacteristicReadRequest *request HAP_UNUSED, bool *value,
    void *_Nullable context HAP_UNUSED);

// Vane vert

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertCurrentStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertTypeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertCurrentTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertTargetTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertTargetTiltAngleWrite(
    HAPAccessoryServerRef *server,
    const HAPIntCharacteristicWriteRequest *request, int32_t value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertSwingModeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertSwingModeWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED);

// Vane Horiz

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizCurrentStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizTypeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizCurrentTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizTargetTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizTargetTiltAngleWrite(
    HAPAccessoryServerRef *server,
    const HAPIntCharacteristicWriteRequest *request, int32_t value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizSwingModeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizSwingModeWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED);

// Fan

HAP_RESULT_USE_CHECK
HAPError HandleFanActiveRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleFanActiveWrite(HAPAccessoryServerRef *server,
                              const HAPUInt8CharacteristicWriteRequest *request,
                              uint8_t value,
                              void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleFanCurrentStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleFanTargetStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleFanTargetStateWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleFanRotationSpeedRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPFloatCharacteristicReadRequest *request HAP_UNUSED, float *value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleFanRotationSpeedWrite(
    HAPAccessoryServerRef *server,
    const HAPFloatCharacteristicWriteRequest *request, float value,
    void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleModeFanOnRead(HAPAccessoryServerRef *server HAP_UNUSED,
                             const HAPBoolCharacteristicReadRequest *request
                                 HAP_UNUSED,
                             bool *value, void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleModeFanOnWrite(HAPAccessoryServerRef *server,
                              const HAPBoolCharacteristicWriteRequest *request,
                              bool value, void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleModeDryOnRead(HAPAccessoryServerRef *server HAP_UNUSED,
                             const HAPBoolCharacteristicReadRequest *request
                                 HAP_UNUSED,
                             bool *value, void *_Nullable context HAP_UNUSED);

HAP_RESULT_USE_CHECK
HAPError HandleModeDryOnWrite(HAPAccessoryServerRef *server,
                              const HAPBoolCharacteristicWriteRequest *request,
                              bool value, void *_Nullable context HAP_UNUSED);

/**
 * Initialize the application.
 */
void AppCreate(HAPAccessoryServerRef *server,
               HAPPlatformKeyValueStoreRef keyValueStore);

/**
 * Deinitialize the application.
 */
void AppRelease(void);

/**
 * Start the accessory server for the app.
 */
void AppAccessoryServerStart(void);

/**
 * Handle the updated state of the Accessory Server.
 */
void AccessoryServerHandleUpdatedState(HAPAccessoryServerRef *server,
                                       void *_Nullable context);

void AccessoryServerHandleSessionAccept(HAPAccessoryServerRef *server,
                                        HAPSessionRef *session,
                                        void *_Nullable context);

void AccessoryServerHandleSessionInvalidate(HAPAccessoryServerRef *server,
                                            HAPSessionRef *session,
                                            void *_Nullable context);

/**
 * Restore platform specific factory settings.
 */
void RestorePlatformFactorySettings(void);

/**
 * Returns pointer to accessory information
 */
HAPAccessory *AppGetAccessoryInfo();

// LED
#if CS_PLATFORM == CS_P_ESP32
#define LED_ON true
#define LED_OFF false
#endif
#if CS_PLATFORM == CS_P_ESP8266
#define LED_ON false
#define LED_OFF true
#endif

// RPC
void wifi_rpc_start(void);

void mel_cb(int ev, void *ev_data, void *arg);

void mgos_hap_reset(void *arg);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif

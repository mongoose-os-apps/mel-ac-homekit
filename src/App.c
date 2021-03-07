// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "App.h"

#include "DB.h"
#include "mgos.h"
#include "mgos_hap.h"
#include "mgos_mel_ac.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool requestedFactoryReset = false;
bool clearPairings = false;

/**
 * Domain used in the key value store for application data.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreDomain_Configuration \
  ((HAPPlatformKeyValueStoreDomain) 0x00)

/**
 * Key used in the key value store to store the configuration state.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreKey_Configuration_State \
  ((HAPPlatformKeyValueStoreDomain) 0x00)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Global accessory configuration.
 */
typedef struct {
  struct {
    uint8_t ThermostatTemperatureDisplayUnits;
  } state;
  HAPAccessoryServerRef *server;
  HAPPlatformKeyValueStoreRef keyValueStore;
} AccessoryConfiguration;

static AccessoryConfiguration accessoryConfiguration;
//----------------------------------------------------------------------------------------------------------------------

/**
 * Load the accessory state from persistent memory.
 */
static void LoadAccessoryState(void) {
  HAPPrecondition(accessoryConfiguration.keyValueStore);

  HAPError err;

  // Load persistent state if available
  bool found;
  size_t numBytes;

  err = HAPPlatformKeyValueStoreGet(
      accessoryConfiguration.keyValueStore,
      kAppKeyValueStoreDomain_Configuration,
      kAppKeyValueStoreKey_Configuration_State, &accessoryConfiguration.state,
      sizeof accessoryConfiguration.state, &numBytes, &found);

  if (err) {
    HAPAssert(err == kHAPError_Unknown);
    HAPFatalError();
  }
  if (!found || numBytes != sizeof accessoryConfiguration.state) {
    if (found) {
      HAPLogError(&kHAPLog_Default,
                  "Unexpected app state found in key-value store. Resetting to "
                  "default.");
    }
    HAPRawBufferZero(&accessoryConfiguration.state,
                     sizeof accessoryConfiguration.state);
    accessoryConfiguration.state.ThermostatTemperatureDisplayUnits =
        kHAPCharacteristicValue_TemperatureDisplayUnits_Celsius;
  }
}

/**
 * Save the accessory state to persistent memory.
 */
static void SaveAccessoryState(void) {
  HAPPrecondition(accessoryConfiguration.keyValueStore);

  HAPError err;
  err = HAPPlatformKeyValueStoreSet(accessoryConfiguration.keyValueStore,
                                    kAppKeyValueStoreDomain_Configuration,
                                    kAppKeyValueStoreKey_Configuration_State,
                                    &accessoryConfiguration.state,
                                    sizeof accessoryConfiguration.state);
  if (err) {
    HAPAssert(err == kHAPError_Unknown);
    HAPFatalError();
  }
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * HomeKit accessory that provides the Light Bulb service.
 *
 * Note: Not constant to enable BCT Manual Name Change.
 */
static HAPAccessory accessory = {
    .aid = 1,
    .category = kHAPAccessoryCategory_AirConditioners,
    .name = CS_STRINGIFY_MACRO(HAP_PRODUCT_NAME),
    .manufacturer = CS_STRINGIFY_MACRO(HAP_PRODUCT_VENDOR),
    .model = CS_STRINGIFY_MACRO(HAP_PRODUCT_MODEL),
    .serialNumber = NULL,     // Set from config.
    .firmwareVersion = NULL,  // Set from build_id.
    .hardwareVersion = CS_STRINGIFY_MACRO(HAP_PRODUCT_HW_REV),
    .services =
        (const HAPService *const[]){
            &mgos_hap_accessory_information_service,
            &mgos_hap_protocol_information_service, &mgos_hap_pairing_service,
            &ThermostatService, &FanService, &VaneHorizService,
            &VaneVertService, &ModeFanService, &ModeDryService, NULL},
    .callbacks = {.identify = IdentifyAccessory}};

//----------------------------------------------------------------------------------------------------------------------

void AccessoryNotification(const HAPService *service,
                           const HAPCharacteristic *characteristic) {
  HAPLogInfo(&kHAPLog_Default, "Accessory Notification");

  HAPAccessoryServerRaiseEvent(accessoryConfiguration.server, characteristic,
                               service, &accessory);
}

static void identify_timer_cb(void *arg) {
  mgos_gpio_blink(mgos_sys_config_get_pins_led(), 0, 0);
  mgos_gpio_write(mgos_sys_config_get_pins_led(), LED_OFF);
  (void) arg;
}

HAP_RESULT_USE_CHECK
HAPError IdentifyAccessory(HAPAccessoryServerRef *server HAP_UNUSED,
                           const HAPAccessoryIdentifyRequest *request
                               HAP_UNUSED,
                           void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s", __func__);
  mgos_gpio_blink(mgos_sys_config_get_pins_led(), 50, 100);
  mgos_set_timer(1000, 0, identify_timer_cb, NULL);
  return kHAPError_None;
}

float c2f(float c) {
  return c * 9 / 5 + 32;
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatCurrentTempRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPFloatCharacteristicReadRequest *request HAP_UNUSED, float *value,
    void *_Nullable context HAP_UNUSED) {
  *value = accessoryConfiguration.state.ThermostatTemperatureDisplayUnits ==
                   kHAPCharacteristicValue_TemperatureDisplayUnits_Celsius
               ? mgos_mel_ac_get_room_temperature()
               : c2f(mgos_mel_ac_get_room_temperature());

  *value = *value > request->characteristic->constraints.maximumValue
               ? request->characteristic->constraints.maximumValue
               : *value;
  *value = *value < request->characteristic->constraints.minimumValue
               ? request->characteristic->constraints.minimumValue
               : *value;
  HAPLogInfo(&kHAPLog_Default, "%s: %.1f", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetTempRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPFloatCharacteristicReadRequest *request HAP_UNUSED, float *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_setpoint();
  *value = *value > request->characteristic->constraints.maximumValue
               ? request->characteristic->constraints.maximumValue
               : *value;
  *value = *value < request->characteristic->constraints.minimumValue
               ? request->characteristic->constraints.minimumValue
               : *value;
  HAPLogInfo(&kHAPLog_Default, "%s: %.1f", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetTempWrite(
    HAPAccessoryServerRef *server,
    const HAPFloatCharacteristicWriteRequest *request, float value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %.1f", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON)
    mgos_mel_ac_set_setpoint(value);

  AccessoryNotification(request->service, request->characteristic);

  return kHAPError_None;
}

static uint8_t handleThermostatCurrentState() {
  if ((mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_OFF) ||
      (!mgos_mel_ac_get_operating()))
    return kHAPCharacteristicValue_CurrentHeatingCoolingState_Off;

  float currentTemp = mgos_mel_ac_get_room_temperature();
  float targetTemp = mgos_mel_ac_get_setpoint();
  switch (mgos_mel_ac_get_mode()) {
    case MGOS_MEL_AC_PARAM_MODE_COOL:
      return kHAPCharacteristicValue_CurrentHeatingCoolingState_Cool;
    case MGOS_MEL_AC_PARAM_MODE_HEAT:
      return kHAPCharacteristicValue_CurrentHeatingCoolingState_Heat;
    case MGOS_MEL_AC_PARAM_MODE_AUTO:
      if (currentTemp > targetTemp)
        return kHAPCharacteristicValue_CurrentHeatingCoolingState_Cool;
      else if (currentTemp < targetTemp)
        return kHAPCharacteristicValue_CurrentHeatingCoolingState_Heat;
      else 
        return kHAPCharacteristicValue_CurrentHeatingCoolingState_Off;
      // all other cases = off
    case MGOS_MEL_AC_PARAM_MODE_FAN:
    case MGOS_MEL_AC_PARAM_MODE_DRY:
    default:
      return kHAPCharacteristicValue_CurrentHeatingCoolingState_Off;
  }
}

static uint8_t handleThermostatTargetState() {
  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_OFF)
    return kHAPCharacteristicValue_TargetHeatingCoolingState_Off;

  switch (mgos_mel_ac_get_mode()) {
    case MGOS_MEL_AC_PARAM_MODE_AUTO:
      return kHAPCharacteristicValue_TargetHeatingCoolingState_Auto;
    case MGOS_MEL_AC_PARAM_MODE_COOL:
      return kHAPCharacteristicValue_TargetHeatingCoolingState_Cool;
    case MGOS_MEL_AC_PARAM_MODE_HEAT:
      return kHAPCharacteristicValue_TargetHeatingCoolingState_Heat;
    case MGOS_MEL_AC_PARAM_MODE_FAN:
    case MGOS_MEL_AC_PARAM_MODE_DRY:
    default:
      return kHAPCharacteristicValue_TargetHeatingCoolingState_Off;
  }
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatCurrentHCstateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = handleThermostatCurrentState();
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetHCstateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = handleThermostatTargetState();
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

static float handleFan() {
  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_OFF) return 0;
  switch (mgos_mel_ac_get_fan()) {
    case MGOS_MEL_AC_PARAM_FAN_AUTO:
      return 100;
    case MGOS_MEL_AC_PARAM_FAN_QUIET:
      return 0;
    case MGOS_MEL_AC_PARAM_FAN_LOW:
      return 25;
    case MGOS_MEL_AC_PARAM_FAN_MED:
      return 50;
    case MGOS_MEL_AC_PARAM_FAN_HIGH:
      return 75;
    case MGOS_MEL_AC_PARAM_FAN_TURBO:
      return 100;
    default:
      return 0;
  }
}

static void handleFanService() {
  //   AccessoryNotification(&FanService, &FanStatusActiveCharacteristic);
  AccessoryNotification(&FanService, &FanActiveCharacteristic);
  AccessoryNotification(&FanService, &FanCurrentSateCharacteristic);
  AccessoryNotification(&FanService, &FanTargetSateCharacteristic);
  AccessoryNotification(&FanService, &FanRotationSpeedCharacteristic);
}

static void handleModeFanService() {
  //   AccessoryNotification(&ModeFanService,
  //   &ModeFanStatusActiveCharacteristic);
  AccessoryNotification(&ModeFanService, &ModeFanOnCharacteristic);
}

static void handleModeDryService() {
  //   AccessoryNotification(&ModeDryService,
  //   &ModeDryStatusActiveCharacteristic);
  AccessoryNotification(&ModeDryService, &ModeDryOnCharacteristic);
}

static void handleThermostatService() {
  //   AccessoryNotification(&ThermostatService,
  //                         &ThermostatStatusActiveCharacteristic);
  AccessoryNotification(&ThermostatService,
                        &ThermostatCurrentHCstateCharacteristic);
  AccessoryNotification(&ThermostatService,
                        &ThermostatTargetHCstateCharacteristic);
  AccessoryNotification(&ThermostatService,
                        &ThermostatTargetTempCharacteristic);
}

static void handleThermostatServiceTargetState() {
  //   AccessoryNotification(&ThermostatService,
  //                         &ThermostatStatusActiveCharacteristic);
  AccessoryNotification(&ThermostatService,
                        &ThermostatCurrentHCstateCharacteristic);
  AccessoryNotification(&ThermostatService,
                        &ThermostatTargetTempCharacteristic);
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTargetHCstateWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  enum mgos_mel_ac_param_mode mode = mgos_mel_ac_get_mode();

  mgos_mel_ac_set_power(
      value == kHAPCharacteristicValue_TargetHeatingCoolingState_Off
          ? ((mode == MGOS_MEL_AC_PARAM_MODE_DRY) ||
             (mode == MGOS_MEL_AC_PARAM_MODE_FAN))
                ? MGOS_MEL_AC_PARAM_POWER_ON
                : MGOS_MEL_AC_PARAM_POWER_OFF
          : MGOS_MEL_AC_PARAM_POWER_ON);

  switch (value) {
    case kHAPCharacteristicValue_TargetHeatingCoolingState_Auto:
      mode = MGOS_MEL_AC_PARAM_MODE_AUTO;
      break;
    case kHAPCharacteristicValue_TargetHeatingCoolingState_Cool:
      mode = MGOS_MEL_AC_PARAM_MODE_COOL;
      break;
    case kHAPCharacteristicValue_TargetHeatingCoolingState_Heat:
      mode = MGOS_MEL_AC_PARAM_MODE_HEAT;
      break;
  }
  mgos_mel_ac_set_mode(mode);

  handleFanService();
  handleThermostatServiceTargetState();  // do not update HCtarget state,
                                         // because we just set it from HAP
  handleModeFanService();
  handleModeDryService();

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTemperatureDisplayUnitsRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = accessoryConfiguration.state.ThermostatTemperatureDisplayUnits;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleThermostatTemperatureDisplayUnitsWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (accessoryConfiguration.state.ThermostatTemperatureDisplayUnits != value) {
    accessoryConfiguration.state.ThermostatTemperatureDisplayUnits = value;

    SaveAccessoryState();

    AccessoryNotification(request->service, request->characteristic);
    AccessoryNotification(request->service,
                          &ThermostatCurrentTempCharacteristic);
  }

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleStatusActiveRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPBoolCharacteristicReadRequest *request HAP_UNUSED, bool *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_connected();
  HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, *value ? "true" : "false");

  return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

void AppCreate(HAPAccessoryServerRef *server,
               HAPPlatformKeyValueStoreRef keyValueStore) {
  HAPPrecondition(server);
  HAPPrecondition(keyValueStore);

  HAPLogInfo(&kHAPLog_Default, "%s", __func__);

  HAPRawBufferZero(&accessoryConfiguration, sizeof accessoryConfiguration);
  accessoryConfiguration.server = server;
  accessoryConfiguration.keyValueStore = keyValueStore;
  LoadAccessoryState();
}

void AppRelease(void) {
}

void AppAccessoryServerStart(void) {
  HAPAccessoryServerStart(accessoryConfiguration.server, &accessory);
}

//-------------------------------------------------------------------------------------------------------

// VaneVert

static int32_t handleVaneVert() {
  switch (mgos_mel_ac_get_vane_vert()) {
    case MGOS_MEL_AC_PARAM_VANE_VERT_AUTO:
    case MGOS_MEL_AC_PARAM_VANE_VERT_LEFTRIGHT:
      return 0;
    case MGOS_MEL_AC_PARAM_VANE_VERT_LEFTEST:
      return -90;
    case MGOS_MEL_AC_PARAM_VANE_VERT_LEFT:
      return -45;
    case MGOS_MEL_AC_PARAM_VANE_VERT_CENTER:
      return 0;
    case MGOS_MEL_AC_PARAM_VANE_VERT_RIGHT:
      return 45;
    case MGOS_MEL_AC_PARAM_VANE_VERT_RIGHTEST:
      return 90;
    default:
      return 0;
  }
}

static void handleVaneVertService() {
  //   AccessoryNotification(&VaneVertService,
  //   &VaneVertStatusActiveCharacteristic);
  AccessoryNotification(&VaneVertService, &VaneVertSwingModeCharacteristic);
  AccessoryNotification(&VaneVertService, &VaneVertCurrentSateCharacteristic);
  AccessoryNotification(&VaneVertService,
                        &VaneVertCurrentTiltAngleCharacteristic);
  AccessoryNotification(&VaneVertService,
                        &VaneVertTargetTiltAngleCharacteristic);
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertCurrentStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_vane_vert() == MGOS_MEL_AC_PARAM_VANE_VERT_SWING
               ? kHAPCharacteristicValue_CurrentSlatState_Swinging
               : kHAPCharacteristicValue_CurrentSlatState_Fixed;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertTypeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = kHAPCharacteristicValue_SlatType_Vertical;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertCurrentTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = handleVaneVert();
  HAPLogInfo(&kHAPLog_Default, "%s: %ld", __func__, (long int) *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertTargetTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = handleVaneVert();
  HAPLogInfo(&kHAPLog_Default, "%s: %ld", __func__, (long int) *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertTargetTiltAngleWrite(
    HAPAccessoryServerRef *server,
    const HAPIntCharacteristicWriteRequest *request, int32_t value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %ld", __func__, (long int) value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON) {
    enum mgos_mel_ac_param_vane_vert vane_vert;
    switch (value) {
      case -90:
        vane_vert = MGOS_MEL_AC_PARAM_VANE_VERT_LEFTEST;
        break;
      case -45:
        vane_vert = MGOS_MEL_AC_PARAM_VANE_VERT_LEFT;
        break;
      case 0:
        vane_vert = MGOS_MEL_AC_PARAM_VANE_VERT_CENTER;
        break;
      case 45:
        vane_vert = MGOS_MEL_AC_PARAM_VANE_VERT_RIGHT;
        break;
      case 90:
        vane_vert = MGOS_MEL_AC_PARAM_VANE_VERT_RIGHTEST;
        break;
      default:
        vane_vert = MGOS_MEL_AC_PARAM_VANE_VERT_CENTER;
        break;
    }
    mgos_mel_ac_set_vane_vert(vane_vert);
  }

  handleVaneVertService();

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertSwingModeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_vane_vert() == MGOS_MEL_AC_PARAM_VANE_VERT_SWING
               ? kHAPCharacteristicValue_SwingMode_Enabled
               : kHAPCharacteristicValue_SwingMode_Disabled;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneVertSwingModeWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON)
    mgos_mel_ac_set_vane_vert(value == kHAPCharacteristicValue_SwingMode_Enabled
                                  ? MGOS_MEL_AC_PARAM_VANE_VERT_SWING
                                  : MGOS_MEL_AC_PARAM_VANE_VERT_AUTO);

  handleVaneVertService();

  return kHAPError_None;
}

// VaneHoriz

static int32_t handleVaneHoriz() {
  switch (mgos_mel_ac_get_vane_horiz()) {
    case MGOS_MEL_AC_PARAM_VANE_HORIZ_AUTO:
      return 0;
    case MGOS_MEL_AC_PARAM_VANE_HORIZ_1:
      return -90;
    case MGOS_MEL_AC_PARAM_VANE_HORIZ_2:
      return -45;
    case MGOS_MEL_AC_PARAM_VANE_HORIZ_3:
      return 0;
    case MGOS_MEL_AC_PARAM_VANE_HORIZ_4:
      return 45;
    case MGOS_MEL_AC_PARAM_VANE_HORIZ_5:
      return 90;
    default:
      return 0;
  }
}

static void handleVaneHorizService() {
  //   AccessoryNotification(&VaneHorizService,
  //                         &VaneHorizStatusActiveCharacteristic);
  AccessoryNotification(&VaneHorizService, &VaneHorizSwingModeCharacteristic);
  AccessoryNotification(&VaneHorizService, &VaneHorizCurrentSateCharacteristic);
  AccessoryNotification(&VaneHorizService,
                        &VaneHorizCurrentTiltAngleCharacteristic);
  AccessoryNotification(&VaneHorizService,
                        &VaneHorizTargetTiltAngleCharacteristic);
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizCurrentStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_vane_horiz() == MGOS_MEL_AC_PARAM_VANE_HORIZ_SWING
               ? kHAPCharacteristicValue_CurrentSlatState_Swinging
               : kHAPCharacteristicValue_CurrentSlatState_Fixed;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizTypeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = kHAPCharacteristicValue_SlatType_Horizontal;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizCurrentTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = handleVaneHoriz();
  HAPLogInfo(&kHAPLog_Default, "%s: %ld", __func__, (long int) *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizTargetTiltAngleRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPIntCharacteristicReadRequest *request HAP_UNUSED, int32_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = handleVaneHoriz();
  HAPLogInfo(&kHAPLog_Default, "%s: %ld", __func__, (long int) *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizTargetTiltAngleWrite(
    HAPAccessoryServerRef *server,
    const HAPIntCharacteristicWriteRequest *request, int32_t value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %ld", __func__, (long int) value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON) {
    enum mgos_mel_ac_param_vane_horiz vane_horiz;

    switch (value) {
      case -90:
        vane_horiz = MGOS_MEL_AC_PARAM_VANE_HORIZ_1;
        break;
      case -45:
        vane_horiz = MGOS_MEL_AC_PARAM_VANE_HORIZ_2;
        break;
      case 0:
        vane_horiz = MGOS_MEL_AC_PARAM_VANE_HORIZ_3;
        break;
      case 45:
        vane_horiz = MGOS_MEL_AC_PARAM_VANE_HORIZ_4;
        break;
      case 90:
        vane_horiz = MGOS_MEL_AC_PARAM_VANE_HORIZ_5;
        break;
      default:
        vane_horiz = MGOS_MEL_AC_PARAM_VANE_HORIZ_AUTO;
        break;
    }
    mgos_mel_ac_set_vane_horiz(vane_horiz);
  }

  handleVaneHorizService();

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizSwingModeRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_vane_horiz() == MGOS_MEL_AC_PARAM_VANE_HORIZ_SWING
               ? kHAPCharacteristicValue_SwingMode_Enabled
               : kHAPCharacteristicValue_SwingMode_Disabled;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleVaneHorizSwingModeWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON)
    mgos_mel_ac_set_vane_horiz(value ==
                                       kHAPCharacteristicValue_SwingMode_Enabled
                                   ? MGOS_MEL_AC_PARAM_VANE_HORIZ_SWING
                                   : MGOS_MEL_AC_PARAM_VANE_HORIZ_AUTO);

  handleVaneHorizService();

  return kHAPError_None;
}

// Fan

HAP_RESULT_USE_CHECK
HAPError HandleFanActiveRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON
               ? kHAPCharacteristicValue_Active_Active
               : kHAPCharacteristicValue_Active_Inactive;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleFanActiveWrite(HAPAccessoryServerRef *server,
                              const HAPUInt8CharacteristicWriteRequest *request,
                              uint8_t value,
                              void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  handleFanService();
  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleFanCurrentStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON
               ? kHAPCharacteristicValue_CurrentFanState_BlowingAir
               : kHAPCharacteristicValue_CurrentFanState_Inactive;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleFanTargetStateRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPUInt8CharacteristicReadRequest *request HAP_UNUSED, uint8_t *value,
    void *_Nullable context HAP_UNUSED) {
  *value = mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_OFF
               ? kHAPCharacteristicValue_TargetFanState_Manual
               : mgos_mel_ac_get_fan() == MGOS_MEL_AC_PARAM_FAN_AUTO
                     ? kHAPCharacteristicValue_TargetFanState_Auto
                     : kHAPCharacteristicValue_TargetFanState_Manual;
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleFanTargetStateWrite(
    HAPAccessoryServerRef *server,
    const HAPUInt8CharacteristicWriteRequest *request, uint8_t value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %d", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON)
    mgos_mel_ac_set_fan(value == kHAPCharacteristicValue_TargetFanState_Auto
                            ? MGOS_MEL_AC_PARAM_FAN_AUTO
                            : MGOS_MEL_AC_PARAM_FAN_MED);

  handleFanService();

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleFanRotationSpeedRead(
    HAPAccessoryServerRef *server HAP_UNUSED,
    const HAPFloatCharacteristicReadRequest *request HAP_UNUSED, float *value,
    void *_Nullable context HAP_UNUSED) {
  *value = handleFan();
  HAPLogInfo(&kHAPLog_Default, "%s: %f", __func__, *value);

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleFanRotationSpeedWrite(
    HAPAccessoryServerRef *server,
    const HAPFloatCharacteristicWriteRequest *request, float value,
    void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %f", __func__, value);

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  if (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON) {
    enum mgos_mel_ac_param_fan fan = mgos_mel_ac_get_fan();
    switch ((uint8_t) value) {
      case 0:
        fan = MGOS_MEL_AC_PARAM_FAN_QUIET;
        break;
      case 25:
        fan = MGOS_MEL_AC_PARAM_FAN_LOW;
        break;
      case 50:
        fan = MGOS_MEL_AC_PARAM_FAN_MED;
        break;
      case 75:
        fan = MGOS_MEL_AC_PARAM_FAN_HIGH;
        break;
      case 100:
        fan = MGOS_MEL_AC_PARAM_FAN_TURBO;
        break;
      default:
        break;
    }
    mgos_mel_ac_set_fan(fan);
  }
  handleFanService();

  return kHAPError_None;
}

// ModeFan

HAP_RESULT_USE_CHECK
HAPError HandleModeFanOnRead(HAPAccessoryServerRef *server HAP_UNUSED,
                             const HAPBoolCharacteristicReadRequest *request
                                 HAP_UNUSED,
                             bool *value, void *_Nullable context HAP_UNUSED) {
  *value = (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON) &&
           (mgos_mel_ac_get_mode() == MGOS_MEL_AC_PARAM_MODE_FAN);
  HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, *value ? "true" : "false");

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleModeFanOnWrite(HAPAccessoryServerRef *server,
                              const HAPBoolCharacteristicWriteRequest *request,
                              bool value, void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, value ? "true" : "false");

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  mgos_mel_ac_set_power(value ? MGOS_MEL_AC_PARAM_POWER_ON
                              : MGOS_MEL_AC_PARAM_POWER_OFF);

  mgos_mel_ac_set_mode(value ? MGOS_MEL_AC_PARAM_MODE_FAN
                             : MGOS_MEL_AC_PARAM_MODE_AUTO);

  handleThermostatService();
  handleFanService();
  handleModeFanService();
  handleModeDryService();

  return kHAPError_None;
}

// ModeDry

HAP_RESULT_USE_CHECK
HAPError HandleModeDryOnRead(HAPAccessoryServerRef *server HAP_UNUSED,
                             const HAPBoolCharacteristicReadRequest *request
                                 HAP_UNUSED,
                             bool *value, void *_Nullable context HAP_UNUSED) {
  *value = (mgos_mel_ac_get_power() == MGOS_MEL_AC_PARAM_POWER_ON) &&
           (mgos_mel_ac_get_mode() == MGOS_MEL_AC_PARAM_MODE_DRY);
  HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, *value ? "true" : "false");

  return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HandleModeDryOnWrite(HAPAccessoryServerRef *server,
                              const HAPBoolCharacteristicWriteRequest *request,
                              bool value, void *_Nullable context HAP_UNUSED) {
  HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, value ? "true" : "false");

  if (!mgos_mel_ac_get_connected()) return kHAPError_InvalidState;

  mgos_mel_ac_set_power(value ? MGOS_MEL_AC_PARAM_POWER_ON
                              : MGOS_MEL_AC_PARAM_POWER_OFF);

  mgos_mel_ac_set_mode(value ? MGOS_MEL_AC_PARAM_MODE_DRY
                             : MGOS_MEL_AC_PARAM_MODE_AUTO);

  handleThermostatService();
  handleFanService();
  handleModeFanService();
  handleModeDryService();

  return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

void AccessoryServerHandleUpdatedState(HAPAccessoryServerRef *server,
                                       void *_Nullable context) {
  HAPPrecondition(server);
  HAPPrecondition(!context);

  switch (HAPAccessoryServerGetState(server)) {
    case kHAPAccessoryServerState_Idle: {
      HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Idle.");
      return;
    }
    case kHAPAccessoryServerState_Running: {
      HAPLogInfo(&kHAPLog_Default,
                 "Accessory Server State did update: Running.");
      return;
    }
    case kHAPAccessoryServerState_Stopping: {
      HAPLogInfo(&kHAPLog_Default,
                 "Accessory Server State did update: Stopping.");
      return;
    }
  }
  HAPFatalError();
}

HAPAccessory *AppGetAccessoryInfo() {
  return &accessory;
}

void AppInitialize(
    HAPAccessoryServerOptions *hapAccessoryServerOptions HAP_UNUSED,
    HAPPlatform *hapPlatform HAP_UNUSED,
    HAPAccessoryServerCallbacks *hapAccessoryServerCallbacks HAP_UNUSED) {
  accessory.firmwareVersion = mgos_sys_ro_vars_get_fw_version();
  accessory.serialNumber = mgos_sys_config_get_device_id();
  static char hostname[13] = "MEL-????";
  mgos_expand_mac_address_placeholders(hostname);
  accessory.name = hostname;
  VaneVertService.name = "Wide vane";
  VaneHorizService.name = "Vane";
  ModeFanService.name = "Fan mode";
  ModeDryService.name = "Dry mode";
}

void AppDeinitialize() {
  /*no-op*/
}

static void handleStatusActive() {
  AccessoryNotification(&ThermostatService,
                        &ThermostatStatusActiveCharacteristic);
  AccessoryNotification(&FanService, &FanStatusActiveCharacteristic);
  AccessoryNotification(&VaneVertService, &VaneVertStatusActiveCharacteristic);
  AccessoryNotification(&VaneHorizService,
                        &VaneHorizStatusActiveCharacteristic);
  AccessoryNotification(&ModeFanService, &ModeFanStatusActiveCharacteristic);
  AccessoryNotification(&ModeDryService, &ModeDryStatusActiveCharacteristic);
}

static void led_off_timer_cb(void *arg) {
  mgos_gpio_write(mgos_sys_config_get_pins_led(), LED_OFF);
}

static void led_on(int msec) {
  mgos_gpio_write(mgos_sys_config_get_pins_led(), LED_ON);
  mgos_set_timer(msec, 0, led_off_timer_cb, NULL);
}

void mel_cb(int ev, void *ev_data, void *arg) {
  switch (ev) {
    case MGOS_MEL_AC_EV_INITIALIZED:
      LOG(LL_INFO, ("MEL init done"));
      break;
    case MGOS_MEL_AC_EV_CONNECTED:
      LOG(LL_INFO, ("connected: %s", *(bool *) ev_data ? "true" : "false"));
      if (!accessoryConfiguration.server) goto hap_not_running;
      handleStatusActive();
      break;
    case MGOS_MEL_AC_EV_CONNECT_ERROR:
      LOG(LL_INFO, ("connect_error: %d", *(uint8_t *) ev_data));
      break;
    case MGOS_MEL_AC_EV_PACKET_WRITE:
      LOG(LL_DEBUG, ("tx: %s", (char *) ev_data));
      break;
    case MGOS_MEL_AC_EV_PACKET_READ:
      LOG(LL_DEBUG, ("rx: %s", (char *) ev_data));
      break;
    case MGOS_MEL_AC_EV_OPERATING_CHANGED:
      LOG(LL_INFO, ("opeating: %s", *(bool *) ev_data ? "true" : "false"));
      if (!accessoryConfiguration.server) goto hap_not_running;

      AccessoryNotification(&ThermostatService,
                            &ThermostatCurrentHCstateCharacteristic);
      break;
    case MGOS_MEL_AC_EV_PARAMS_SET:
      LOG(LL_INFO, ("new params aplied to HVAC"));
      led_on(mgos_sys_config_get_app_blink_ms_update());
      break;
    case MGOS_MEL_AC_EV_PARAMS_NOT_SET:
      LOG(LL_WARN, ("HVAC failed to apply new params"));
      break;
    case MGOS_MEL_AC_EV_PARAMS_CHANGED: {
      led_on(mgos_sys_config_get_app_blink_ms_sync());
      if (!accessoryConfiguration.server) goto hap_not_running;
      // Thermostat
      handleThermostatService();
      // Fan
      handleFanService();
      // VaneVert
      handleVaneVertService();
      // VaneHoriz
      handleVaneHorizService();
      // ModeFan
      handleModeFanService();
      // ModeDry
      handleModeDryService();

    } break;
    case MGOS_MEL_AC_EV_ROOMTEMP_CHANGED: {
      led_on(mgos_sys_config_get_app_blink_ms_room());
      LOG(LL_INFO, ("room_temp: %.1f", *(float *) ev_data));

      if (!accessoryConfiguration.server) goto hap_not_running;

      AccessoryNotification(&ThermostatService,
                            &ThermostatCurrentTempCharacteristic);
    } break;
    case MGOS_MEL_AC_EV_PACKET_READ_ERROR:
      LOG(LL_ERROR, ("error: packet crc"));
      break;
    case MGOS_MEL_AC_EV_TIMER:
      break;
    default:
      LOG(LL_VERBOSE_DEBUG, ("event: %d", ev));
  }
  return;

hap_not_running:
  LOG(LL_WARN, ("HAP server is not running, skipping accessory update"));

  (void) ev_data;
}

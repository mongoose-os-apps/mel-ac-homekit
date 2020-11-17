// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "App.h"
#include "DB.h"
#include "HAP.h"
#include "HAPPlatform+Init.h"
#include "HAPPlatformAccessorySetup+Init.h"
#if BLE
#include "HAPPlatformBLEPeripheralManager+Init.h"
#endif
#include "HAPPlatformKeyValueStore+Init.h"
#if IP
#include "HAPPlatformServiceDiscovery+Init.h"
#include "HAPPlatformTCPStreamManager+Init.h"
#endif

#include "HAP+Internal.h"
#include "mgos.h"
#include "mgos_dns_sd.h"
#include "mgos_hap.h"
#include "mgos_mel_ac.h"

static bool requestedFactoryReset = false;
static bool clearPairings = false;

#define MAX_NUM_SESSIONS 16

#define PREFERRED_ADVERTISING_INTERVAL \
  (HAPBLEAdvertisingIntervalCreateFromMilliseconds(417.5f))

/**
 * Global platform objects.
 * Only tracks objects that will be released in DeinitializePlatform.
 */
static struct {
  HAPPlatformKeyValueStore keyValueStore;
  HAPAccessoryServerOptions hapAccessoryServerOptions;
  HAPPlatform hapPlatform;
  HAPAccessoryServerCallbacks hapAccessoryServerCallbacks;

#if HAVE_NFC
  HAPPlatformAccessorySetupNFC setupNFC;
#endif

#if IP
  HAPPlatformTCPStreamManager tcpStreamManager;
#endif
} platform;

/**
 * HomeKit accessory server that hosts the accessory.
 */
static HAPAccessoryServerRef accessoryServer;

void HandleUpdatedState(HAPAccessoryServerRef *_Nonnull server,
                        void *_Nullable context);

/**
 * Functions provided by App.c for each accessory application.
 */
extern void AppRelease(void);
extern void AppCreate(HAPAccessoryServerRef *server,
                      HAPPlatformKeyValueStoreRef keyValueStore);
extern void AppInitialize(
    HAPAccessoryServerOptions *hapAccessoryServerOptions,
    HAPPlatform *hapPlatform,
    HAPAccessoryServerCallbacks *hapAccessoryServerCallbacks);
extern void AppDeinitialize();
extern void AppAccessoryServerStart(void);
extern void AccessoryServerHandleUpdatedState(HAPAccessoryServerRef *server,
                                              void *_Nullable context);

static void timer_cb(void *arg) {
  static bool s_tick_tock = false;
  LOG(LL_INFO,
      ("%s uptime: %.2lf, RAM: %lu, %lu free", (s_tick_tock ? "Tick" : "Tock"),
       mgos_uptime(), (unsigned long) mgos_get_heap_size(),
       (unsigned long) mgos_get_free_heap_size()));
  s_tick_tock = !s_tick_tock;
  (void) arg;
}

static void adv_timer_cb(void *arg) {
  if (!HAPAccessoryServerIsPaired(HAPNonnull(&accessoryServer))) {
    LOG(LL_DEBUG, ("Advertising accessory"));
    mgos_dns_sd_advertise();
  }
  (void) arg;
}

/**
 * Initialize global platform objects.
 */
static void InitializePlatform() {
  // Key-value store.
  HAPPlatformKeyValueStoreCreate(
      &platform.keyValueStore,
      &(const HAPPlatformKeyValueStoreOptions){.fileName = "kv.json"});
  platform.hapPlatform.keyValueStore = &platform.keyValueStore;

  // Accessory setup manager. Depends on key-value store.
  static HAPPlatformAccessorySetup accessorySetup;
  HAPPlatformAccessorySetupCreate(&accessorySetup,
                                  &(const HAPPlatformAccessorySetupOptions){});
  platform.hapPlatform.accessorySetup = &accessorySetup;

#if IP
  // TCP stream manager.
  HAPPlatformTCPStreamManagerCreate(
      &platform.tcpStreamManager,
      &(const HAPPlatformTCPStreamManagerOptions){
          .port = kHAPNetworkPort_Any,  // Listen on unused port number from the
                                        // ephemeral port range.
          .maxConcurrentTCPStreams = MAX_NUM_SESSIONS});

  // Service discovery.
  static HAPPlatformServiceDiscovery serviceDiscovery;
  HAPPlatformServiceDiscoveryCreate(
      &serviceDiscovery, &(const HAPPlatformServiceDiscoveryOptions){});
  platform.hapPlatform.ip.serviceDiscovery = &serviceDiscovery;
#endif

#if BLE
  // BLE peripheral manager. Depends on key-value store.
  static HAPPlatformBLEPeripheralManagerOptions blePMOptions = {0};
  blePMOptions.keyValueStore = &platform.keyValueStore;

  static HAPPlatformBLEPeripheralManager blePeripheralManager;
  HAPPlatformBLEPeripheralManagerCreate(&blePeripheralManager, &blePMOptions);
  platform.hapPlatform.ble.blePeripheralManager = &blePeripheralManager;
#endif

  // Run loop.
  // HAPPlatformRunLoopCreate(&(const HAPPlatformRunLoopOptions) {
  // .keyValueStore = &platform.keyValueStore });

  platform.hapAccessoryServerOptions.maxPairings =
      kHAPPairingStorage_MinElements;

  platform.hapAccessoryServerCallbacks.handleUpdatedState = HandleUpdatedState;

  mgos_set_timer(1000, MGOS_TIMER_REPEAT, timer_cb, NULL);
}

/**
 * Deinitialize global platform objects.
 */
void DeinitializePlatform() {
#if IP
  // TCP stream manager.
  HAPPlatformTCPStreamManagerRelease(&platform.tcpStreamManager);
#endif

  AppDeinitialize();

  // Run loop.
  // HAPPlatformRunLoopRelease();
}

/**
 * Restore platform specific factory settings.
 */
void RestorePlatformFactorySettings(void) {
}

/**
 * Either simply passes State handling to app, or processes Factory Reset
 */
void HandleUpdatedState(HAPAccessoryServerRef *_Nonnull server,
                        void *_Nullable context) {
  if (HAPAccessoryServerGetState(server) == kHAPAccessoryServerState_Idle &&
      requestedFactoryReset) {
    HAPPrecondition(server);

    HAPError err;

    HAPLogInfo(&kHAPLog_Default, "A factory reset has been requested.");

    // Purge app state.
    err = HAPPlatformKeyValueStorePurgeDomain(
        &platform.keyValueStore, ((HAPPlatformKeyValueStoreDomain) 0x00));
    if (err) {
      HAPAssert(err == kHAPError_Unknown);
      HAPFatalError();
    }

    // Reset HomeKit state.
    err = HAPRestoreFactorySettings(&platform.keyValueStore);
    if (err) {
      HAPAssert(err == kHAPError_Unknown);
      HAPFatalError();
    }

    // Restore platform specific factory settings.
    RestorePlatformFactorySettings();

    // De-initialize App.
    AppRelease();

    requestedFactoryReset = false;

    // Re-initialize App.
    AppCreate(server, &platform.keyValueStore);

    // Restart accessory server.
    AppAccessoryServerStart();
    return;
  } else if (HAPAccessoryServerGetState(server) ==
                 kHAPAccessoryServerState_Idle &&
             clearPairings) {
    HAPError err;
    err = HAPRemoveAllPairings(&platform.keyValueStore);
    if (err) {
      HAPAssert(err == kHAPError_Unknown);
      HAPFatalError();
    }
    AppAccessoryServerStart();
  } else {
    AccessoryServerHandleUpdatedState(server, context);
  }
}

#if IP
static void InitializeIP() {
  // Prepare accessory server storage.
  static HAPIPSession ipSessions[MAX_NUM_SESSIONS];
  static uint8_t ipScratchBuffer[2048];
  static HAPIPAccessoryServerStorage ipAccessoryServerStorage = {
      .sessions = ipSessions,
      .numSessions = HAPArrayCount(ipSessions),
      .scratchBuffer = {.bytes = ipScratchBuffer,
                        .numBytes = sizeof ipScratchBuffer}};

  platform.hapAccessoryServerOptions.ip.transport =
      &kHAPAccessoryServerTransport_IP;
  platform.hapAccessoryServerOptions.ip.accessoryServerStorage =
      &ipAccessoryServerStorage;

  platform.hapPlatform.ip.tcpStreamManager = &platform.tcpStreamManager;
}
#endif

#if BLE
static void InitializeBLE() {
  static HAPBLEGATTTableElementRef gattTableElements[kAttributeCount];
  static HAPBLESessionCacheElementRef
      sessionCacheElements[kHAPBLESessionCache_MinElements];
  static HAPSessionRef session;
  static uint8_t procedureBytes[3072];
  static HAPBLEProcedureRef procedures[1];

  static HAPBLEAccessoryServerStorage bleAccessoryServerStorage = {
      .gattTableElements = gattTableElements,
      .numGATTTableElements = HAPArrayCount(gattTableElements),
      .sessionCacheElements = sessionCacheElements,
      .numSessionCacheElements = HAPArrayCount(sessionCacheElements),
      .session = &session,
      .procedures = procedures,
      .numProcedures = HAPArrayCount(procedures),
      .procedureBuffer = {.bytes = procedureBytes,
                          .numBytes = sizeof procedureBytes}};

  platform.hapAccessoryServerOptions.ble.transport =
      &kHAPAccessoryServerTransport_BLE;
  platform.hapAccessoryServerOptions.ble.accessoryServerStorage =
      &bleAccessoryServerStorage;
  platform.hapAccessoryServerOptions.ble.preferredAdvertisingInterval =
      PREFERRED_ADVERTISING_INTERVAL;
  platform.hapAccessoryServerOptions.ble.preferredNotificationDuration =
      kHAPBLENotification_MinDuration;
}
#endif

enum mgos_app_init_result mgos_app_init(void) {
  /* LED */
  mgos_gpio_set_mode(mgos_sys_config_get_pins_led(), MGOS_GPIO_MODE_OUTPUT);
  mgos_gpio_write(mgos_sys_config_get_pins_led(), LED_OFF);
  /* Captive */
  if (mgos_sys_config_get_wifi_ap_enable()) {
    LOG(LL_WARN, ("Runing captive portal to setup WiFi"));
    return MGOS_APP_INIT_SUCCESS;
  };
  if (!mgos_sys_config_get_mel_ac_enable()) {
    LOG(LL_INFO, ("Updating config..."));
    /* Config */
    mgos_sys_config_set_debug_stdout_uart(-1);
    mgos_sys_config_set_debug_stderr_uart(-1);
    mgos_sys_config_set_mel_ac_enable(true);
    mgos_sys_config_save(&mgos_sys_config, false, NULL);
    mgos_system_restart();  // Its better to restart
  }
  LOG(LL_INFO, ("Starting services..."));
  /* MEL-AC events */
  mgos_event_add_group_handler(MGOS_EVENT_GRP_MEL_AC, mel_cb, NULL);
  /* HAP */
  HAPAssert(HAPGetCompatibilityVersion() == HAP_COMPATIBILITY_VERSION);
  // Initialize global platform objects.
  InitializePlatform();

#if IP
  InitializeIP();
#endif

#if BLE
  InitializeBLE();
#endif

  // Perform Application-specific initalizations such as setting up callbacks
  // and configure any additional unique platform dependencies
  AppInitialize(&platform.hapAccessoryServerOptions, &platform.hapPlatform,
                &platform.hapAccessoryServerCallbacks);

  // Initialize accessory server.
  HAPAccessoryServerCreate(
      &accessoryServer, &platform.hapAccessoryServerOptions,
      &platform.hapPlatform, &platform.hapAccessoryServerCallbacks,
      /* context: */ NULL);

  // Create app object.
  AppCreate(&accessoryServer, &platform.keyValueStore);

  // Start accessory server for App.
  if (mgos_hap_config_valid()) {
    // MDNS lost queries workaround
    mgos_set_timer(2000, MGOS_TIMER_REPEAT, adv_timer_cb, NULL);
    AppAccessoryServerStart();
  } else {
    LOG(LL_INFO, ("=== Accessory is not provisioned"));
  }

  mgos_hap_add_rpc_service(&accessoryServer, AppGetAccessoryInfo());

  return MGOS_APP_INIT_SUCCESS;
}

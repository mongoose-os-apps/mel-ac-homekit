# Mitsubishi Electric Air-Co control for HomeKit

Add your Air Conditioner or the ATW unit to HomeKit smarthome infrastructure. Use `Siri` voice control and all the scripting potential from `Apple`

![](https://github.com/mongoose-os-apps/mel-ac-homekit/blob/master/docs/adding.gif)

## Supported platforms

* `ESP8266`
* `ESP32`

## Dependencies 

* [MEL-AC library](https://github.com/mongoose-os-libs/mel-ac) - the `Mitsubishi Electric` protocol implementation
* [HomeKit ADK library](https://github.com/mongoose-os-libs/homekit-adk) which is port of the official [Apple HomeKit ADK](https://github.com/Apple/HomeKitADK/)
* [WiFi Setup library](https://github.com/d4rkmen/wifi-setup) - Web-based WiFi setup captive portal

## Accessory design

![](https://github.com/mongoose-os-apps/mel-ac-homekit/blob/master/docs/services.gif)

### Identification

Build in LED blinks during the identification

### HomeKit services

* `Thermostat` - used to control `Heat`, `Cool`, `Auto` mode and Power ON / OFF
* `Fan` -  control the fan speed
* `Vane horizontal` - (not visible in Home App yet, use Eve) control up-down vane
* `Vane vertical` - (not visible in Home App yet, use Eve) control left-right vane
* `Fan mode` - turns `Fan only` mode, `Thermosat` is disabled for this
* `Dry mode` - turns `Dry` mode, `Thermosat` is disabled for this

HomeKit `Status Active` reporting the HVAC interface connected / disconnected

`Mitsubishi Electric` protocol limitations:

* HomeKit widget changes apply time up to 2 seconds
* HVAC parameter or room temperature changes sync time up to 8 seconds.

### LED indication

* LED blink on remote params change `app.blink_ms_sync`
* LED blink on new params apply `app.blink_ms_update`
* LED blink on room temp change `app.blink_ms_room` 

## Setup

Using the [Mongoose OS](http://mongoose-os.com) framework:

```
$ mos build
$ mos flash
```

## WiFI settings

Connect WiFi access point name `MEL-XXXX` password `macdrive`, select home network and save credentials

![](https://github.com/mongoose-os-apps/mel-ac-homekit/blob/master/docs/wifi-setup.gif)

## Factory reset

Hold a button for factory reset. This will remove WiFi settings, HAP server status.

Configuration:

```yml
  - ["pins.button", "i", -1, {title: "Button GPIO pin"}]
  - ["pins.button_hold_ms", "i", 5000, {title: "Button hold time for reset"}]
  - ["pins.button_pull_up", "b", true, {title: "Button pull up or down"}]
```

## ToDo

Index page for Web GUI holding the device information and factory reset feature

## Known issues

`ESP32` platform works well with `mos` tool `2.19.1`, but not with newer. The issue is related to UART driver and still under investigation

## Copyrights

 * [d4rkmen](https://github.com/d4rkmen)
 * [Deomid "rojer" Ryabkov](https://github.com/rojer)
 * The HomeKit ADK Contributors
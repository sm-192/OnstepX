// Website plugin

#include "Website.h"
#include "Common.h"
#include "pages/Pages.h"

#if OTA_WEBSERVER == ON
  #include <ElegantOTA.h>
#endif
#if OTA_ARDUINO == ON
  #include <ArduinoOTA.h>
#endif

TaskHandle_t _webSvrTask;
void pollWebSvr(void * parameter) {
  for(;;) {
    #if OTA_ARDUINO == ON
      ArduinoOTA.handle();
    #endif
    www.handleClient();
    #if OTA_WEBSERVER == ON
      ElegantOTA.loop();
    #endif
    state.poll();
  }
}

void Website::init() {
  VLF("MSG: Website Plugin");

  VLF("MSG: Set webpage handlers");
  www.on("/index.htm", handleRoot);
  www.on("/index-ajax-get.txt", indexAjaxGet);
  www.on("/index.txt", indexAjax);

  www.on("/mount.htm", handleMount);
  www.on("/mount-ajax-get.txt", mountAjaxGet);
  www.on("/mount-ajax.txt", mountAjax);
  www.on("/libraryHelp.htm", handleLibraryHelp);

  www.on("/rotator.htm", handleRotator);
  www.on("/rotator-ajax-get.txt", rotatorAjaxGet);
  www.on("/rotator-ajax.txt", rotatorAjax);

  www.on("/focuser.htm", handleFocuser);
  www.on("/focuser-ajax-get.txt", focuserAjaxGet);
  www.on("/focuser-ajax.txt", focuserAjax);

  www.on("/auxiliary.htm", handleAux);
  www.on("/auxiliary-ajax-get.txt", auxAjaxGet);
  www.on("/auxiliary-ajax.txt", auxAjax);

  www.on("/net.htm", handleNetwork);

  www.on("/", handleRoot);
  
  www.onNotFound(handleNotFound);

  #if OTA_WEBSERVER == ON
    const char *otaPassword = OTA_WEBSERVER_PASSWORD;
    if (otaPassword[0] == 0) otaPassword = wifiManager.settings.masterPassword;
    ElegantOTA.setAuth(OTA_WEBSERVER_USERNAME, otaPassword);
    ElegantOTA.begin(&www);
    VLF("MSG: ElegantOTA endpoint ready at /update");
  #endif

  #if OTA_ARDUINO == ON
    const char *espOtaPassword = OTA_ARDUINO_PASSWORD;
    if (espOtaPassword[0] == 0) espOtaPassword = wifiManager.settings.masterPassword;
    ArduinoOTA.setHostname(OTA_ARDUINO_HOSTNAME);
    ArduinoOTA.setPort(OTA_ARDUINO_PORT);
    ArduinoOTA.setPassword(espOtaPassword);
    ArduinoOTA.begin();
    VF("MSG: ArduinoOTA ready at "); V(OTA_ARDUINO_HOSTNAME); VF(":"); VL(OTA_ARDUINO_PORT);
  #endif

  VLF("MSG: Starting port 80 web server");
  www.begin();

  // allow time for the background servers to come up
  delay(2000);

  if (status.onStepFound) {
    status.update();
    delay(100);
  }

  state.init();

  VLF("MSG: Setup, starting web server FreeRTOS task (priority 1)");
  xTaskCreatePinnedToCore(pollWebSvr,"WebSvrTask", 10000, NULL, 1, &_webSvrTask, 0);

  VLF("MSG: Website Plugin ready");
}

Website website;

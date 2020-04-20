String makeMacAddress() {
  
  String mac = "";

  for (int i = 0; i < macAddress.length(); i++) {
    if (macAddress[i] != ':') mac += macAddress[i];
  }
  
  return mac;
}

/* For read and write to EEPROM */
unsigned long crc_update(unsigned long crc, byte data) {
  byte tbl_idx;
  tbl_idx = crc ^ (data >> (0 * 4));
  crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
  tbl_idx = crc ^ (data >> (1 * 4));
  crc = pgm_read_dword_near(crc_table + (tbl_idx & 0x0f)) ^ (crc >> 4);
  return crc;
}


unsigned long crc_byte(byte *b, int len) {
  unsigned long crc = ~0L;
  int i;

  for (i = 0 ; i < len ; i++) {
    crc = crc_update(crc, *b++);
  }
  crc = ~crc;
  return crc;
}

/* Vcc or Battery in volt */
String returnVccStr() {
  String v = "";
  String Vcc;
  int volt;
  int voltInt;
  
  Vcc = "Vcc: ";
  voltInt = ESP.getVcc();
  volt = (voltInt*117+5000)/100;
/*  Serial.printf("voltInt: %d\n", voltInt);*/
  
    v += volt;

    Vcc += v.substring(0, 1);
    Vcc += ',';
    Vcc += v.substring(1, 3);
    Vcc += 'V';

    return Vcc;
    
}

/* Received signal strength indicator in dBm */
String returnRssiStr() {
  String rssi = "WiFi: ";
  rssi += WiFi.RSSI();
  rssi += " dBm";
  return rssi;
}

/* Init PIN */
void initPin() {
  pinMode(HOT_PIN, INPUT_PULLUP);
  pinMode(COLD_PIN, INPUT_PULLUP);
  
}

void startApMsg() {
  Serial.printf("WiFi network Name: %s, Password: %s\n", wmConfig.apSsid, wmConfig.apPassword);
  Serial.print("Go to: "); Serial.print(WiFi.softAPIP()); Serial.println(" please");
}

/* Init external interrupt           */
void initInterrupt() {
  
  attachInterrupt(digitalPinToInterrupt(HOT_PIN), hotInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(COLD_PIN), coldInterrupt, CHANGE);

  hotInt = coldInt = 0;
  hotState = digitalRead(HOT_PIN);
  coldState = digitalRead(COLD_PIN);
}

/* External interrupt for hot water  */
ICACHE_RAM_ATTR void hotInterrupt() {
  if (hotState == LOW) {
    if (digitalRead(HOT_PIN)) {
      /* First interrupt if hotInt == 0  */
      if (hotInt == 0) {    
        hotInt++;
        hotTimeBounce = millis();
      } else os_timer_disarm(&hotTimer);
      os_timer_arm(&hotTimer, TIME_BOUNCE, true); 
    }
  } else {
    if (!digitalRead(HOT_PIN)) {
      /* First interrupt if hotInt == 0  */
      if (hotInt == 0) {    
        hotInt++;
        hotTimeBounce = millis();
      } else os_timer_disarm(&hotTimer);
      os_timer_arm(&hotTimer, TIME_BOUNCE, true); 
    }
  }
}

/* External interrupt for cold water */
ICACHE_RAM_ATTR void coldInterrupt() {
  if (coldState == LOW) {
    if (digitalRead(COLD_PIN)) {
      /* First interrupt if coldInt == 0  */
      if (coldInt == 0) {    
        coldInt++;
        coldTimeBounce = millis();
      } else os_timer_disarm(&coldTimer);
      os_timer_arm(&coldTimer, TIME_BOUNCE, true); 
    }
  } else {
    if (!digitalRead(COLD_PIN)) {
      /* First interrupt if coldInt == 0  */
      if (coldInt == 0) {    
        coldInt++;
        coldTimeBounce = millis();
      } else os_timer_disarm(&coldTimer);
      os_timer_arm(&coldTimer, TIME_BOUNCE, true); 
    }
  }
}

void hotTimerCallback(void *pArg) {
  if (hotState == LOW) {
    if (!digitalRead(HOT_PIN)) {
      os_timer_disarm(&hotTimer);
      hotInt = 0;
      return;
    }
    if (digitalRead(HOT_PIN) && millis() - hotTimeBounce < TIME_BOUNCE) return;
    os_timer_disarm(&hotTimer);
    hotInt = 0;
    counterHotWater++;
    hotState = HIGH;
  } else {
    if (digitalRead(HOT_PIN)) {
      os_timer_disarm(&hotTimer);
      hotInt = 0;
      return;
    }
    if (!digitalRead(HOT_PIN) && millis() - hotTimeBounce < TIME_BOUNCE) return;
    os_timer_disarm(&hotTimer);
    hotInt = 0;
    hotState = LOW;
  }
}

void coldTimerCallback(void *pArg) {
  if (coldState == LOW) {
    if (!digitalRead(COLD_PIN)) {
      os_timer_disarm(&coldTimer);
      coldInt = 0;
      return;
    }
    if (digitalRead(COLD_PIN) && millis() - coldTimeBounce < TIME_BOUNCE) return;
    os_timer_disarm(&coldTimer);
    coldInt = 0;
    counterColdWater++;
    coldState = HIGH;
  } else {
    if (digitalRead(COLD_PIN)) {
      os_timer_disarm(&coldTimer);
      coldInt = 0;
      return;
    }
    if (!digitalRead(COLD_PIN) && millis() - coldTimeBounce < TIME_BOUNCE) return;
    os_timer_disarm(&coldTimer);
    coldInt = 0;
    coldState = LOW;
  }
}

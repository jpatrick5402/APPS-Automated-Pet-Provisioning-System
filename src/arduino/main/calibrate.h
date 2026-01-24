void calibrate(HX711 scale, String var_name) { // run this to get your offset and divider for each scale
  delay(1000);
  if (scale.is_ready()) {
    Serial.printf("Calibrating %s\n\r", var_name);
    scale.set_scale(1);
    scale.set_offset(0);
    Serial.println("Offset... remove any weights from the scale.");
    delay(8000);
    Serial.print("Offset: ");
    long offset = scale.read_average(30);
    Serial.println(offset);
    Serial.println("Place a known weight on the scale...");
    delay(8000);
    Serial.print("Scale: ");
    long known = scale.get_units(30);
    Serial.print(known - offset);
    Serial.println(" / KNOWN WEIGHT = SCALE");
  }
  else {
    Serial.println("HX711 not found.");
  }
  delay(1000);
}

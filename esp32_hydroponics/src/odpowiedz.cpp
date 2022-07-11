  // /* Sekcja obsługująca odpowiedź */
  // const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 100;

  // DynamicJsonDocument jsonBuffer(capacity);
  // // jsonBuffer["method"] = "rpc_call";
  // // JsonObject params = jsonBuffer.createNestedObject("params");
  // // odpowiedź
  // // params["status"] = "ok";
  // // Odpowiedź wygląda tak: {"method": "rpc_call", "params": {"status": "ok"}}

  // // jsonBuffer["method"] = methodName;
  // // JsonObject params = jsonBuffer.createNestedObject("params");
  // // params["enabled"] = 1;
  // jsonBuffer["enabled"] = 1;

  // char response[256];
  // serializeJson(jsonBuffer, response);

  // char response_topic[256];
  // // Skopiowanie const char z adresem kanału odpowiedzi do bufora
  // strcpy(response_topic, rpc_response_topic);
  // // doklejenie do bufora numeru żądania RPC
  // strcat(response_topic, methodName);
  // strcat(response_topic, "/");
  // strcat(response_topic, request_id);
  // mqtt_client.publish(response_topic, response);
  // Serial.printf("Resp topic: %s \n", response_topic);
  // Serial.printf("Resp message: %s \n\n", response);

#include "ComponentManager.h"
#include <ArduinoJson.h>

// JSON input string.
//
// It's better to use a char[] as shown here.
// If you use a const char* or a String, ArduinoJson will
// have to make a copy of the input in the JsonBuffer.
// @param payload
bool ComponentManager::Load(const String& componentsJson)
{
  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  // const size_t bufferSize = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 20;
  DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + 20);

  // Root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.
  JsonObject& root = jsonBuffer.parseObject(componentsJson);

  // Test if parsing succeeds.
  if (!root.success())
  {
    Serial.println("parseObject() failed");
    return false;
  }

  size_t compIndex = 0;
  for (size_t index = 0; index < root["components_count"]; ++index)
  {
    for (size_t propIndex = 0; propIndex < root["data"][root["components"][index]]["count"]; ++propIndex)
    {
      m_components[index][propIndex] = root["data"][root["components"][index]]["properties"][propIndex];
      if (propIndex > MAX_PROPERTY_COUNT * MAX_COMPONENT_COUNT)
      {
        break;
      }
    }
    if (index > MAX_COMPONENT_COUNT)
    {
      break;
    }
  }
  return true;  
}

void ComponentManager::UpdateStates()
{
  for (size_t index = 0; index < MAX_COMPONENT_COUNT; ++index)
  {
    for (size_t propIndex = 0; propIndex < MAX_COMPONENT_COUNT * MAX_PROPERTY_COUNT; ++propIndex)
    {
      digitalWrite(m_relayInputs[index], m_components[index][propIndex] == States::ON ? LOW : HIGH); // turn relay on/ off
      Serial.printf("Component %d is %s\n", index + 1, m_components[index][propIndex] == States::ON ?  "ON" : "OFF");
    }
  }
  delay(1000);    
}



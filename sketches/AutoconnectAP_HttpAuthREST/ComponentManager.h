
/**************************************************************
   ComponentManager is a library for the electrical automation component
   Built by Tajuddin Khandaker https://github.com/tajuddinkhandaker
   Licensed under MIT license
 **************************************************************/

#ifndef ComponentManager_h
#define ComponentManager_h

#include <Arduino.h>
#include "config.h"

enum States { OFF = 0, ON = 1 };
enum Pins { PinD4 = 2, PinD2 = 0 };

class ComponentManager
{
  public:
    ComponentManager() = default;
    bool Load(const String& componentsJson);
    void UpdateStates();
    
  private:
    int m_components[MAX_COMPONENT_COUNT][MAX_PROPERTY_COUNT * MAX_COMPONENT_COUNT];
    int m_relayInputs[MAX_COMPONENT_COUNT] = { Pins::PinD4, Pins::PinD2 };
};

#endif

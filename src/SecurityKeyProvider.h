#pragma once

#include <Preferences.h>

#include "GsdCore/ISecurityKeyProvider.h"

class SecurityKeyProvider : public gsd::ISecurityKeyProvider {
   public:
    Key getKey();
};
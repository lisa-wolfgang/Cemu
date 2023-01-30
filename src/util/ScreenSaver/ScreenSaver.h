#include "Cemu/Logging/CemuLogging.h"

#if BOOST_OS_MACOS
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

#if BOOST_OS_LINUX
#include <SDL2/SDL.h>
#endif

class ScreenSaver
{
public:
  static void SetInhibit(bool inhibit)
  {
    // Adapted from Dolphin's implementation: https://github.com/dolphin-emu/dolphin/blob/e3e6c3dfa41d377520f74ec2488fc1f7b6c05be3/Source/Core/UICommon/UICommon.cpp#L454
#if BOOST_OS_WINDOWS
    SetThreadExecutionState(ES_CONTINUOUS |
                            (inhibit ? (ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED) : 0));
#endif

#if BOOST_OS_MACOS
    static IOPMAssertionID s_power_assertion = kIOPMNullAssertionID;
    if (inhibit)
    {
      CFStringRef reason_for_activity = CFSTR("Emulation Running");
      if (IOPMAssertionCreateWithName(kIOPMAssertionTypePreventUserIdleDisplaySleep,
                                      kIOPMAssertionLevelOn, reason_for_activity,
                                      &s_power_assertion) != kIOReturnSuccess)
      {
        s_power_assertion = kIOPMNullAssertionID;
      }
    }
    else
    {
      if (s_power_assertion != kIOPMNullAssertionID)
      {
        IOPMAssertionRelease(s_power_assertion);
        s_power_assertion = kIOPMNullAssertionID;
      }
    }
#endif

#if BOOST_OS_LINUX
    // Initialize video subsystem if necessary
    if (SDL_WasInit(SDL_INIT_VIDEO) != 0)
    {
      int initErr = SDL_InitSubSystem(SDL_INIT_VIDEO);
      if (initErr)
      {
        cemuLog_force("Could not disable screen saver (SDL video subsystem initialization error)");
      }
    }
    // Toggle SDL's screen saver inhibition
    if (inhibit)
    {
      SDL_DisableScreenSaver();
      if (SDL_IsScreenSaverEnabled() == SDL_TRUE)
      {
        cemuLog_force("Could not verify if screen saver was disabled (`SDL_IsScreenSaverEnabled()` returned SDL_TRUE)");
      }
    }
    else
    {
      SDL_EnableScreenSaver();
      if (SDL_IsScreenSaverEnabled() == SDL_FALSE)
      {
        cemuLog_force("Could not verify if screen saver was re-enabled (`SDL_IsScreenSaverEnabled()` returned SDL_FALSE)");
      }
    }
#endif
  };
};

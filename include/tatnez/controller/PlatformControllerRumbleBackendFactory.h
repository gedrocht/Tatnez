#pragma once

#include "tatnez/controller/IControllerRumbleBackend.h"

#include <memory>

namespace tatnez::controller {

/**
 * @brief Create the best controller rumble backend that is available on the current platform.
 *
 * On Windows this function creates an XInput-backed implementation. On other operating systems
 * it creates a diagnostic backend that explains why live controller playback is unavailable.
 */
[[nodiscard]] auto createPlatformControllerRumbleBackend() -> std::unique_ptr<IControllerRumbleBackend>;

} // namespace tatnez::controller

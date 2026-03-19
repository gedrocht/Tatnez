#pragma once

#include "tatnez/controller/ControllerConnectionSummary.h"

#include <cstdint>
#include <string>

namespace tatnez::controller {

/**
 * @brief Abstraction over a rumble-capable controller backend.
 *
 * The core library talks to this interface instead of directly talking to XInput. That design
 * decision is what makes the project testable on GitHub Actions runners that do not have any
 * physical controller hardware attached.
 */
class IControllerRumbleBackend {
public:
  virtual ~IControllerRumbleBackend() = default;

  /**
   * @brief Return a short backend name such as `XInput`.
   */
  [[nodiscard]] virtual auto getBackendDisplayName() const -> std::string = 0;

  /**
   * @brief Inspect one controller slot and describe what was found.
   */
  [[nodiscard]] virtual auto queryControllerConnectionSummary(std::uint32_t controllerIndex) const
      -> ControllerConnectionSummary = 0;

  /**
   * @brief Send motor intensity values to one controller slot.
   *
   * @param controllerIndex Which controller slot should receive the rumble command.
   * @param largeMotorIntensity The low-frequency motor intensity in the range [0, 65535].
   * @param smallMotorIntensity The high-frequency motor intensity in the range [0, 65535].
   */
  virtual void setMotorIntensities(std::uint32_t controllerIndex, std::uint16_t largeMotorIntensity,
                                   std::uint16_t smallMotorIntensity) = 0;

  /**
   * @brief Stop both motors for one controller slot.
   */
  virtual void stopMotors(std::uint32_t controllerIndex) = 0;
};

} // namespace tatnez::controller

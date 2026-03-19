#pragma once

#include <cstdint>
#include <string>

namespace tatnez::controller {

/**
 * @brief Describes what the backend knows about one controller slot.
 */
struct ControllerConnectionSummary final {
  /**
   * @brief The numeric XInput slot index, typically in the inclusive range [0, 3].
   */
  std::uint32_t controllerIndex{};

  /**
   * @brief Whether the backend believes that a controller is connected in this slot.
   */
  bool isConnected{};

  /**
   * @brief A human-readable name for the backend that produced the summary.
   */
  std::string backendDisplayName{};

  /**
   * @brief Extra diagnostic text intended for logs and troubleshooting.
   */
  std::string diagnosticMessage{};
};

} // namespace tatnez::controller

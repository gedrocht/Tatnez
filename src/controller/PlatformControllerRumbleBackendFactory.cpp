#include "tatnez/controller/PlatformControllerRumbleBackendFactory.h"

#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#include <Xinput.h>
#endif

namespace {

#ifdef _WIN32

auto describeXInputResult(unsigned long xInputResultCode) -> std::string {
  switch (xInputResultCode) {
  case ERROR_SUCCESS:
    return "success";
  case ERROR_DEVICE_NOT_CONNECTED:
    return "device not connected";
  default:
    return "unexpected XInput error code " + std::to_string(xInputResultCode);
  }
}

class XInputControllerRumbleBackend final : public tatnez::controller::IControllerRumbleBackend {
public:
  [[nodiscard]] auto getBackendDisplayName() const -> std::string override { return "XInput"; }

  [[nodiscard]] auto queryControllerConnectionSummary(std::uint32_t controllerIndex) const
      -> tatnez::controller::ControllerConnectionSummary override {
    XINPUT_STATE controllerState{};
    const auto xInputResultCode = XInputGetState(controllerIndex, &controllerState);

    return tatnez::controller::ControllerConnectionSummary{controllerIndex, xInputResultCode == ERROR_SUCCESS,
                                                           getBackendDisplayName(),
                                                           describeXInputResult(xInputResultCode)};
  }

  void setMotorIntensities(std::uint32_t controllerIndex, std::uint16_t largeMotorIntensity,
                           std::uint16_t smallMotorIntensity) override {
    // XInput exposes one 16-bit value for each motor. The larger left motor is the heavy,
    // lower-frequency motor, and the right motor is the smaller higher-frequency motor.
    XINPUT_VIBRATION vibrationState{};
    vibrationState.wLeftMotorSpeed = largeMotorIntensity;
    vibrationState.wRightMotorSpeed = smallMotorIntensity;

    const auto xInputResultCode = XInputSetState(controllerIndex, &vibrationState);
    if (xInputResultCode != ERROR_SUCCESS) {
      throw std::runtime_error("XInputSetState failed for controller slot " +
                               std::to_string(controllerIndex) + " with result " +
                               describeXInputResult(xInputResultCode) + ".");
    }
  }

  void stopMotors(std::uint32_t controllerIndex) override { setMotorIntensities(controllerIndex, 0U, 0U); }
};

#else

class UnsupportedPlatformControllerRumbleBackend final : public tatnez::controller::IControllerRumbleBackend {
public:
  [[nodiscard]] auto getBackendDisplayName() const -> std::string override { return "Unsupported platform"; }

  [[nodiscard]] auto queryControllerConnectionSummary(std::uint32_t controllerIndex) const
      -> tatnez::controller::ControllerConnectionSummary override {
    // On non-Windows platforms there is no live XInput backend, so the summary is purely
    // diagnostic and intentionally explains the limitation instead of pretending the slot
    // is merely disconnected.
    return tatnez::controller::ControllerConnectionSummary{
        controllerIndex, false, getBackendDisplayName(),
        "Live controller rumble playback is only implemented for Windows via XInput. Use --dry-run "
        "on other platforms."};
  }

  void setMotorIntensities(std::uint32_t controllerIndex, std::uint16_t /* largeMotorIntensity */,
                           std::uint16_t /* smallMotorIntensity */) override {
    throw std::runtime_error(
        "Controller slot " + std::to_string(controllerIndex) +
        " cannot be driven because the current operating system does not provide the Windows "
        "XInput backend.");
  }

  void stopMotors(std::uint32_t /* controllerIndex */) override {}
};

#endif

} // namespace

namespace tatnez::controller {

auto createPlatformControllerRumbleBackend() -> std::unique_ptr<IControllerRumbleBackend> {
#ifdef _WIN32
  return std::make_unique<XInputControllerRumbleBackend>();
#else
  return std::make_unique<UnsupportedPlatformControllerRumbleBackend>();
#endif
}

} // namespace tatnez::controller

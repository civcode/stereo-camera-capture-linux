#ifndef PS3_EYE_CAMERA_HPP_
#define PS3_EYE_CAMERA_HPP_

namespace ps3_eye {
  enum class Resolution {
    kVGA,
    kQVGA
  };
  enum class FrameRate {
    k15FPS,
    k30FPS,
    k60FPS,
    k120FPS
  };
  enum class ColorMode {
    kColor,
    kGrayscale
  };
}

#endif // PS3_EYE_CAMERA_HPP_
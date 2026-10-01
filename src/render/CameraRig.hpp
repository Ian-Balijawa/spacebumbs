#pragma once

#include "physics/RigidBody.hpp"

namespace sg {

enum class CameraMode { Chase, Cockpit, Orbit };

class CameraRig {
public:
    CameraRig();

    void cycleMode();
    void snapTo(const RigidBody& target);
    void update(const RigidBody& target, float dt);

    const Camera3D& camera() const { return camera_; }
    CameraMode mode() const { return mode_; }
    const char* modeName() const;

private:
    Camera3D camera_{};
    CameraMode mode_ = CameraMode::Chase;
    Quaternion smoothed_ = QuaternionIdentity();
    float orbitAngle_ = 0.0f;
};

}  // namespace sg

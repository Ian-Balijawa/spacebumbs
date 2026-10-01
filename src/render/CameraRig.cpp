#include "render/CameraRig.hpp"

#include <cmath>

namespace sg {

CameraRig::CameraRig() {
    camera_.position = {0.0f, 5.0f, 14.0f};
    camera_.target = {0.0f, 0.0f, 0.0f};
    camera_.up = {0.0f, 1.0f, 0.0f};
    camera_.fovy = 70.0f;
    camera_.projection = CAMERA_PERSPECTIVE;
}

void CameraRig::cycleMode() {
    switch (mode_) {
        case CameraMode::Chase: mode_ = CameraMode::Cockpit; break;
        case CameraMode::Cockpit: mode_ = CameraMode::Orbit; break;
        case CameraMode::Orbit: mode_ = CameraMode::Chase; break;
    }
}

void CameraRig::snapTo(const RigidBody& target) { smoothed_ = target.orientation; }

const char* CameraRig::modeName() const {
    switch (mode_) {
        case CameraMode::Chase: return "Chase";
        case CameraMode::Cockpit: return "Cockpit";
        case CameraMode::Orbit: return "Orbit";
    }
    return "";
}

void CameraRig::update(const RigidBody& target, float dt) {
    // The chase camera follows the ship position rigidly but turns with a delay,
    // which keeps fast flight readable without dragging the camera behind.
    Quaternion goal = target.orientation;
    const float dot = smoothed_.x * goal.x + smoothed_.y * goal.y + smoothed_.z * goal.z + smoothed_.w * goal.w;
    if (dot < 0.0f) goal = {-goal.x, -goal.y, -goal.z, -goal.w};
    smoothed_ = QuaternionSlerp(smoothed_, goal, 1.0f - std::exp(-6.0f * dt));

    const Vector3 fwd = Vector3RotateByQuaternion({0.0f, 0.0f, -1.0f}, smoothed_);
    const Vector3 up = Vector3RotateByQuaternion({0.0f, 1.0f, 0.0f}, smoothed_);

    switch (mode_) {
        case CameraMode::Chase:
            camera_.fovy = 70.0f;
            camera_.position = target.position - fwd * 14.0f + up * 4.5f;
            camera_.target = target.position + fwd * 40.0f;
            camera_.up = up;
            break;
        case CameraMode::Cockpit:
            camera_.fovy = 80.0f;
            camera_.position = target.position + target.forward() * 1.4f + target.up() * 0.5f;
            camera_.target = camera_.position + target.forward() * 50.0f;
            camera_.up = target.up();
            break;
        case CameraMode::Orbit:
            camera_.fovy = 60.0f;
            orbitAngle_ += dt * 0.4f;
            camera_.position = target.position + Vector3{std::cos(orbitAngle_) * 30.0f, 9.0f, std::sin(orbitAngle_) * 30.0f};
            camera_.target = target.position;
            camera_.up = {0.0f, 1.0f, 0.0f};
            break;
    }
}

}  // namespace sg

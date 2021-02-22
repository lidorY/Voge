#pragma once

#include <DirectXMath.h>

// TODO: move those things to cpp file
#undef min // Use __min instead.
#undef max // Use __max instead.

class Camera {
public:
    Camera() {

        using namespace DirectX;
        // Setup the view matrix.
        SetViewParams(
            XMFLOAT3(0.0f, 0.0f, 0.0f),   // Default eye position.
            XMFLOAT3(0.0f, 0.0f, 1.0f),   // Default look at position.
            XMFLOAT3(0.0f, 1.0f, 0.0f)    // Default up vector.
        );

        // Setup the projection matrix.
        SetProjParams(XM_PI / 4, 1.0f, 1.0f, 1000.0f);
    }
    Camera(Camera const&) = delete;
    void operator=(Camera const&) = delete;

    DirectX::XMMATRIX Projection() {
        return DirectX::XMLoadFloat4x4(&projection_matrix_);
    }

private:
    void SetViewParams(
        _In_ DirectX::XMFLOAT3 eye,
        _In_ DirectX::XMFLOAT3 lookAt,
        _In_ DirectX::XMFLOAT3 up) {
        using namespace DirectX;
        
        eye_ = eye;
        look_at_ = lookAt;
        up_ = up;

        // Calculate the view matrix.
        XMMATRIX view = XMMatrixLookAtLH(
            XMLoadFloat3(&eye_),
            XMLoadFloat3(&look_at_),
            XMLoadFloat3(&up_)
        );

        XMVECTOR det;
        XMMATRIX inverseView = XMMatrixInverse(&det, view);
        XMStoreFloat4x4(&view_matrix_, view);
        XMStoreFloat4x4(&inverse_view_matrix_, inverseView);
        
        // The axis basis vectors and camera position are stored inside the
        // position matrix in the 4 rows of the camera's world matrix.
        // To figure out the yaw/pitch of the camera, we just need the Z basis vector.
        XMFLOAT3 zBasis;
        XMStoreFloat3(&zBasis, inverseView.r[2]);
        float len = sqrtf(zBasis.z * zBasis.z + zBasis.x * zBasis.x);
        camera_pitch_angle_ = atan2f(zBasis.y, len);
    }

    void SetProjParams(
        _In_ float fieldOfView,
        _In_ float aspectRatio,
        _In_ float nearPlane,
        _In_ float farPlane
    ) {
        using namespace DirectX;
        field_of_view_ = fieldOfView;
        aspect_ratio_ = aspectRatio;
        near_plane_ = nearPlane;
        far_plane_ = farPlane;

        XMStoreFloat4x4(
            &projection_matrix_,
            XMMatrixPerspectiveFovLH(
                field_of_view_,
                aspect_ratio_,
                near_plane_,
                far_plane_
            )
        );
    }

    DirectX::XMFLOAT3 eye_;
    DirectX::XMFLOAT3 look_at_;
    DirectX::XMFLOAT3 up_;

    DirectX::XMFLOAT4X4 view_matrix_;
    DirectX::XMFLOAT4X4 inverse_view_matrix_;

    
    float camera_pitch_angle_; // Pitch == Up/Down

    // Projection data
    // we calculate and cache in addition 
    // the projection matrix
    // TODO: any reason to cahce those values as well?
    float  field_of_view_;
    float  aspect_ratio_;
    float  near_plane_;
    float  far_plane_;

    DirectX::XMFLOAT4X4 projection_matrix_;
};

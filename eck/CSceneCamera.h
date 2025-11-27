#pragma once
#include "ECK.h"

#include <DirectXMath.h>

ECK_NAMESPACE_BEGIN
struct alignas(16) CSceneCamera
{
    DirectX::XMFLOAT3 Pos{};                // 相机位置
    float NearZ{ 1.f };                     // 近平面
    DirectX::XMFLOAT3 LookTo{ 0.f,0.f,1.f };// (相机Z)视线向量，单位向量
    float FarZ{ 1000.f };                   // 远平面
    DirectX::XMFLOAT3 Right{ 1.f,0.f,0.f }; // (相机X)右向量，单位向量
    float Aspect{ 1.f };                    // 横纵比
    DirectX::XMFLOAT3 Up{ 0.f,1.f,0.f };    // 世界正上向量，单位向量
    float FovY{ 0.25f * PiF };              // 纵向视角

    EckInlineNd DirectX::XMVECTOR XmLoadPosition() const noexcept
    {
        return DirectX::XMLoadFloat3A((const DirectX::XMFLOAT3A*)&Pos);
    }
    EckInlineNd DirectX::XMVECTOR XmLoadLookTo() const noexcept
    {
        return DirectX::XMLoadFloat3A((const DirectX::XMFLOAT3A*)&LookTo);
    }
    EckInlineNd DirectX::XMVECTOR XmLoadRight() const noexcept
    {
        return DirectX::XMLoadFloat3A((const DirectX::XMFLOAT3A*)&Right);
    }
    EckInlineNd DirectX::XMVECTOR XmLoadWorldUp() const noexcept
    {
        return DirectX::XMLoadFloat3A((const DirectX::XMFLOAT3A*)&Up);
    }

    EckInlineNd DirectX::XMMATRIX CalcViewMatrix() const noexcept
    {
        return DirectX::XMMatrixLookToLH(XmLoadPosition(), XmLoadLookTo(), XmLoadWorldUp());
    }
    EckInlineNd DirectX::XMMATRIX CalcProjectMatrix() const noexcept
    {
        return DirectX::XMMatrixPerspectiveFovLH(FovY, Aspect, NearZ, FarZ);
    }
    EckInlineNd DirectX::XMMATRIX CalcViewProjectMatrix() const noexcept
    {
        return CalcViewMatrix() * CalcProjectMatrix();
    }

    void WalkX(float d) noexcept
    {
        Pos.x += (Right.x * d);
        Pos.y += (Right.y * d);
        Pos.z += (Right.z * d);
    }
    void WalkY(float d) noexcept
    {
        const auto U = DirectX::XMVector3Cross(XmLoadLookTo(), XmLoadRight());
        const auto D = DirectX::XMVectorReplicate(d);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Pos,
            DirectX::XMVectorMultiplyAdd(D, U, XmLoadPosition()));
    }
    void WalkZ(float d) noexcept
    {
        Pos.x += (LookTo.x * d);
        Pos.y += (LookTo.y * d);
        Pos.z += (LookTo.z * d);
    }
    void WalkWorldY(float d) noexcept
    {
        Pos.x += (Up.x * d);
        Pos.y += (Up.y * d);
        Pos.z += (Up.z * d);
    }

    void RotateX(float Angle) noexcept
    {
        const auto R = DirectX::XMMatrixRotationAxis(XmLoadRight(), Angle);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&LookTo,
            DirectX::XMVector3TransformNormal(XmLoadLookTo(), R));
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Up,
            DirectX::XMVector3TransformNormal(XmLoadWorldUp(), R));
    }
    void RotateY(float Angle) noexcept
    {
        const auto R = DirectX::XMMatrixRotationAxis(
            DirectX::XMVector3Cross(XmLoadLookTo(), XmLoadRight()), Angle);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&LookTo,
            DirectX::XMVector3TransformNormal(XmLoadLookTo(), R));
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Up,
            DirectX::XMVector3TransformNormal(XmLoadWorldUp(), R));
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Right,
            DirectX::XMVector3TransformNormal(XmLoadRight(), R));
    }
    void RotateZ(float Angle) noexcept
    {
        const auto R = DirectX::XMMatrixRotationAxis(XmLoadLookTo(), Angle);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Right,
            DirectX::XMVector3TransformNormal(XmLoadRight(), R));
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Up,
            DirectX::XMVector3TransformNormal(XmLoadWorldUp(), R));
    }
    void RotateWorldY(float Angle) noexcept
    {
        const auto R = DirectX::XMMatrixRotationY(Angle);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&LookTo,
            DirectX::XMVector3TransformNormal(XmLoadLookTo(), R));
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Up,
            DirectX::XMVector3TransformNormal(XmLoadWorldUp(), R));
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Right,
            DirectX::XMVector3TransformNormal(XmLoadRight(), R));
    }

    void LookAt(DirectX::FXMVECTOR ptCamPos, DirectX::FXMVECTOR ptTarget,
        DirectX::FXMVECTOR vecWorldUp) noexcept
    {
        const auto L = DirectX::XMVector3Normalize(
            DirectX::XMVectorSubtract(ptTarget, ptCamPos));
        const auto R = DirectX::XMVector3Normalize(
            DirectX::XMVector3Cross(vecWorldUp, L));
        const auto U = DirectX::XMVector3Cross(L, R);

        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Pos, ptCamPos);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&LookTo, L);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Right, R);
        DirectX::XMStoreFloat3A((DirectX::XMFLOAT3A*)&Up, U);
    }
};
ECK_NAMESPACE_END
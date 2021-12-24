#pragma once
#include "pch.h"

class PerspectiveCamera {


	//обновляет матрицу на основе текущих значений переменных.
	void Update();
	DirectX::XMMATRIX GetCommonMatrix();
	DirectX::XMMATRIX GetProjectionMatrix();
	DirectX::XMMATRIX GetViewMatrix();

	void SetAspectRatio();	
	void SetFlovAngleY();
	void SetEye();
	void SetAt();
	void SetUp();

	float GetAspectRatio();
	float GetFlovAngleY();
	DirectX::XMVECTORF32 GetEye();
	DirectX::XMVECTORF32 GetAt();
	DirectX::XMVECTORF32 GetUp();

	DirectX::XMMATRIX projectionMatrix;
	float flovAngleY = 70.0f * DirectX::XM_PI/180.0f;
	float aspectRatio;

	DirectX::XMMATRIX viewMatrix;
	DirectX::XMVECTORF32 eye = { 0.0f, 0.0f, 1.5f, 0.0f };
	DirectX::XMVECTORF32 at = { 0.0f, -0.1f, 0.0f, 0.0f };
	DirectX::XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	DirectX::XMMATRIX commonMatrix;

};


class OrthogonalCamera {



};
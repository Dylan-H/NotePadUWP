#pragma once
#include "pch.h"
#include<cmath>
#define M_PI       3.14159265358979323846
using namespace winrt::Windows::UI::Xaml::Media;
namespace winrt::NotePad {

	static Matrix CreateSkewRadians(double skewX, double skewY)
	{

		return  Matrix{ 1.0, tan(skewY),
			tan(skewX), 1.0,
			0.0, 0.0 };

	}
	static Matrix Multiply(Matrix matrix1, Matrix matrix2)
	{
		return  Matrix{
			(matrix1.M11 * matrix2.M11) + (matrix1.M12 * matrix2.M21),
			(matrix1.M11 * matrix2.M12) + (matrix1.M12 * matrix2.M22),
			(matrix1.M21 * matrix2.M11) + (matrix1.M22 * matrix2.M21),
			(matrix1.M21 * matrix2.M12) + (matrix1.M22 * matrix2.M22),
			((matrix1.OffsetX * matrix2.M11) + (matrix1.OffsetY * matrix2.M21)) + matrix2.OffsetX,
			((matrix1.OffsetX * matrix2.M12) + (matrix1.OffsetY * matrix2.M22)) + matrix2.OffsetY };
	}
	static Matrix CreateRotationRadians(double angle, double centerX, double centerY)
	{
		auto dsin = sin(angle);
		auto dcos = cos(angle);
		auto dx = (centerX * (1.0 - dcos)) + (centerY * dsin);
		auto dy = (centerY * (1.0 - dcos)) - (centerX * dsin);
		return  Matrix{ dcos, dsin,
			-dsin, dcos,
			dx, dy };

	}
	static Matrix Translate(Matrix matrix, double offsetX, double offsetY)
	{
		return  Matrix{ matrix.M11, matrix.M12, matrix.M21, matrix.M22, matrix.OffsetX + offsetX, matrix.OffsetY + offsetY };
	}
	static Matrix Skew(Matrix matrix, double skewX, double skewY)
	{
		return Multiply(matrix, CreateSkewRadians(fmod(skewX , 360) * (M_PI / 180.0), fmod(skewY , 360) * (M_PI / 180.0)));
	}

	static Matrix GetMatrix(SkewTransform transform)
	{
		Matrix matrix = MatrixHelper::Identity();

		auto angleX = transform.AngleX();
		auto angleY = transform.AngleY();
		auto centerX = transform.CenterX();
		auto centerY = transform.CenterY();

		bool hasCenter = centerX != 0 || centerY != 0;

		if (hasCenter)
		{
			// If we have a center, translate matrix before/after skewing.
			matrix = Translate(matrix, -centerX, -centerY);
			matrix = Skew(matrix, angleX, angleY);
			matrix = Translate(matrix, centerX, centerY);
		}
		else
		{
			matrix = Skew(matrix, angleX, angleY);
		}

		return matrix;
	}

	static Matrix CreateScaling(double scaleX, double scaleY, double centerX, double centerY)
	{

		return  Matrix{ scaleX, 0,
			0, scaleY,
			centerX - (scaleX * centerX), centerY - (scaleY * centerY) };

	}
	static Matrix ScaleAt(Matrix matrix, double scaleX, double scaleY, double centerX, double centerY)
	{
		return Multiply(matrix, CreateScaling(scaleX, scaleY, centerX, centerY));
	}
	static Matrix  GetMatrix(ScaleTransform transform)
	{
		return ScaleAt(MatrixHelper::Identity(), transform.ScaleX(), transform.ScaleY(), transform.CenterX(), transform.CenterY());
	}
	static Matrix RotateAt(Matrix matrix, double angle, double centerX, double centerY)
	{
		return Multiply(matrix, CreateRotationRadians(fmod(angle , 360) * (M_PI / 180.0), centerX, centerY));
	}
	static Matrix GetMatrix(RotateTransform transform)
	{
		return RotateAt(MatrixHelper::Identity(), transform.Angle(), transform.CenterX(), transform.CenterY());
	}

	static Matrix Round(Matrix matrix)
	{
		return  Matrix{
			round(matrix.M11),
			round(matrix.M12),
			round(matrix.M21),
			round(matrix.M22),
			matrix.OffsetX,
			matrix.OffsetY };
	}
	static bool HasInverse(Matrix matrix)
	{
		// TODO: Check if we can make this an extension property in C#8.

		// WPF equivalent of following code:
		// return matrix.HasInverse;
		return ((matrix.M11 * matrix.M22) - (matrix.M12 * matrix.M21)) != 0;
	}
}
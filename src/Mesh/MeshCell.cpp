#include "Mesh/MeshCell.h"

#include <algorithm>

namespace
{
    void ClearLayer(MaterialType& material, double& fraction)
    {
        material = MaterialType::Empty;
        fraction = 0.0;
    }

    void NormalizeLayer(MaterialType& material, double& fraction)
    {
        fraction = std::clamp(fraction, 0.0, 1.0);

        if (fraction <= kFractionEpsilon)
        {
            ClearLayer(material, fraction);
        }
    }
}

MeshCell::MeshCell(
    int i,
    int j,
    int k,
    double x,
    double y,
    double z,
    double dx,
    double dy,
    double dz,
    MaterialType material
)
    : i_(i),
      j_(j),
      k_(k),
      x_(x),
      y_(y),
      z_(z),
      dx_(dx),
      dy_(dy),
      dz_(dz),
      primaryMaterial_(material),
      primaryFraction_(material == MaterialType::Empty ? 0.0 : 1.0),
      secondaryMaterial_(MaterialType::Empty),
      secondaryFraction_(0.0)
{
}

int MeshCell::GetI() const
{
    return i_;
}

int MeshCell::GetJ() const
{
    return j_;
}

int MeshCell::GetK() const
{
    return k_;
}

double MeshCell::GetX() const
{
    return x_;
}

double MeshCell::GetY() const
{
    return y_;
}

double MeshCell::GetZ() const
{
    return z_;
}

double MeshCell::GetDx() const
{
    return dx_;
}

double MeshCell::GetDy() const
{
    return dy_;
}

double MeshCell::GetDz() const
{
    return dz_;
}

MaterialType MeshCell::GetPrimaryMaterial() const
{
    return primaryMaterial_;
}

double MeshCell::GetPrimaryFraction() const
{
    return primaryFraction_;
}

MaterialType MeshCell::GetSecondaryMaterial() const
{
    return secondaryMaterial_;
}

double MeshCell::GetSecondaryFraction() const
{
    return secondaryFraction_;
}

MaterialType MeshCell::GetSurfaceMaterial() const
{
    if (secondaryMaterial_ != MaterialType::Empty &&
        secondaryFraction_ > kFractionEpsilon)
    {
        return secondaryMaterial_;
    }

    return primaryMaterial_;
}

double MeshCell::GetMaterialFraction() const
{
    return primaryFraction_ + secondaryFraction_;
}

double MeshCell::GetCellVolume() const
{
    return dx_ * dy_ * dz_;
}

double MeshCell::GetRemainingVolume() const
{
    return GetCellVolume() * GetMaterialFraction();
}

double MeshCell::GetRemainingVolume(MaterialType material) const
{
    double volume = 0.0;

    if (primaryMaterial_ == material)
    {
        volume += GetCellVolume() * primaryFraction_;
    }

    if (secondaryMaterial_ == material)
    {
        volume += GetCellVolume() * secondaryFraction_;
    }

    return volume;
}

bool MeshCell::IsEmpty() const
{
    return GetMaterialFraction() <= kFractionEpsilon;
}

bool MeshCell::IsFull() const
{
    return !IsEmpty() && GetMaterialFraction() >= 1.0 - kFractionEpsilon;
}

bool MeshCell::HasMultipleMaterials() const
{
    return primaryMaterial_ != MaterialType::Empty &&
           primaryFraction_ > kFractionEpsilon &&
           secondaryMaterial_ != MaterialType::Empty &&
           secondaryFraction_ > kFractionEpsilon;
}

void MeshCell::RemoveFraction(double fraction)
{
    if (fraction <= 0.0 || IsEmpty())
    {
        return;
    }

    double remaining = fraction;

    if (secondaryMaterial_ != MaterialType::Empty &&
        secondaryFraction_ > kFractionEpsilon)
    {
        const double removedFromSecondary =
            std::min(remaining, secondaryFraction_);

        secondaryFraction_ -= removedFromSecondary;
        remaining -= removedFromSecondary;

        NormalizeLayer(secondaryMaterial_, secondaryFraction_);
    }

    if (remaining > kFractionEpsilon &&
        primaryMaterial_ != MaterialType::Empty &&
        primaryFraction_ > kFractionEpsilon)
    {
        primaryFraction_ -= remaining;
        NormalizeLayer(primaryMaterial_, primaryFraction_);
    }

    if (IsEmpty())
    {
        Clear();
    }
}

void MeshCell::AddFraction(double fraction, MaterialType material)
{
    if (fraction <= 0.0 || material == MaterialType::Empty)
    {
        return;
    }

    const double voidFraction =
        1.0 - primaryFraction_ - secondaryFraction_;

    if (voidFraction <= kFractionEpsilon)
    {
        return;
    }

    const double addedFraction = std::min(fraction, voidFraction);

    if (IsEmpty())
    {
        primaryMaterial_ = material;
        primaryFraction_ = addedFraction;
        return;
    }

    if (secondaryMaterial_ != MaterialType::Empty)
    {
        if (secondaryMaterial_ == material)
        {
            secondaryFraction_ += addedFraction;
            NormalizeLayer(secondaryMaterial_, secondaryFraction_);
            return;
        }

        return;
    }

    if (primaryMaterial_ == material)
    {
        primaryFraction_ += addedFraction;
        NormalizeLayer(primaryMaterial_, primaryFraction_);
        return;
    }

    secondaryMaterial_ = material;
    secondaryFraction_ = addedFraction;
    NormalizeLayer(secondaryMaterial_, secondaryFraction_);
}

double MeshCell::RemoveMaterial(MaterialType material)
{
    if (material == MaterialType::Empty)
    {
        return 0.0;
    }

    double removedFraction = 0.0;

    if (secondaryMaterial_ == material &&
        secondaryFraction_ > kFractionEpsilon)
    {
        removedFraction += secondaryFraction_;
        ClearLayer(secondaryMaterial_, secondaryFraction_);
    }

    if (primaryMaterial_ == material &&
        primaryFraction_ > kFractionEpsilon)
    {
        removedFraction += primaryFraction_;

        // Promote the remaining secondary layer so the cell never
        // ends up with an empty primary below a filled secondary.
        primaryMaterial_ = secondaryMaterial_;
        primaryFraction_ = secondaryFraction_;
        ClearLayer(secondaryMaterial_, secondaryFraction_);
    }

    if (IsEmpty())
    {
        Clear();
    }

    return removedFraction;
}

void MeshCell::Clear()
{
    ClearLayer(primaryMaterial_, primaryFraction_);
    ClearLayer(secondaryMaterial_, secondaryFraction_);
}

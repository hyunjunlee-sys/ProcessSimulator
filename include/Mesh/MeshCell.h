#ifndef MESHCELL_H
#define MESHCELL_H

#include "Materials/Material.h"

// Fractions at or below this value are treated as zero.
inline constexpr double kFractionEpsilon = 1e-9;

class MeshCell
{
private:
    int i_;
    int j_;
    int k_;

    double x_;
    double y_;
    double z_;

    double dx_;
    double dy_;
    double dz_;

    MaterialType primaryMaterial_;
    double primaryFraction_;

    MaterialType secondaryMaterial_;
    double secondaryFraction_;

private:
    void Clear();

public:
    MeshCell(
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
    );

    int GetI() const;
    int GetJ() const;
    int GetK() const;

    double GetX() const;
    double GetY() const;
    double GetZ() const;

    double GetDx() const;
    double GetDy() const;
    double GetDz() const;

    MaterialType GetPrimaryMaterial() const;
    double GetPrimaryFraction() const;

    MaterialType GetSecondaryMaterial() const;
    double GetSecondaryFraction() const;

    MaterialType GetSurfaceMaterial() const;
    double GetMaterialFraction() const;

    double GetCellVolume() const;
    double GetRemainingVolume() const;
    double GetRemainingVolume(MaterialType material) const;

    bool IsEmpty() const;
    bool IsFull() const;
    bool HasMultipleMaterials() const;

    void RemoveFraction(double fraction);
    void AddFraction(double fraction, MaterialType material);

    // Removes every layer of the given material from this cell and
    // returns the removed fraction (0.0 if the material is absent).
    double RemoveMaterial(MaterialType material);
};

#endif
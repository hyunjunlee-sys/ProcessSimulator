#include "Mesh/MeshGrid.h"
#include "Process/Etcher.h"
#include "Process/Depositor.h"
#include "Process/Photolithography.h"

#include <cmath>
#include <iostream>
#include <string>

namespace
{
    int g_failures = 0;

    bool NearlyEqual(double a, double b)
    {
        return std::fabs(a - b) < 1e-6;
    }

    void Check(bool condition, const std::string& name)
    {
        if (condition)
        {
            std::cout << "[ PASS ] " << name << "\n";
        }
        else
        {
            std::cout << "[ FAIL ] " << name << "\n";
            ++g_failures;
        }
    }

    // dx = dy = dz = 5 -> a full cell is 125 nm^3.
    constexpr double kCellVolume = 125.0;

    MeshGrid MakeGrid(int nx, int ny, int nz, int substrateLayers)
    {
        return MeshGrid(
            nx, ny, nz,
            5.0, 5.0, 5.0,
            MaterialType::Silicon,
            substrateLayers
        );
    }

    void TestSubstrateInitialization()
    {
        MeshGrid mesh = MakeGrid(1, 1, 4, 2);

        Check(
            !mesh.At(0, 0, 0).IsEmpty() && mesh.At(0, 0, 0).IsFull(),
            "substrate bottom cell is full silicon"
        );
        Check(
            mesh.At(0, 0, 2).IsEmpty(),
            "cell above substrate is empty"
        );
        Check(
            NearlyEqual(
                mesh.CalculateTotalRemainingVolume(MaterialType::Silicon),
                2 * kCellVolume
            ),
            "initial silicon volume matches substrate layers"
        );
    }

    void TestDepositFillsFromBottom()
    {
        MeshGrid mesh = MakeGrid(1, 1, 4, 2);

        Depositor depositor;
        depositor.DepositBoxRegion(
            mesh, 0, 0, 0, 0, 5.0, MaterialType::Oxide
        );

        Check(
            mesh.At(0, 0, 2).IsFull() &&
            mesh.At(0, 0, 2).GetPrimaryMaterial() == MaterialType::Oxide,
            "deposition fills the lowest empty cell with oxide"
        );
        Check(
            mesh.At(0, 0, 3).IsEmpty(),
            "deposition does not overfill above the requested depth"
        );
    }

    void TestEtchRemovesFromTop()
    {
        MeshGrid mesh = MakeGrid(1, 1, 4, 4);

        Etcher etcher;
        etcher.EtchBoxRegion(mesh, 0, 0, 0, 0, 5.0);

        Check(
            mesh.At(0, 0, 3).IsEmpty(),
            "etch removes the topmost cell"
        );
        Check(
            NearlyEqual(
                mesh.CalculateTotalRemainingVolume(MaterialType::Silicon),
                3 * kCellVolume
            ),
            "etch removes exactly one cell of material"
        );
    }

    void TestThirdMaterialRejected()
    {
        MeshGrid mesh = MakeGrid(1, 1, 1, 0);
        MeshCell& cell = mesh.At(0, 0, 0);

        const double addedSilicon =
            cell.AddFraction(0.5, MaterialType::Silicon);
        const double addedOxide =
            cell.AddFraction(0.3, MaterialType::Oxide);
        const double addedMetal =
            cell.AddFraction(0.1, MaterialType::Metal);

        Check(NearlyEqual(addedSilicon, 0.5), "first material added");
        Check(NearlyEqual(addedOxide, 0.3), "second material added");
        Check(
            NearlyEqual(addedMetal, 0.0),
            "third distinct material is rejected"
        );
    }

    void TestPhotoresistMasksEtch()
    {
        // Two columns; only column 1 has its resist developed open.
        MeshGrid mesh = MakeGrid(2, 1, 6, 3);

        Photolithography litho;
        litho.PatternBoxRegion(mesh, 1, 1, 0, 0, 5.0);

        Etcher etcher;
        etcher.EtchBoxRegion(mesh, 0, 1, 0, 0, 100.0);

        const double column0Silicon =
            mesh.At(0, 0, 0).GetRemainingVolume(MaterialType::Silicon) +
            mesh.At(0, 0, 1).GetRemainingVolume(MaterialType::Silicon) +
            mesh.At(0, 0, 2).GetRemainingVolume(MaterialType::Silicon);

        Check(
            NearlyEqual(column0Silicon, 3 * kCellVolume),
            "resist-masked column is protected from etch"
        );
        Check(
            NearlyEqual(
                mesh.CalculateTotalRemainingVolume(MaterialType::Silicon),
                3 * kCellVolume
            ),
            "open column is fully etched away"
        );
    }

    void TestStripRemovesAllResist()
    {
        MeshGrid mesh = MakeGrid(1, 1, 4, 2);

        Photolithography litho;
        litho.PatternBoxRegion(mesh, 0, 0, 0, 0, 5.0);

        const double strippedBefore =
            litho.StripResist(mesh);

        Check(
            strippedBefore == 0.0,
            "developed window leaves no resist to strip"
        );

        // Coat without developing this single column: pattern an empty
        // region so the blanket coat stays in place, then strip it.
        MeshGrid mesh2 = MakeGrid(2, 1, 4, 2);
        litho.PatternBoxRegion(mesh2, 1, 1, 0, 0, 5.0);

        const double stripped = litho.StripResist(mesh2);

        Check(
            NearlyEqual(stripped, kCellVolume),
            "stripping removes the remaining resist volume"
        );
        Check(
            mesh2.CountCellsByMaterial(MaterialType::Photoresist) == 0,
            "no photoresist remains after strip"
        );
    }
}

int main()
{
    TestSubstrateInitialization();
    TestDepositFillsFromBottom();
    TestEtchRemovesFromTop();
    TestThirdMaterialRejected();
    TestPhotoresistMasksEtch();
    TestStripRemovesAllResist();

    if (g_failures == 0)
    {
        std::cout << "\nAll tests passed.\n";
        return 0;
    }

    std::cout << "\n" << g_failures << " test(s) failed.\n";
    return 1;
}

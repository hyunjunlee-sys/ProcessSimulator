#ifndef REGIONCOLUMNITERATOR_H
#define REGIONCOLUMNITERATOR_H

#include "Mesh/MeshGrid.h"

// Invokes `action(i, j)` for every in-bounds (i, j) column in the
// inclusive box [iStart, iEnd] x [jStart, jEnd]. Templated on the
// action type so the callable is inlined with no type-erasure overhead.
template <typename Action>
inline void ForEachColumnInBox(
    const MeshGrid& mesh,
    int iStart,
    int iEnd,
    int jStart,
    int jEnd,
    Action action
)
{
    for (int j = jStart; j <= jEnd; ++j)
    {
        for (int i = iStart; i <= iEnd; ++i)
        {
            if (mesh.IsValidIndex(i, j, 0))
            {
                action(i, j);
            }
        }
    }
}

#endif

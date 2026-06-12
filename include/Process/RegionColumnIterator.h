#ifndef REGIONCOLUMNITERATOR_H
#define REGIONCOLUMNITERATOR_H

#include "Mesh/MeshGrid.h"

#include <functional>

inline void ForEachColumnInBox(
    const MeshGrid& mesh,
    int iStart,
    int iEnd,
    int jStart,
    int jEnd,
    const std::function<void(int i, int j)>& action
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

#pragma once

namespace geo {

struct Coordinates {
    Coordinates();
    Coordinates(double _lat, double _lng);

    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;

    double lat = 0.0;
    double lng = 0.0;
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
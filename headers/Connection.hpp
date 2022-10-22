#pragma once
#include <cstdlib>

class Connection {
public:
    Connection() {_weight = randomizeWeight();}     // Assing random weight - to be trained


    void weight(double weight) {_weight = weight;}
    double weight() const {return _weight;}

    void weightChange(double weightChange) {_weightChange = weightChange;}
    double weightChange() const {return _weightChange;}

private:
    double _weight;
    double _weightChange;

    static double randomizeWeight() {return rand() / double(RAND_MAX);}
};
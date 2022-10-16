#pragma once
#include <cstdlib>

class Connection {
public:
    Connection();
    ~Connection();


    void weight(double weight);
    double weight() const;

    void weightChange(double weightChange);
    double weightChange() const;

private:
    double _weight;
    double _weightChange;

    static double randomizeWeight() {return rand() / double(RAND_MAX);}
};
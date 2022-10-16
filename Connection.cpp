#include <cstdlib>

#include "headers/Connection.hpp"

Connection::Connection() {_weight = randomizeWeight();}
Connection::~Connection(){}

void Connection::weight(double weight) {_weight = weight;}
double Connection::weight() const {return _weight;}

void Connection::weightChange(double weightChange) {_weightChange = weightChange;}
double Connection::weightChange() const {return _weightChange;}
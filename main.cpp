#include "LinkedList.hpp"
#include <iostream>
#include <string>
#include <emscripten/bind.h>

collection::LinkedList<std::string> ll;

void insert(std::string data) {
    ll.insert(data);
}

void insertR(std::string data) {
    ll.insertR(data);
}

void removeN() {
    ll.remove();
}

void removeR() {
    ll.removeR();
}

int length() {
    return ll.length();
}

std::string items() {
    return ll.display();
}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("insert", &insert);
    emscripten::function("insertR",&insertR);
    emscripten::function("remove", &removeN);
    emscripten::function("removeR",&removeR);
    emscripten::function("length", &length);
    emscripten::function("items",&items);
}

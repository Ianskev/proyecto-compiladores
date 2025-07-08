#include "imp_value.h"
#include <iostream>

ImpValue::ImpValue() : type(NOTYPE), int_value(0), bool_value(false), string_value("") {}

ImpValue::ImpValue(int v) : type(TINT), int_value(v), bool_value(false), string_value("") {}

ImpValue::ImpValue(bool v) : type(TBOOL), int_value(0), bool_value(v), string_value("") {}

ImpValue::ImpValue(string v) : type(TSTRING), int_value(0), bool_value(false), string_value(v) {}

ImpValue::ImpValue(ImpVType t) : type(t), int_value(0), bool_value(false), string_value("") {
    set_default_value(t);
}

void ImpValue::set_default_value(ImpVType tt) {
    type = tt;
    switch(tt) {
        case TINT:
            int_value = 0;
            break;
        case TBOOL:
            bool_value = false;
            break;
        case TSTRING:
            string_value = "";
            break;
        default:
            break;
    }
}

ImpVType ImpValue::get_basic_type(string s) {
    if (s == "int") return TINT;
    if (s == "bool") return TBOOL;
    if (s == "string") return TSTRING;
    return NOTYPE;
}

string ImpValue::to_string() {
    switch(type) {
        case TINT:
            return std::to_string(int_value);
        case TBOOL:
            return bool_value ? "true" : "false";
        case TSTRING:
            return string_value;
        default:
            return "undefined";
    }
}
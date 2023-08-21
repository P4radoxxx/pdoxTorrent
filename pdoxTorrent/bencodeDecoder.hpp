#include <iostream>
#include <string>
#include <vector>
#include <map>

enum class BencodeType {
    Integer,
    String,
    List,
    Dictionary
};

struct BencodeValue {
    BencodeType type;
    union {
        long long integer;
        std::string* str;
        std::vector<BencodeValue*>* list;
        std::map<std::string, BencodeValue*>* dictionary;
    };
};

BencodeValue* decodeBencode(std::istream& input);

BencodeValue* decodeInteger(std::istream& input) {
    std::string integerStr;
    char c;
    while (input.get(c) && c != 'e') {
        integerStr += c;
    }
    BencodeValue* value = new BencodeValue;
    value->type = BencodeType::Integer;
    value->integer = std::stoll(integerStr);
    return value;
}

BencodeValue* decodeString(std::istream& input) {
    std::string lengthStr;
    char c;
    while (input.get(c) && c != ':') {
        lengthStr += c;
    }
    int length = std::stoi(lengthStr);
    std::string* str = new std::string(length, ' ');
    input.read(&((*str)[0]), length);
    BencodeValue* value = new BencodeValue;
    value->type = BencodeType::String;
    value->str = str;
    return value;
}

BencodeValue* decodeList(std::istream& input) {
    std::vector<BencodeValue*>* list = new std::vector<BencodeValue*>;
    char c;
    while (input.peek() != 'e') {
        BencodeValue* element = decodeBencode(input);
        list->push_back(element);
    }
    input.get(c);
    BencodeValue* value = new BencodeValue;
    value->type = BencodeType::List;
    value->list = list;
    return value;
}

BencodeValue* decodeDictionary(std::istream& input) {
    std::map<std::string, BencodeValue*>* dictionary = new std::map<std::string, BencodeValue*>;
    char c;
    while (input.peek() != 'e') {
        std::string* key = decodeString(input)->str;
        BencodeValue* value = decodeBencode(input);
        (*dictionary)[*key] = value;
        delete key;
    }
    input.get(c);
    BencodeValue* result = new BencodeValue;
    result->type = BencodeType::Dictionary;
    result->dictionary = dictionary;
    return result;
}

BencodeValue* decodeBencode(std::istream& input) {
    char c;
    if (!input.get(c)) {
        return nullptr;
    }

    switch (c) {
    case 'i': return decodeInteger(input);
    case '0' ... '9': {
        input.unget();
        return decodeString(input);
    }
    case 'l': return decodeList(input);
    case 'd': return decodeDictionary(input);
    default: return nullptr;
    }
}

int main() {
    std::string bencodeData = "li123ei456e5:helloe";

    std::istringstream input(bencodeData);
    BencodeValue* value = decodeBencode(input);

    // Access and process the decoded data here

    return 0;
}

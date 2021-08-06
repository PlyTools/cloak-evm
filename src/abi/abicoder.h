#pragma once
#include "iostream"
#include "vector"
#include <stdio.h>
#include <stdlib.h>
#include "string"
#include <array>
#include "errors.h"
#include "../app/utils.h"
#include <sstream>
#include <eEVM/address.h>
#include <eEVM/bigint.h>
#include <eEVM/processor.h>
#include <eEVM/rlp.h>
#include <eEVM/util.h>
#include "math.h"
#include "optional"
#include "parsing.h"
#include "unordered_map"
using namespace std;

using ByteData = std::string;
using uint256 = intx::uint256;
using UINT8ARRAY = vector<uint8_t>;
namespace abicoder {
inline double alignSize(size_t size) {
    return 32 * (ceil(size / 32.0));
}
inline void print_bytes( const UINT8ARRAY &in_bytes )
{
    for (size_t i = 0; i < in_bytes.size(); i++)
    {
       printf("%02X", in_bytes[i]);
    }
    printf("\n");
}
void insert(UINT8ARRAY &coder,const UINT8ARRAY &input, size_t offset = 0);

// template <size_t N>
inline void to_array(UINT8ARRAY& result, const UINT8ARRAY &value, size_t offset = 0, bool signal = true) {
    if(signal) {
        for(size_t i=offset, x=0; x<value.size()  ; i++, x++) {
            result.at(i) = value.at(x);
        }
        return;
    } 
    for(size_t i=offset, x = value.size()-1; i<result.size() && x >=0 ; i++, x--) {
        result.at(i) = value.at(x);
    }
    
}
// template <size_t N>
inline void to_array(UINT8ARRAY& result, const uint8_t &value, size_t offset) {
    result.at(offset) = value;
}

UINT8ARRAY to_bytes(const std::string& _s, size_t offset = 0, bool boolean = true);

UINT8ARRAY fixed_to_bytes(const std::string &_s);

UINT8ARRAY string_to_bytes(const std::string& _s);

class Coder {
public:
    virtual UINT8ARRAY encode() = 0;
    virtual void setValue(const ByteData & _value) = 0;
    virtual bool getDynamic() const = 0;
    virtual ~Coder() {}
};


class UintNumber {
public:
    UintNumber() {}
    UintNumber(size_t _size, bool _signal) :
        size(_size),
        signal(_signal)
    {
        name = (signal ? "int" : "uint") + to_string(size * 8);
    }

    UINT8ARRAY encode(const ByteData& value) {
        return encode(eevm::to_uint256(value));
    }

    UINT8ARRAY encode(uint256 value) {
        UINT8ARRAY result(32);
        vector<uint8_t> dest_hash(32);
        std::memcpy(&dest_hash[0], &value, 32);
        to_array(result, dest_hash, 0, false);
        return result;
    }

    UINT8ARRAY encode(bool boolean) {
        UINT8ARRAY result(32);
        to_array(result, (uint8_t)boolean, 31);
        return result;
    }


private:
    ByteData name;
    size_t size;
    bool signal;
};

class CoderNumber : public Coder {
private:
    ByteData    name;
    size_t      size;
    ByteData value;
    bool        Dynamic = false;
    bool        signal;
public:
    CoderNumber() {}
    CoderNumber(size_t _size, bool _signal) :
        size(_size),
        signal(_signal)
    {
        name = (signal ? "int" : "uint") + to_string(size * 8);
    }

    void setValue(const ByteData &_value) {
        value = _value;
    }

    UINT8ARRAY encode() {
        return UintNumber().encode(value);  
    }
    bool getDynamic() const {
        return Dynamic;
    }


};


class CoderAddress : public Coder {
public:
    CoderAddress(ByteData _name) :name(_name) {}
    void setValue(const ByteData &_value) {
        value = _value;
    }
    UINT8ARRAY encode() {        
        UINT8ARRAY result = to_bytes(value, 12u);
        return result;       
    }
    bool getDynamic() const {
        return Dynamic;
    }

private:
    ByteData name;
    ByteData coderType = "address";
    ByteData value;
    bool     Dynamic = false;

};

class CoderBoolean : public Coder {
public:
    CoderBoolean(ByteData _name) :name(_name) {
        uint256CoderBool =  UintNumber(32, true);
    }
    void setValue(const ByteData &_value) {
        value = _value;
    }
    UINT8ARRAY encode() {
        return uint256CoderBool.encode(value == "1" ? true : false);
    }

    bool getDynamic() const {
        return Dynamic;
    }

private:
    ByteData name;
    ByteData coderType = "bool";
    ByteData value;
    bool     Dynamic = false;
    UintNumber uint256CoderBool;
};


UINT8ARRAY encodeDynamicBytes(const UINT8ARRAY& value);


class CoderString : public Coder {
public:
    CoderString(ByteData _name, bool _dynamic = true) :name(_name), Dynamic(_dynamic) {}
    void setValue(const ByteData &_value) {
        value = _value;
    }
    UINT8ARRAY encode() {
        UINT8ARRAY result = string_to_bytes(value);
        return encodeDynamicBytes(result);
    }

    bool getDynamic() const {
        return Dynamic;
    }

private:
    ByteData name;
    ByteData coderType = "string";
    ByteData value;
    bool     Dynamic = false;

};

class CoderDynamicBytes : public Coder {
public:
    CoderDynamicBytes() {}
    CoderDynamicBytes(ByteData _name, bool _dynamic = true) :name(_name), Dynamic(_dynamic) {}
    void setValue(const ByteData &_value) {
        value = _value;
    }

    UINT8ARRAY encode() {
        UINT8ARRAY result = to_bytes(value, 0, false);
        return encodeDynamicBytes(result);
    }

    bool getDynamic() const {
        return Dynamic;
    }

private:
    ByteData name;
    ByteData coderType = "bytes";
    ByteData value;
    bool     Dynamic = false;

};


class CoderFixedBytes : public Coder {
public:
    CoderFixedBytes() {}
    CoderFixedBytes(size_t _length) : length(_length) {
        name = "bytes" + to_string(length);
    }
    void setValue(const ByteData &_value) {
        value = _value;
    }

    UINT8ARRAY encode() {
        auto s = eevm::strip(value);
        if(s.size() != length) throw errors::make_length_error(name, length, s.size());
        UINT8ARRAY result = fixed_to_bytes(s); 
        return result;           
    }

    bool getDynamic() const {
        return Dynamic;
    }

private:
    ByteData name;
    bool     Dynamic = false;
    ByteData value;
    size_t      length;
};

struct PackParams
{
    bool Dynamic;
    UINT8ARRAY data;
};

UINT8ARRAY uint256Coder(size_t size);

UINT8ARRAY basic_pack(const vector<PackParams>& parts);
UINT8ARRAY pack(const std::vector<void*>& coders);

UINT8ARRAY pack(const std::vector<void*>& coders, const vector<ByteData> &value);

class CoderArray : Coder{
public:
    // CoderArray( ByteData _type, int _length, bool dynamic = true) : length(_length) {
    //     // type = _type + "[" + (length >=0 ? to_string(length) : "") + "]";
    //     type = _type;
    //     Dynamic =  dynamic;
    //     // coder = _coder;
    // }

    CoderArray(ByteData _name, ByteData _type, int _length, bool dynamic = true) : length(_length), name(_name) {
        type = _type;
        Dynamic =  dynamic;
    }

    void setValue(const vector<ByteData> &_value){
        value = _value;
    }

    void setValue(const ByteData &_value){ 
        value = Utils::stringToArray(_value);
        length = length > 1 ? length : value.size();
    }

    UINT8ARRAY encode();

    bool getDynamic() const {
        return Dynamic;
    }

private:
    size_t length;
    bool   Dynamic = false;
    ByteData type;
    ByteData name;
    vector<void*> coders;
    vector<ByteData> value;
};

enum Type  {
      ADDRESS,
      UINT,
      INT,
      BYTES,
      STRING,
      BOOL
};
static std::unordered_map<ByteData, int> contractType = {
      {"string", STRING},
      {"bytes", BYTES},
      {"bool", BOOL},
      {"address", ADDRESS},
      {"uint", UINT},
      {"int", INT},
};
void paramCoder(vector<void*> &coders, const ByteData &name, const ByteData &_type,const vector<ByteData> & value);
void paramCoder(vector<void*> &coders, const ByteData &name, const ByteData &type,const ByteData & value, int length);
void paramCoder(vector<void*> &coders, const ByteData &name, const ByteData &_type,const ByteData & value);

std::vector<std::string> decode_uint256_array(const std::vector<uint8_t>& states);
std::string decode_string(const std::vector<uint8_t> &data);
std::vector<std::string> decode_string_array(const std::vector<uint8_t> &data);

}

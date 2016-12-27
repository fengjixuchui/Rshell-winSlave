#pragma once
std::string base64Encode(unsigned char const*, unsigned int len);
std::vector<unsigned char> base64Decode(std::string const& encoded_string);

char* vecCharToChar(std::vector<unsigned char> vChar);
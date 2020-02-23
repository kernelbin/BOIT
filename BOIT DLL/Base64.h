#pragma once

int Base64Encode(const unsigned char* sourcedata, char* base64);
int Base64Decode(const char* base64, unsigned char* bindata);
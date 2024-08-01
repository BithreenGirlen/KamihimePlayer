#ifndef JSON_MINIMAL_H_
#define JSON_MINIMAL_H_

namespace json_minimal
{
	bool ExtractJsonObject(char** src, const char* name, char** dst);
	bool ExtractJsonArray(char** src, const char* name, char** dst);
	bool GetJsonElementValue(char* src, const char* name, char* dst, size_t nDstSize);

	bool ReadNextKey(char** src, char* key, size_t nKeySize, char* value, size_t nValueSize);
	bool ReadNextArrayValue(char** src, char* dst, size_t nDstSize);
}

#endif //JSON_MINIMAL_H_
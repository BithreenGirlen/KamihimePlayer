/*Minimal JSON extractor.*/

#include <string.h>
#include <malloc.h>

#include "json_minimal.h"

namespace json_minimal
{

/*JSON�����̂̒��o*/
bool ExtractJsonObject(char** src, const char* name, char** dst)
{
	char* p = nullptr;
	char* pp = *src;
	char* q = nullptr;
	char* qq = nullptr;
	size_t nLen = 0;
	int iCount = 0;

	if (name != nullptr)
	{
		p = strstr(pp, name);
		if (p == nullptr)return false;

		pp = strchr(p, ':');
		if (pp == nullptr)return false;
	}
	else
	{
		p = strchr(pp, '{');
		if (p == nullptr)return false;
		++iCount;
		pp = p + 1;
	}

	for (;;)
	{
		q = strchr(pp, '}');
		if (q == nullptr)return false;

		qq = strchr(pp, '{');
		if (qq == nullptr)break;

		if (q < qq)
		{
			--iCount;
			pp = q + 1;
		}
		else
		{
			++iCount;
			pp = qq + 1;
		}

		if (iCount == 0)break;
	}

	for (; iCount > 0; ++q)
	{
		if (*q == '}')
		{
			--iCount;
		}
	}

	nLen = q - p + 1;
	char* pBuffer = static_cast<char*>(malloc(nLen + 1));
	if (pBuffer == nullptr)return false;

	memcpy(pBuffer, p, nLen);
	*(pBuffer + nLen) = '\0';
	*dst = pBuffer;
	*src = q;

	return true;
}
/*JSON�z��̒��o*/
bool ExtractJsonArray(char** src, const char* name, char** dst)
{
	char* p = nullptr;
	char* pp = *src;
	char* q = nullptr;
	char* qq = nullptr;
	size_t nLen = 0;
	int iCount = 0;

	if (name != nullptr)
	{
		p = strstr(pp, name);
		if (p == nullptr)return false;

		pp = strchr(p, ':');
		if (pp == nullptr)return false;
	}
	else
	{
		p = strchr(pp, '[');
		if (p == nullptr)return false;
		++iCount;
		pp = p + 1;
	}

	for (;;)
	{
		q = strchr(pp, ']');
		if (q == nullptr)return false;

		qq = strchr(pp, '[');
		if (qq == nullptr)break;

		if (q < qq)
		{
			--iCount;
			pp = q + 1;
		}
		else
		{
			++iCount;
			pp = qq + 1;
		}

		if (iCount == 0)break;
	}

	for (; iCount > 0; ++q)
	{
		if (*q == ']')
		{
			--iCount;
		}
	}

	nLen = q - p + 1;
	char* pBuffer = static_cast<char*>(malloc(nLen + 1));
	if (pBuffer == nullptr)return false;

	memcpy(pBuffer, p, nLen);
	*(pBuffer + nLen) = '\0';
	*dst = pBuffer;
	*src = q;

	return true;
}
/*JSON��؂�ʒu�T��*/
char* FindJsonValueEnd(char* src)
{
	const char ref[] = ",}\"]";
	return strpbrk(src, ref);
}
/*JSON�v�f�̒l���擾*/
bool GetJsonElementValue(char* src, const char* name, char* dst, size_t nDstSize)
{
	char* p = nullptr;
	char* pp = src;
	size_t nLen = 0;

	p = strstr(pp, name);
	if (p == nullptr)return false;

	pp = strchr(p, ':');
	if (pp == nullptr)return false;
	++pp;

	p = FindJsonValueEnd(pp);
	if (p == nullptr)return false;
	if (*p == '"')
	{
		pp = p + 1;
		p = strchr(pp, '"');
		if (p == nullptr)return false;
	}

	nLen = p - pp;
	if (nLen > nDstSize)return false;
	memcpy(dst, pp, nLen);
	*(dst + nLen) = '\0';

	return true;
}
/*JSON�ϐ����J�n�ʒu�T��*/
char* FindJsonNameStart(char* src)
{
	const char ref[] = " :{[,";
	for (char* p = src; p != nullptr; ++p)
	{
		bool b = false;
		/*�I�[���O*/
		for (size_t i = 0; i < sizeof(ref) - 1; ++i)
		{
			if (*p == ref[i])
			{
				b = true;
			}
		}
		if (!b)return p;
	}

	return nullptr;
}
/*JSON�Ηv�f�ǂݎ��*/
bool ReadNextKey(char** src, char* key, size_t nKeySize, char* value, size_t nValueSize)
{
	char* p = nullptr;
	char* pp = *src;
	size_t nLen = 0;

	p = FindJsonNameStart(pp);
	if (p == nullptr)return false;
	if (*p == '"')
	{
		++p;
		pp = strchr(p, '"');
		if (pp == nullptr)return false;
	}
	else
	{
		pp = strchr(p, ':');
		if (pp == nullptr)return false;
	}

	nLen = pp - p;
	if (nLen > nKeySize)return false;
	memcpy(key, p, nLen);
	*(key + nLen) = '\0';

	++pp;
	p = FindJsonValueEnd(pp);
	if (p == nullptr)return false;
	if (*p == '"')
	{
		pp = p + 1;
		p = strchr(pp, '"');
		if (p == nullptr)return false;
	}

	nLen = p - pp;
	if (nLen > nValueSize)return false;
	memcpy(value, pp, nLen);
	*(value + nLen) = '\0';
	*src = p + 1;

	return true;
}
/*���̔z��v�f�ǂݎ��*/
bool ReadNextArrayValue(char** src, char* dst, size_t nDstSize)
{
	char* p = nullptr;
	char* pp = *src;
	size_t nLen = 0;

	p = FindJsonNameStart(pp);
	if (p == nullptr)return false;
	if (*p == '"')
	{
		++p;
		pp = strchr(p, '"');
		if (pp == nullptr)return false;
	}
	else if (*p == ']' || *p == '\0')
	{
		return false;
	}
	else
	{
		pp = FindJsonValueEnd(p);
		if (pp == nullptr)return false;
	}

	nLen = pp - p;
	if (nLen > nDstSize)return false;
	memcpy(dst, p, nLen);
	*(dst + nLen) = '\0';
	*src = *pp == '"' ? pp + 1: pp;

	return true;
}

} // namespace json_minimal
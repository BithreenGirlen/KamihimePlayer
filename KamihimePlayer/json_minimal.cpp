/*Minimal JSON extractor.*/

#include <string.h>
#include <malloc.h>

#include "json_minimal.h"


/*JSON�W���v�f�I�[�T��*/
static bool FindCollectionEnd(char* src, char** dst, int* pForeCount, bool bObject)
{
	if (src == nullptr || dst == nullptr)return false;
	int iCount = pForeCount == nullptr ? 0 : *pForeCount;

	const char cStart = bObject ? '{' : '[';
	const char cEnd = bObject ? '}' : ']';

	char* p = src;
	char* pEnd = nullptr;
	char* pStart = nullptr;

	for (;;)
	{
		pEnd = strchr(p, cEnd);
		if (pEnd == nullptr)return false;

		pStart = strchr(p, cStart);
		if (pStart == nullptr)break;

		if (pEnd < pStart)
		{
			--iCount;
			p = pEnd + 1;
		}
		else
		{
			++iCount;
			p = pStart + 1;
		}

		if (iCount == 0)break;
	}

	for (; iCount > 0; ++pEnd)
	{
		if (*pEnd == cEnd)
		{
			--iCount;
		}
	}

	*dst = ++pEnd;

	return true;
}

/*JSON�ϐ����J�n�ʒu�T��*/
static char* FindJsonNameStart(char* src)
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
/*JSON�l�J�n�ʒu�T��*/
static char* FindJsonValueStart(char* src)
{
	const char ref[] = "\"{[0123456789-";
	return strpbrk(src, ref);
}
/*JSON��؂�ʒu�T��*/
static char* FindJsonValueEnd(char* src)
{
	const char ref[] = ",}\"]";
	return strpbrk(src, ref);
}


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

	bool bRet = FindCollectionEnd(pp, &q, &iCount, true);
	if (!bRet)return false;

	nLen = q - p;
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

	bool bRet = FindCollectionEnd(pp, &q, &iCount, false);
	if (!bRet)return false;

	nLen = q - p;
	char* pBuffer = static_cast<char*>(malloc(nLen + 1));
	if (pBuffer == nullptr)return false;

	memcpy(pBuffer, p, nLen);
	*(pBuffer + nLen) = '\0';
	*dst = pBuffer;
	*src = q;

	return true;
}
/*JSON�v�f�̒l���擾*/
bool GetJsonElementValue(char* src, const char* name, char* dst, size_t nDstSize, int* iDepth, char** pEnd)
{
	char* p = nullptr;
	char* pp = src;
	size_t nLen = 0;

	p = strstr(pp, name);
	if (p == nullptr)return false;

	pp = strchr(p, ':');
	if (pp == nullptr)return false;
	++pp;

	p = FindJsonValueStart(pp);
	if (p == nullptr)return false;
	if (*p == '[' || *p == '{') /* �W���v�f */
	{
		int iCount = 0;
		bool bRet = FindCollectionEnd(pp, &p, &iCount, *p == '{');
		if (!bRet)return false;
	}
	else /* �P�v�f */
	{
		p = FindJsonValueEnd(pp);
		if (p == nullptr)return false;
		if (*p == '"')
		{
			pp = p + 1;
			p = strchr(pp, '"');
			if (p == nullptr)return false;
		}
		else
		{
			for (; *pp == ' '; ++pp);
			for (char* q = p - 1;; --q)
			{
				if (*q != ' ' && *q != '\r' && *q != '\n' && *q != '\t')break;
				p = q;
			}
		}
	}

	nLen = p - pp;
	if (nLen > nDstSize - 1)return false;
	memcpy(dst, pp, nLen);
	*(dst + nLen) = '\0';

	/*����q�̗v�f�ł����iDepth > 0*/
	if (iDepth != nullptr && *pEnd != nullptr)
	{
		*pEnd = p + 1;

		char* q = nullptr;
		char* qq = nullptr;
		pp = src;

		for (;;)
		{
			q = strchr(pp, '}');
			if (q == nullptr)break;

			qq = strchr(pp, '{');
			if (qq == nullptr)break;

			if (q < qq)
			{
				--(*iDepth);
				pp = q + 1;
			}
			else
			{
				++(*iDepth);
				pp = qq + 1;
			}

			if (pp > p)break;
		}
	}

	return true;
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
	if (nLen > nKeySize - 1)return false;
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
	if (nLen > nValueSize - 1)return false;
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
	if (nLen > nDstSize - 1)return false;
	memcpy(dst, p, nLen);
	*(dst + nLen) = '\0';
	*src = *pp == '"' ? pp + 1: pp;

	return true;
}
/*���̏I���ʒu�܂œǂݐi��*/
bool ReadUpToNameEnd(char** src, const char* name, char* value, size_t nValueSize)
{
	char* p = nullptr;
	char* pp = *src;

	if (name != nullptr)
	{
		p = strstr(pp, name);
		if (p == nullptr)return false;
	}
	else
	{
		p = FindJsonNameStart(pp);
		if (p == nullptr)return false;
		++p;
	}

	pp = FindJsonValueEnd(p);
	if (pp == nullptr)return false;

	/*���̎擾*/
	if (name == nullptr && value != nullptr && nValueSize != 0)
	{
		size_t nLen = pp - p;
		if (nLen > nValueSize - 1)return false;
		memcpy(value, p, nLen);
		*(value + nLen) = '\0';
	}

	*src = *pp == '"' ? pp + 1 : pp;

	return true;
}

} // namespace json_minimal

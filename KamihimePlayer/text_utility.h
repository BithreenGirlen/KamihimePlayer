#ifndef TEXT_UTILITY_H_
#define TEXT_UTILITY_H_

#include <string>
#include <vector>

namespace text_utility
{
	template <typename CharType>
	void TextToLines(const std::basic_string<CharType>& text, std::vector<std::basic_string<CharType>>& lines)
	{
		std::basic_string<CharType> temp{};
		for (auto& c : text)
		{
			if (c == CharType('\r') || c == CharType('\n'))
			{
				if (!temp.empty())
				{
					lines.push_back(temp);
					temp.clear();
				}
				continue;
			}
			temp.push_back(c);
		}

		if (!temp.empty())
		{
			lines.push_back(temp);
		}
	}

	template <typename CharType>
	void SplitTextBySeparator(const std::basic_string<CharType>& text, const CharType separator, std::vector<std::basic_string<CharType>>& splits)
	{
		for (size_t nRead = 0; nRead < text.size();)
		{
			size_t nPos = text.find(separator, nRead);
			if (nPos == std::basic_string<CharType>::npos)
			{
				size_t nLen = text.size() - nRead;
				splits.emplace_back(text.substr(nRead, nLen));
				break;
			}

			size_t nLen = nPos - nRead;
			splits.emplace_back(text.substr(nRead, nLen));
			nRead += nLen + 1;
		}
	}

	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const std::basic_string<CharType>& strOld, const std::basic_string<CharType>& strNew)
	{
		if (strOld.empty() || strOld == strNew) return;

		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(strOld, nRead);
			if (nPos == std::basic_string<CharType>::npos) break;
			src.replace(nPos, strOld.size(), strNew);
			nRead = nPos + strNew.size();
		}
	}
	template <typename CharType>
	void ReplaceAll(std::basic_string<CharType>& src, const CharType* strOld, const CharType* strNew)
	{
		ReplaceAll(src, std::basic_string<CharType>(strOld), std::basic_string<CharType>(strNew));
	}
}

#endif // !TEXT_UTILITY_H_

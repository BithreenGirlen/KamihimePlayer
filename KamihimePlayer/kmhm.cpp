
#include <memory>

#include "kmhm.h"
#include "json_minimal.h"
#include "win_text.h"

bool kmhm::LoadScenarioFile(const std::string &scenarioText, std::vector<adv::TextDatum>& textData, std::vector<std::vector<std::wstring>> &imageFileNamesList)
{
	std::vector<std::string> commands;
	char* p = const_cast<char*>(&scenarioText[0]);
	for (;;)
	{
		auto pp = std::make_unique<char*>();
		bool bRet = json_minimal::ExtractJsonObject(&p, nullptr, &*pp);
		if (!bRet)break;
		commands.push_back(*pp);
	}

	std::vector<std::string> talks;
	/*音声・文章*/
	for (auto &command : commands)
	{
		p = &command[0];
		auto pp1 = std::make_unique<char*>();
		bool bRet = json_minimal::ExtractJsonArray(&p, "talk", &*pp1);
		if (!bRet)continue;

		p = *pp1;
		for (;;)
		{
			auto pp2 = std::make_unique<char*>();
			bRet = json_minimal::ExtractJsonObject(&p, nullptr, &*pp2);
			if (!bRet)break;
			talks.push_back(*pp2);
		}
	}

	std::vector<char> vBuffer(512, '\0');
	for (auto& talk : talks)
	{
		adv::TextDatum textDatum;

		bool bRet = json_minimal::GetJsonElementValue(&talk[0], "chara", vBuffer.data(), vBuffer.size());
		if (bRet && vBuffer.front() != '\0')
		{
			textDatum.wstrText = win_text::WidenUtf8(vBuffer.data());
			textDatum.wstrText += L": ";
		}

		bRet = json_minimal::GetJsonElementValue(&talk[0], "words", vBuffer.data(), vBuffer.size());
		if (!bRet)continue;

		textDatum.wstrText += win_text::WidenUtf8(vBuffer.data());
		if (textDatum.wstrText.empty())continue;

		bRet = json_minimal::GetJsonElementValue(&talk[0], "voice", vBuffer.data(), vBuffer.size());
		if (bRet)
		{
			/*階層は呼び出し側で連結する。*/
			textDatum.wstrVoicePath = win_text::WidenUtf8(vBuffer.data());
		}
		textData.push_back(textDatum);
	}

	const auto ReplaceAll = [](std::wstring& src, const std::wstring& strOld, const std::wstring& strNew)
		-> void
		{
			if (strOld == strNew)return;

			for (size_t nRead = 0;;)
			{
				size_t nPos = src.find(strOld, nRead);
				if (nPos == std::wstring::npos)break;
				src.replace(nPos, strOld.size(), strNew);
				nRead = nPos + strNew.size();
			}
		};

	for (auto& textDatum : textData)
	{
		ReplaceAll(textDatum.wstrText, L"{{主人公}}", L"マスター");
	}

	/*画像*/
	for (auto& command : commands)
	{
		bool bRet = json_minimal::GetJsonElementValue(&command[0], "film", vBuffer.data(), vBuffer.size());
		if (!bRet)continue;

		std::vector<std::wstring> imageFileNames;
		/*新規場面*/
		if (vBuffer[0] == '[')
		{
			p = &vBuffer[0];
			for (;;)
			{
				bRet = json_minimal::ReadNextArrayValue(&p, vBuffer.data(), vBuffer.size());
				if (!bRet)break;

				/*
				* There is a parameter to specify loop count, which makes eye blinking natural.
				* But here, neglect this one for simplicity.
				*/
				if (strlen(vBuffer.data()) > 1)
				{
					imageFileNames.push_back(win_text::WidenUtf8(vBuffer.data()));
				}
			}
		}
		else
		{
			imageFileNames.push_back(win_text::WidenUtf8(vBuffer.data()));
		}

		if (!imageFileNames.empty())
		{
			if (strstr(vBuffer.data(), "black.jpg") != nullptr ||
				strstr(vBuffer.data(), "pink_s.jpg") != nullptr)
			{
				continue;
			}

			imageFileNamesList.push_back(imageFileNames);
		}
	}
	return !textData.empty();
}

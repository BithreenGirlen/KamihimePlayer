
#include <memory>

#include "kmhm.h"

#include "json_minimal.h"
#include "win_filesystem.h"
#include "win_text.h"

/* 内部用 */
namespace kmhm
{
	struct TalkDatum
	{
		std::string chara;
		std::string words;
		std::string voice;
	};

	struct CommandDatum
	{
		std::vector<std::string> filmes;
		std::vector<TalkDatum> talks;
	};

	/* 台本解析 */
	static void ParseScenarioFile(const std::string& scenarioText, std::vector<CommandDatum>& commandData)
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

		std::vector<char> vBuffer(512, '\0');

		for (auto& command : commands)
		{
			CommandDatum commandDatum;

			bool bRet = json_minimal::GetJsonElementValue(&command[0], "film", vBuffer.data(), vBuffer.size());
			if (!bRet)continue;

			/* 新規場面 */
			if (vBuffer[0] == '[')
			{
				p = &vBuffer[0];
				for (;;)
				{
					bRet = json_minimal::ReadNextArrayValue(&p, vBuffer.data(), vBuffer.size());
					if (!bRet)break;

					/*
					* There is a parameter to specify loop count, which helps to make eye blinking less frequent.
					* But judging from its trivial benefits and complexity, ignore this one for simplicity.
					*/
					if (strlen(vBuffer.data()) > 1)
					{
						commandDatum.filmes.push_back(vBuffer.data());
					}
				}
			}
			else
			{
				if (strstr(vBuffer.data(), "black.jpg") == nullptr && strstr(vBuffer.data(), "pink_s.jpg") == nullptr)
				{
					commandDatum.filmes.push_back(vBuffer.data());
				}
			}

			p = &command[0];
			auto pp1 = std::make_unique<char*>();
			bRet = json_minimal::ExtractJsonArray(&p, "talk", &*pp1);
			if (!bRet)continue;
			p = *pp1;

			std::vector<std::string> talks;
			for (;;)
			{
				auto pp2 = std::make_unique<char*>();
				bRet = json_minimal::ExtractJsonObject(&p, nullptr, &*pp2);
				if (!bRet)break;
				talks.push_back(*pp2);
			}

			for (auto& talk : talks)
			{
				TalkDatum talkDatum;

				bRet = json_minimal::GetJsonElementValue(&talk[0], "chara", vBuffer.data(), vBuffer.size());
				if (bRet && vBuffer.front() != '\0')
				{
					talkDatum.chara = vBuffer.data();
				}
				bRet = json_minimal::GetJsonElementValue(&talk[0], "words", vBuffer.data(), vBuffer.size());
				if (bRet && vBuffer.front() != '\0')
				{
					talkDatum.words = vBuffer.data();
				}
				bRet = json_minimal::GetJsonElementValue(&talk[0], "voice", vBuffer.data(), vBuffer.size());
				if (bRet && vBuffer.front() != '\0')
				{
					talkDatum.voice = vBuffer.data();
				}

				commandDatum.talks.push_back(std::move(talkDatum));
			}

			commandData.push_back(std::move(commandDatum));
		}
	}

	static void ReplaceAll(std::wstring& src, const std::wstring& strOld, const std::wstring& strNew)
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
} /* namespace kmhm */

/* 台本読み取り */
bool kmhm::ReadScenario(const std::wstring& wstrFolderPath, std::vector<adv::TextDatum>& textData, std::vector<std::vector<std::wstring>>& imageFileNamesList, std::vector<adv::SceneDatum>& sceneData, std::vector<adv::LabelDatum>& labelData)
{
	std::vector<std::wstring> textFile;
	win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".json", textFile);
	if (!textFile.empty())
	{
		std::string scenarioText = win_filesystem::LoadFileAsString(textFile[0].c_str());

		std::vector<CommandDatum> commandData;
		ParseScenarioFile(scenarioText, commandData);

		adv::SceneDatum sceneDatumBuffer;

		for (const auto& commandDatum : commandData)
		{
			std::wstring labelBuffer;
			/* 画像 */
			if (!commandDatum.filmes.empty())
			{
				std::vector<std::wstring> imageFilePaths;
				for (const auto& fileName : commandDatum.filmes)
				{
					imageFilePaths.emplace_back(wstrFolderPath + L"\\" + win_text::WidenUtf8(fileName));
				}

				imageFileNamesList.push_back(std::move(imageFilePaths));
				sceneDatumBuffer.nImageIndex = imageFileNamesList.size() - 1;
				/* 切り替わり場面名称 */
				std::wstring wstrFileName = win_text::WidenUtf8(commandDatum.filmes[0]);
				size_t nPos = wstrFileName.find(L'.');
				if (nPos != std::wstring::npos)
				{
					labelBuffer = wstrFileName.substr(0, nPos);
				}
			}
			/* 人物名・文章・音声 */
			for (const auto& talk : commandDatum.talks)
			{
				/* 暗転場面 */
				if (talk.words.empty())
				{
					/* 画像に対応する文章が存在しないので、前文章の紐付けを上書き */
					if (!sceneData.empty() && !imageFileNamesList.empty())
					{
						sceneData.back().nImageIndex = imageFileNamesList.size() - 1;
					}

					if (!labelBuffer.empty())
					{
						labelData.emplace_back(adv::LabelDatum{ labelBuffer, sceneData.size() - 1 });
						labelBuffer.clear();
					}

					continue;
				}

				adv::TextDatum textDatum;
				if (!talk.chara.empty())
				{
					textDatum.wstrText = win_text::WidenUtf8(talk.chara);
					textDatum.wstrText += L":";
				}

				textDatum.wstrText += L" \n";
				textDatum.wstrText += win_text::WidenUtf8(talk.words);

				if (!talk.voice.empty())
				{
					textDatum.wstrVoicePath = wstrFolderPath + L"\\" + win_text::WidenUtf8(talk.voice);
				}

				textData.push_back(std::move(textDatum));

				sceneDatumBuffer.nTextIndex = textData.size() - 1;
				sceneData.push_back(sceneDatumBuffer);

				if (!labelBuffer.empty())
				{
					labelData.emplace_back(adv::LabelDatum{ labelBuffer, sceneData.size() - 1 });
					labelBuffer.clear();
				}
			}
		}
	}

	if (textData.empty())
	{
		/* 台本ファイルなし・解析失敗 */
		std::vector<std::wstring> audioFilePaths;
		win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".mp3", audioFilePaths);

		for (const auto& voicePath : audioFilePaths)
		{
			textData.emplace_back(adv::TextDatum{ L"", voicePath });
		}

		std::vector<std::wstring> imageFilePaths;
		bool bRet = win_filesystem::CreateFilePathList(wstrFolderPath.c_str(), L".jpg", imageFilePaths);
		for (const auto& imageFilePath : imageFilePaths)
		{
			std::vector<std::wstring> vBuffer;
			vBuffer.push_back(imageFilePath);
			imageFileNamesList.push_back(std::move(vBuffer));
		}
	}

	for (auto& textDatum : textData)
	{
		ReplaceAll(textDatum.wstrText, L"{{主人公}}", L"マスター");
	}

	return true;
}

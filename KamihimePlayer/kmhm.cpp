
#include <memory>

#include "kmhm.h"
#include "json_minimal.h"
#include "win_text.h"

bool kmhm::LoadScenarioFile(const std::string &scenarioText, std::vector<adv::TextDatum>& textData)
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

    return !textData.empty();
}

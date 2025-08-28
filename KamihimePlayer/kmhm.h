#ifndef KMHM_H_
#define KMHM_H_

#include <string>
#include <vector>

#include "adv.h"

namespace kmhm
{
	bool ReadScenario(
		const std::wstring& wstrFolderPath,
		std::vector<adv::TextDatum>& textData,
		std::vector<std::vector<std::wstring>>& imageFileNamesList,
		std::vector<adv::SceneDatum>& sceneData,
		std::vector<adv::LabelDatum>& labelData);
}
#endif // !KMHM_H_

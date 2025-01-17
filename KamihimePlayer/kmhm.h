#ifndef KMHM_H_
#define KMHM_H_

#include <string>
#include <vector>

#include "adv.h"

namespace kmhm
{
	bool LoadScenarioFile(const std::string &scenarioText, std::vector<adv::TextDatum>& textData, std::vector<std::vector<std::wstring>> &imageFileNamesList);
}
#endif // !KMHM_H_

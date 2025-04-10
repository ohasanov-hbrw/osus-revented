#include "settingsParser.hpp"
#include "globals.hpp"

std::pair<std::string, std::string> parseLine(std::string line, bool hasSpaceBefore, bool hasSpaceAfter){
	size_t spliceLocation;
	for(spliceLocation = 0; spliceLocation < line.size(); spliceLocation++)
		if(line[spliceLocation] == ':')
			break;
	return make_pair(line.substr(0, spliceLocation - hasSpaceBefore), line.substr(spliceLocation + hasSpaceAfter + 1, line.size() - 1 - spliceLocation - hasSpaceAfter));
}

void parseSettings(){
	std::ifstream ifs(Global.GamePath + "/resources/settings.ini");
	std::string line;
	if (ifs.is_open()){
        std::cout << "opened.\n";
		while(std::getline(ifs, line)){
            std::cout << line << std::endl;
			if(line[line.size()-1] == 13)
				line.pop_back();
			if(line[0] == '[' and line[line.size()-1] == ']'){
				if(line == "[Settings]"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;
						std::pair<std::string, std::string> keyValue = parseLine(subLine, false, false);
						if(keyValue.first == "circleSector"){
                            Global.circleSector = std::stoi(keyValue.second);
                            std::cout << "settings circleSector to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundR"){
                            Global.Background.r = std::stoi(keyValue.second);
                            std::cout << "settings BackgroundR to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundG"){
                            Global.Background.g = std::stoi(keyValue.second);
                            std::cout << "settings BackgroundG to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundB"){
                            Global.Background.b = std::stoi(keyValue.second);
                            std::cout << "settings BackgroundB to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundA"){
                            Global.Background.a = std::stoi(keyValue.second);
                            std::cout << "settings BackgroundA to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "skinNumberOverlap"){
                            Global.skinNumberOverlap = std::stoi(keyValue.second);
                            std::cout << "settings skinNumberOverlap to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "FPS"){
                            Global.FPS = std::stof(keyValue.second);
                            std::cout << "settings FPS to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "TPS"){
                            Global.TPS = std::stoi(keyValue.second);
                            std::cout << "settings TPS to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "offset"){
                            Global.offset = std::stof(keyValue.second);
                            std::cout << "settings offset to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "maxSliderSize"){
                            Global.maxSliderSize = std::stoi(keyValue.second);
                            std::cout << "settings maxSliderSize to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "extraJudgementTime"){
                            Global.extraJudgementTime = std::stof(keyValue.second);
                            std::cout << "settings extraJudgementTime to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "MouseBlur"){
                            Global.MouseBlur = std::stoi(keyValue.second);
                            std::cout << "settings MouseBlur to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "Path"){
                            Global.Path = keyValue.second;
                            std::cout << "settings Path to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "BeatmapLocation"){
                            Global.BeatmapLocation = keyValue.second;
                            std::cout << "settings BeatmapLocation to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "GamePath"){
                            Global.GamePath = keyValue.second;
                            std::cout << "settings GamePath to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "selectedPath"){
                            Global.selectedPath = keyValue.second;
                            std::cout << "settings selectedPath to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "CurrentLocation"){
                            Global.CurrentLocation = keyValue.second;
                            std::cout << "settings CurrentLocation to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "MouseTrailSize"){
                            Global.MouseTrailSize = std::stoi(keyValue.second);
                            std::cout << "settings MouseTrailSize to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "volume"){
                            Global.volume = std::stof(keyValue.second);
                            std::cout << "settings volume to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "hitVolume"){
                            Global.volume = std::stof(keyValue.second);
                            std::cout << "settings volume to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "enableMouse"){
                            Global.enableMouse = (bool)std::stoi(keyValue.second);
                            std::cout << "settings enableMouse to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "sliderTexSize"){
                            Global.sliderTexSize = std::stof(keyValue.second);
                            std::cout << "settings sliderTexSize to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "legacyRender"){
                            Global.legacyRender = (bool)std::stoi(keyValue.second);
                            std::cout << "settings legacyRender to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "useTopScreen"){
                            Global.useTopScreen = (bool)std::stoi(keyValue.second);
                            std::cout << "settings useTopScreen to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "touchScreenTouchEnabled"){
                            Global.touchScreenTouchEnabled = (bool)std::stoi(keyValue.second);
                            std::cout << "settings touchScreenTouchEnabled to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }

					}
					break;
				}
			}
		}
	}
	return;
}


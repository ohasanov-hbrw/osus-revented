#include "settingsParser.hpp"
#include "globals.hpp"
#include <iostream>
#include <fstream>

std::pair<std::string, std::string> parseLine(std::string line, bool hasSpaceBefore, bool hasSpaceAfter){
	size_t spliceLocation;
	for(spliceLocation = 0; spliceLocation < line.size(); spliceLocation++)
		if(line[spliceLocation] == ':')
			break;
	return make_pair(line.substr(0, spliceLocation - hasSpaceBefore), line.substr(spliceLocation + hasSpaceAfter + 1, line.size() - 1 - spliceLocation - hasSpaceAfter));
}

void parseSettings(){
	std::ifstream ifs(Global.GameBinaryPath + "/resources/settings.ini");
	std::string line;
	if (ifs.is_open()){
        std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Settings ini opened.\n";
		while(std::getline(ifs, line)){
            std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Parsing settings" << line << std::endl;
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
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set circleSector to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundR"){
                            Global.Background.r = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set BackgroundR to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundG"){
                            Global.Background.g = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set BackgroundG to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundB"){
                            Global.Background.b = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set BackgroundB to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "BackgroundA"){
                            Global.Background.a = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set BackgroundA to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "skinNumberOverlap"){
                            Global.skinNumberOverlap = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set skinNumberOverlap to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "FPS"){
                            Global.FPS = std::stof(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set FPS to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "TPS"){
                            Global.TPS = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set TPS to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "offset"){
                            Global.offsetTime = (int)std::stof(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set offset to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "maxSliderSize"){
                            Global.maxSliderSize = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set maxSliderSize to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "extraJudgementTime"){
                            Global.extraJudgementTime = std::stof(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set extraJudgementTime to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "MouseBlur"){
                            Global.MouseBlur = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set MouseBlur to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "Path"){
                            Global.Path = keyValue.second;
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set Path to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "BeatmapLocation"){
                            Global.BeatmapLocation = keyValue.second;
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set BeatmapLocation to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "GameBinaryPath"){
                            Global.GameBinaryPath = keyValue.second;
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set GameBinaryPath to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "selectedPath"){
                            Global.selectedPath = keyValue.second;
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set selectedPath to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "CurrentLocation"){
                            Global.CurrentLocation = keyValue.second;
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set CurrentLocation to: " << keyValue.second << std::endl;                      
                        }
                        else if(keyValue.first == "MouseTrailSize"){
                            Global.MouseTrailSize = std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set MouseTrailSize to: " << std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "volume"){
                            Global.volume = std::stof(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set volume to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "hitVolume"){
                            Global.volume = std::stof(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set volume to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "enableMouse"){
                            Global.enableMouse = (bool)std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set enableMouse to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "sliderTexSize"){
                            Global.sliderTexSize = std::stof(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set sliderTexSize to: " << std::stof(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "legacyRender"){
                            Global.legacyRender = (bool)std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set legacyRender to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "useTopScreen"){
                            Global.useTopScreen = (bool)std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set useTopScreen to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }
                        else if(keyValue.first == "touchScreenTouchEnabled"){
                            Global.touchScreenTouchEnabled = (bool)std::stoi(keyValue.second);
                            std::cout << "\e[1;38;5;21m[SET] \e[38;5;236m" << "set touchScreenTouchEnabled to: " << (bool)std::stoi(keyValue.second) << std::endl;                      
                        }

					}
					break;
				}
			}
		}
	}
	return;
}


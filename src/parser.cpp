#include "parser.hpp"
#include "globals.hpp"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <filesystem>



Parser::Parser(){}

// old implementation with ifstream, fopen should be way faster
//this here is just a dumb parser, nothing to document here tbh
/*GameFile Parser::parseMetadata(std::string filename){
	std::ifstream ifs(filename);
	std::string line;
	GameFile gameFile;
	if (ifs.is_open()){
		while(std::getline(ifs, line)){
			if(line[line.size()-1] == 13)
				line.pop_back();
			if(line[0] == '[' and line[line.size()-1] == ']'){
				std::string header = line.substr(1);
				header.pop_back();
				if(header == "Metadata"){
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
						std::pair<std::string, std::string> keyValue = parseKeyValue(subLine, false, false);
						gameFile.configMetadata[keyValue.first] = keyValue.second;
					}
					break;
				}
			}
		}
	}
	else{
		std::cout << "failed to open file" << std::endl;
	}
	return gameFile;
}*/


std::filesystem::path prepare_long_path(const std::string& input_path) {
#ifdef _WIN32
    // If it's already a UNC/WSL path, use the UNC long path prefix
    if (input_path.rfind("\\\\", 0) == 0) { 
        // Checks if path starts with "\\"
        // Converts \\wsl.localhost\... to \\?\UNC\wsl.localhost\...
        return std::filesystem::path("\\\\?\\UNC\\" + input_path.substr(2));
    }
    // If it's a standard local drive path (e.g., C:\)
    else if (input_path.size() >= 3 && input_path[1] == ':' && input_path[2] == '\\') {
        return std::filesystem::path("\\\\?\\" + input_path);
    }
#endif

    // On Linux/macOS, return the path exactly as-is
    return std::filesystem::path(input_path);
}

// Helper to strip trailing CR (\r, ASCII 13) or LF (\n) from fgets buffer
void sanitizeLine(std::string& line) {
    while (!line.empty() && (line.back() == 13 || line.back() == '\n' || line.back() == '\r')) {
        line.pop_back();
    }
}

// Helper function to read an entire line of any length using a fixed buffer chunk
bool readFullLine(FILE* file, std::string& outLine) {
    outLine.clear();
    char buffer[512]; // Small, efficient stack chunk size

    while (fgets(buffer, sizeof(buffer), file) != nullptr) {
        outLine.append(buffer);

        // fgets stops reading when it hits a newline character ('\n') or EOF.
        // If the last character in our string is '\n', we have completed the line!
        if (!outLine.empty() && outLine.back() == '\n') {
            break;
        }
    }

    // Return true if we actually read data, false if we hit EOF right away
    return !outLine.empty();
}

GameFile Parser::parseMetadata(std::string filename) {
    GameFile gameFile;
	FILE* file = fopen(filename.c_str(), "r");
    if (file == nullptr) {
		std::cout << "\e[1;38;5;220m[WARN] \e[38;5;236m"<< "Couldn't open file for parsing, retrying, maybe length? " << filename.size() << std::endl;
		for (int i = 0; i < filename.size(); i++) {
			if (filename[i] == '/') {
				filename[i] = '\\';
			}
		}
		filename = prepare_long_path(filename).string();
		std::cout << filename << std::endl;
		file = fopen(filename.c_str(), "r");
		if (file == nullptr) {
			std::cout << "\e[1;38;5;52m[ERR] \e[38;5;236m"<< "welp... messy solution didnt work" << std::endl;
        	return gameFile; // Could not open file
		}
    }
	std::string line;
    while (readFullLine(file, line)) {
        sanitizeLine(line);
        if (line.empty()) continue;
        if (line.front() == '[' && line.back() == ']') {
            std::string header = line.substr(1);
            header.pop_back(); // Remove ']'
            if (header == "Metadata") {
				std::string subLine;
                while (readFullLine(file, subLine)) {
                    sanitizeLine(subLine);
					if (subLine.empty()) continue;
                    if (subLine.front() == '[' && subLine.back() == ']') {
						break; //break with next header
					}
                    if (subLine.size() >= 2 && subLine[0] == '/' && subLine[1] == '/') {
                        continue;
                    }
					//std::cout << subLine << std::endl;
                    std::pair<std::string, std::string> keyValue = parseKeyValue(subLine, false, false);
                    gameFile.configMetadata[keyValue.first] = keyValue.second;
                }
                break; // Found Metadata block and finished it, exit outer loop
            }
        }
    }

    fclose(file); // Always close your file handle
    return gameFile;
}



//this here is just a dumb parser, nothing to document here tbh
std::string Parser::parseBackground(std::string filename){
	std::ifstream ifs(filename);
	std::string line;
	std::string background = "[]";
	if (ifs.is_open()){
		while(std::getline(ifs, line)){
			if(line[line.size()-1] == 13)
				line.pop_back();
			if(line[0] == '[' and line[line.size()-1] == ']'){
				std::string header = line.substr(1);
				header.pop_back();
				if(header == "Events"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;
						if(subLine[0] == ' ')
							continue;

						Event tempEvent;
						std::vector<std::string> tempVector;
						tempVector = parseSeperatedLists(subLine, ',');

						if(tempVector[0] == "0")
							tempEvent.eventType = 0;
						else
							continue;

						tempEvent.startTime = std::stoi(tempVector[1]);
						if(tempEvent.eventType == 0){
							tempEvent.filename = tempVector[2].substr(1, tempVector[2].size()-2);
							background = tempEvent.filename;
							return background;
						}
					}
				}
			}
		}
	}
	return background;
}


GameFile Parser::parse(std::string filename){
	GameFile gameFile;
	gameFile.configGeneral["AudioLeadIn"] = "0";
	gameFile.configDifficulty["SliderMultiplier"] = "1";
	gameFile.configDifficulty["HPDrainRate"] = "5";
	gameFile.configDifficulty["CircleSize"] = "5";
	gameFile.configDifficulty["OverallDifficulty"] = "5";
	gameFile.configDifficulty["ApproachRate"] = "5";
	gameFile.configDifficulty["SliderTickRate"] = "1";

	int numLines = 0;
	std::ifstream lineCounter(filename);
	std::string unused;
	if (lineCounter.is_open()){
		while ( std::getline(lineCounter, unused) )
			++numLines;
		lineCounter.close();
	}
	//\\wsl.localhost\archlinux\home\nyarch\Projects\osus-revented\beatmaps\1639480 ichigo from KISIDA KYODAN & THE AKEBOSI ROCKETS - STONE OCEAN (TV Size) [no video]
	////wsl.localhost/archlinux/home/nyarch/Projects/osus-revented/beatmaps/1639480 ichigo from KISIDA KYODAN & THE AKEBOSI ROCKETS - STONE OCEAN (TV Size) [no video]/ichigo from KISIDA KYODAN & THE AKEBOSI ROCKETS - STONE OCEAN (TV Size) (Sotarks) [Akitoshi's NORMAL].osu
	//  This thing crashes the game?
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Opening: " << filename << std::endl;
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Ifstream found " << numLines << " lines to parse" << std::endl;
	Global.numberLines = numLines;
    std::ifstream ifs(filename);
	std::string line;
	int lineNumber = 0;
	Global.parsedLines = 0;
	gameFile.hitObjects.shrink_to_fit();
	gameFile.hitObjects.reserve(numLines);
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Parser reserved " << numLines << " HitObjects based on the number of lines" << std::endl;
	if(!ifs.is_open()){
		std::cout << "\e[1;38;5;220m[WARN] \e[38;5;236m"<< "Couldn't open file for parsing, retrying, maybe length? " << filename.size() << std::endl;
		for (int i = 0; i < filename.size(); i++) {
			if (filename[i] == '/') {
				filename[i] = '\\';
			}
		}
		filename = prepare_long_path(filename).string();
		std::cout << filename << std::endl;
		ifs = std::ifstream(filename);
		if (!ifs.is_open()) {
			std::cout << "\e[1;38;5;52m[ERR] \e[38;5;236m"<< "welp... messy solution didnt work" << std::endl;
			return gameFile; // Could not open file
		}
	}


	if (ifs.is_open()){
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Beatmap file opened\n";
		while(std::getline(ifs, line)){
			if(line.size() > 0 and line[line.size()-1] == 13){
				line.pop_back();
				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Popped back of " << line << " - size: " << line.size() << std::endl;
			}
			lineNumber++;
			//std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Parsing line " << lineNumber << " with size " << line.size() << std::endl;
			Global.parsedLines++;
			if(line.size() > 0 and line[0] == '[' and line[line.size()-1] == ']'){
				std::string header = line.substr(1);
				header.pop_back();
				if(header == "General"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						lineNumber++;
						//std::cout << "Parsing line: " << lineNumber << " with the length: " << subLine.size() << " -> " << subLine << std::endl;
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;

						std::pair<std::string, std::string> keyValue = parseKeyValue(subLine, false, true);
						gameFile.configGeneral[keyValue.first] = keyValue.second;
					}
				}
				else if(header == "Editor"){
					std::string subLine;
                    while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
                        if(subLine[subLine.size()-1] == 13){
                            subLine.pop_back();
							lineNumber++;
							//std::cout << "Parsing line: " << lineNumber << " with the length: " << subLine.size() << " -> " << subLine << std::endl;
                            if(subLine.size() == 0)
								break;
                            if(subLine[0] == '/' and subLine[1] == '/')
								continue;

                            std::pair<std::string, std::string> keyValue = parseKeyValue(subLine, false, true);
                            gameFile.configEditor[keyValue.first] = keyValue.second;
                        }
                    }
                }
				else if(header == "Metadata"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						lineNumber++;
						//std::cout << "Correcting line: " << lineNumber << " with the length: " << subLine.size() << " -> " << subLine << std::endl;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						
						//std::cout << "Parsing line: " << lineNumber << " with the length: " << subLine.size() << " -> " << subLine << std::endl;
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;

						std::pair<std::string, std::string> keyValue = parseKeyValue(subLine, false, false);
						gameFile.configMetadata[keyValue.first] = keyValue.second;
					}
				}
				else if(header == "Difficulty"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;

						std::pair<std::string, std::string> keyValue = parseKeyValue(subLine, false, false);
						gameFile.configDifficulty[keyValue.first] = keyValue.second;
					}
				}
				else if(header == "Events"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;
						if(subLine[0] == ' ')
							continue;

						Event tempEvent;
						std::vector<std::string> tempVector;
						tempVector = parseSeperatedLists(subLine, ',');

						if(tempVector[0] == "0")
							tempEvent.eventType = 0;
						else if(tempVector[0] == "1" or tempVector[0] == "Video")
							tempEvent.eventType = 1;
						else if(tempVector[0] == "2" or tempVector[0] == "Break")
							tempEvent.eventType = 2;
						else
							continue;

						tempEvent.startTime = std::stoi(tempVector[1]);
						if(tempEvent.eventType == 0){
							tempEvent.filename = tempVector[2].substr(1, tempVector[2].size()-2);
							if(tempVector.size()>3){
								tempEvent.xOffset = std::stoi(tempVector[3]);
								tempEvent.yOffset = std::stoi(tempVector[4]);
							}
							else{
								tempEvent.xOffset = 0;
								tempEvent.yOffset = 0;
							}
							
						}
						else if(tempEvent.eventType == 1){
							tempEvent.filename = tempVector[2].substr(1, tempVector[2].size()-2);
							if(tempVector.size()>3){
								tempEvent.xOffset = std::stoi(tempVector[3]);
								tempEvent.yOffset = std::stoi(tempVector[4]);
							}
							else{
								tempEvent.xOffset = 0;
								tempEvent.yOffset = 0;
							}
						}
						else if(tempEvent.eventType == 2)
							tempEvent.endTime = std::stoi(tempVector[2]);
						
						gameFile.events.push_back(tempEvent);
					}
				}
				else if(header == "TimingPoints"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;

						TimingPoint tempTimingPoint;
						std::vector<std::string> tempVector;
						tempVector = parseSeperatedLists(subLine, ',');
						tempTimingPoint.time = std::stoi(tempVector[0]);
						tempTimingPoint.beatLength = -100;
						if(tempVector[1] != "NaN")
							tempTimingPoint.beatLength = std::stod(tempVector[1]);
						else
							tempTimingPoint.renderTicks = false;
						tempTimingPoint.meter = std::stoi(tempVector[2]);
						tempTimingPoint.sampleSet = std::stoi(tempVector[3]);
						tempTimingPoint.sampleIndex = std::stoi(tempVector[4]);
						tempTimingPoint.volume = std::stoi(tempVector[5]);
						tempTimingPoint.uninherited = bool(std::stoi(tempVector[6]));
						tempTimingPoint.effects = std::stoi(tempVector[7]);
						gameFile.timingPoints.push_back(tempTimingPoint);
					}
				}
				else if(header == "Colours"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;

						std::pair<std::string, std::string> keyValue = parseKeyValue(subLine, false, false);
						
						if(subLine[0] == 'C'){
							std::vector<short> tempColour (3);
							std::vector<std::string> tempVectorColours;
							tempVectorColours = parseSeperatedLists(keyValue.second, ',');
							for(int i = 0; i < 3; i++)
								tempColour[i] = std::stoi(tempVectorColours[i]);
							gameFile.comboColours.push_back(tempColour);
						}

						gameFile.configColours[keyValue.first] = keyValue.second;
					}
				}
				else if(header == "HitObjects"){
					std::string subLine;
					while(std::getline(ifs, subLine)){
						Global.parsedLines++;
						if(subLine.size() == 0)
							break;
						if(subLine[subLine.size()-1] == 13)
							subLine.pop_back();
						if(subLine.size() == 0)
							break;
						if(subLine[0] == '/' and subLine[1] == '/')
							continue;
						std::vector<std::string> tempVector;
						tempVector = parseSeperatedLists(subLine, ',');
						HitObjectData tempHitObject;

						tempHitObject.x = std::clamp(std::stoi(tempVector[0]) + 64, -32767, 32767);
						tempHitObject.y = std::clamp(std::stoi(tempVector[1]) + 48, -32767, 32767);
						tempHitObject.time = std::stoi(tempVector[2]);
						tempHitObject.type = std::stoi(tempVector[3]);

						std::string typeInBinary;
						typeInBinary = DecimalToBinaryString(tempHitObject.type);
						if(typeInBinary[0] == '1'){
							tempHitObject.type = 1;
							tempHitObject.hitSound = std::stoi(tempVector[4]);
							tempHitObject.startingACombo = typeInBinary[2] == '1';
							tempHitObject.skipComboColours = int(typeInBinary[4] == '1') + int(typeInBinary[5] == '1') * 2 + int(typeInBinary[6] == '1') * 4;

							if(tempVector.size() > 5){
								std::vector<std::string> tempVectorHitSample;
								tempVectorHitSample = parseSeperatedLists(tempVector[5], ':');
								if(tempVectorHitSample.size() > 0)
									tempHitObject.normalSet = std::stoi(tempVectorHitSample[0]);
								else
									tempHitObject.normalSet = 0;
								if(tempVectorHitSample.size() > 1)
									tempHitObject.additionSet = std::stoi(tempVectorHitSample[1]);
								else
									tempHitObject.additionSet = 0;
								if(tempVectorHitSample.size() > 2)
									tempHitObject.hindex = std::stoi(tempVectorHitSample[2]);
								else
									tempHitObject.hindex = 0;
								if(tempVectorHitSample.size() > 3)
									tempHitObject.volume = std::stoi(tempVectorHitSample[3]);
								else
									tempHitObject.volume = 0;
								if(tempVectorHitSample.size() > 4){
									tempHitObject.filename = tempVectorHitSample[4];
									tempHitObject.useDefaultHitSound = false;
								}
								else
									tempHitObject.useDefaultHitSound = true;
							}
							else{
								tempHitObject.normalSet = 0;
								tempHitObject.additionSet = 0;
								tempHitObject.hindex = 0;
								tempHitObject.volume = 0;
								tempHitObject.useDefaultHitSound = true;
							}

						}
						else if(typeInBinary[1] == '1'){
							tempHitObject.type = 2;
							tempHitObject.hitSound = std::stoi(tempVector[4]);
							tempHitObject.startingACombo = typeInBinary[2] == '1';
							tempHitObject.skipComboColours = int(typeInBinary[4] == '1') + int(typeInBinary[5] == '1') * 2 + int(typeInBinary[6] == '1') * 4;

							std::vector<std::string> tempVectorCurvePoints;
							tempVectorCurvePoints = parseSeperatedLists(tempVector[5], '|');
							tempHitObject.curveType = tempVectorCurvePoints[0][0];

							if(tempVectorCurvePoints.size()-1 == 1){
								tempHitObject.curveType = 'L';
							}
							if(tempVectorCurvePoints.size()-1 >= 3 && tempHitObject.curveType == 'P'){
								tempHitObject.curveType = 'B';
							}
							
							for(size_t i = 1; i < tempVectorCurvePoints.size(); i++){
								std::vector<std::string> tempVectorCurvePointCords;
								tempVectorCurvePointCords = parseSeperatedLists(tempVectorCurvePoints[i], ':');
								tempHitObject.curvePoints.push_back(std::make_pair(std::clamp(std::stoi(tempVectorCurvePointCords[0])+64, -32767, 32767), std::clamp(std::stoi(tempVectorCurvePointCords[1])+48, -32767, 32767)));
							}

							/*if(tempHitObject.curveType == 'P' and tempHitObject.curvePoints.size() == 2 and (
								(tempHitObject.curvePoints[0].first == tempHitObject.curvePoints[1].first and tempHitObject.curvePoints[0].second == tempHitObject.curvePoints[1].second) or 
								(tempHitObject.curvePoints[0].first == tempHitObject.x and tempHitObject.curvePoints[0].second == tempHitObject.y) or 
								(tempHitObject.curvePoints[1].first == tempHitObject.x and tempHitObject.curvePoints[1].second == tempHitObject.y))){
								tempHitObject.curveType = 'L';
							}*/

							tempHitObject.slides = std::stoi(tempVector[6]);
							tempHitObject.length = std::stoi(tempVector[7]);
							if(tempVector.size() > 8){
								std::vector<std::string> tempVectorEdgeSounds;
								tempVectorEdgeSounds = parseSeperatedLists(tempVector[8], '|');
								for(size_t i = 0; i < tempVectorEdgeSounds.size(); i++)
									tempHitObject.edgeSounds.push_back(std::stoi(tempVectorEdgeSounds[i]));
							}
							else{
								for(int i = 0; i < tempHitObject.slides; i++){
									std::vector<std::string> tempVectorEdgeSetsCords;
									tempHitObject.edgeSounds.push_back(0);
								}
							}
							if(tempVector.size() > 9){
								std::vector<std::string> tempVectorEdgeSets;
								tempVectorEdgeSets = parseSeperatedLists(tempVector[9], '|');

								for(size_t i = 0; i < tempVectorEdgeSets.size(); i++){
									std::vector<std::string> tempVectorEdgeSetsCords;
									tempVectorEdgeSetsCords = parseSeperatedLists(tempVectorEdgeSets[i], ':');
									tempHitObject.edgeSets.push_back(std::make_pair(std::stoi(tempVectorEdgeSetsCords[0]), std::stoi(tempVectorEdgeSetsCords[1])));
								}
							}
							else{
								for(int i = 0; i < tempHitObject.slides; i++){
									std::vector<std::string> tempVectorEdgeSetsCords;
									tempHitObject.edgeSets.push_back(std::make_pair(0, 0));
								}
							}
							if(tempVector.size() > 10){
								std::vector<std::string> tempVectorHitSample;
								tempVectorHitSample = parseSeperatedLists(tempVector[10], ':');
								if(tempVectorHitSample.size() > 0)
									tempHitObject.normalSet = std::stoi(tempVectorHitSample[0]);
								else
									tempHitObject.normalSet = 0;
								if(tempVectorHitSample.size() > 1)
									tempHitObject.additionSet = std::stoi(tempVectorHitSample[1]);
								else
									tempHitObject.additionSet = 0;
								if(tempVectorHitSample.size() > 2)
									tempHitObject.hindex = std::stoi(tempVectorHitSample[2]);
								else
									tempHitObject.hindex = 0;
								if(tempVectorHitSample.size() > 3)
									tempHitObject.volume = std::stoi(tempVectorHitSample[3]);
								else
									tempHitObject.volume = 0;
								if(tempVectorHitSample.size() > 4){
									tempHitObject.filename = tempVectorHitSample[4];
									tempHitObject.useDefaultHitSound = false;
								}
								else
									tempHitObject.useDefaultHitSound = true;
							}
							else{
								tempHitObject.normalSet = 0;
								tempHitObject.additionSet = 0;
								tempHitObject.hindex = 0;
								tempHitObject.volume = 0;
								tempHitObject.useDefaultHitSound = true;
							}
						}
						else if(typeInBinary[3] == '1'){
							tempHitObject.type = 3;
							tempHitObject.hitSound = std::stoi(tempVector[4]);
							tempHitObject.startingACombo = typeInBinary[2] == '1';
							tempHitObject.skipComboColours = int(typeInBinary[4] == '1') + int(typeInBinary[5] == '1') * 2 + int(typeInBinary[6] == '1') * 4;

							tempHitObject.endTime = std::stoi(tempVector[5]);

							if(tempVector.size() > 6){
								std::vector<std::string> tempVectorHitSample;
								tempVectorHitSample = parseSeperatedLists(tempVector[6], ':');

								if(tempVectorHitSample.size() > 0)
									tempHitObject.normalSet = std::stoi(tempVectorHitSample[0]);
								else
									tempHitObject.normalSet = 0;
								if(tempVectorHitSample.size() > 1)
									tempHitObject.additionSet = std::stoi(tempVectorHitSample[1]);
								else
									tempHitObject.additionSet = 0;
								if(tempVectorHitSample.size() > 2)
									tempHitObject.hindex = std::stoi(tempVectorHitSample[2]);
								else
									tempHitObject.hindex = 0;
								if(tempVectorHitSample.size() > 3)
									tempHitObject.volume = std::stoi(tempVectorHitSample[3]);
								else
									tempHitObject.volume = 0;
								if(tempVectorHitSample.size() > 4){
									tempHitObject.filename = tempVectorHitSample[4];
									tempHitObject.useDefaultHitSound = false;
								}
								else
									tempHitObject.useDefaultHitSound = true;
							}
							else{
								tempHitObject.normalSet = 0;
								tempHitObject.additionSet = 0;
								tempHitObject.hindex = 0;
								tempHitObject.volume = 0;
								tempHitObject.useDefaultHitSound = true;
							}
						}
						gameFile.hitObjects.push_back(tempHitObject);
					}
				}
				else{
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Empty Line? " << lineNumber << " with size " << line.size() << std::endl;
				}
			}
		}
		ifs.close();
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Beatmap file closed\n";
	}
	
	return gameFile;	
}

std::pair<std::string, std::string> Parser::parseKeyValue(std::string line, bool hasSpaceBefore, bool hasSpaceAfter)
{
	size_t spliceLocation;
	for(spliceLocation = 0; spliceLocation < line.size(); spliceLocation++)
		if(line[spliceLocation] == ':')
			break;
	if(hasSpaceAfter)
		if(spliceLocation + 1 < line.size() && line[spliceLocation + 1] != ' ')
			hasSpaceAfter = false;
	if(hasSpaceBefore)
		if(spliceLocation - 1 >= 0 && line[spliceLocation - 1] != ' ')
			hasSpaceBefore = false;
	return make_pair(line.substr(0, spliceLocation - hasSpaceBefore), 
	line.substr(spliceLocation + hasSpaceAfter + 1, line.size() - 1 - spliceLocation - hasSpaceAfter));
}

std::vector<std::string> Parser::parseSeperatedLists(std::string list, char seperator){
	std::vector<std::string> ans;
	int size = 0;
	int lastCommaLocation = - 1;
	for(size_t i = 0; i < list.size(); i++){
		if(list[i] == seperator or i == list.size()-1){
			size++;
			lastCommaLocation = i;
		}
	}
	ans.reserve(size);
	lastCommaLocation = - 1;
	for(size_t i = 0; i < list.size(); i++){
		if(list[i] == seperator or i == list.size()-1){
			ans.push_back(list.substr(lastCommaLocation + 1, i - lastCommaLocation - 1 + int(i == list.size()-1)));
			lastCommaLocation = i;
		}
	}
	return ans;
}

std::string Parser::DecimalToBinaryString(int a)
{
    std::string binary = "";
    int mask = 1;
    for(int i = 0; i < 7; i++){
        if((mask & a) >= 1)
            binary = binary + "1";
        else
            binary = binary + "0";
        mask <<= 1;
    }
    return binary;
}

#include "gamemanager.hpp"
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <math.h>
#include <hitobject.hpp>
#include "globals.hpp"
#include "utils.hpp"
#include "fs.hpp"
#include <sys/time.h>
#include "followpoint.hpp"
#include "time_util.hpp"
#include "state.hpp"

#include "linkedListImpl.hpp"

//for some reason the clamp function didnt work so here is a manual one
float GameManager::clip(float value, float min, float max){
  	return std::min(std::max(value,min), max);
}


GameManager* GameManager::inst_ = NULL;

//get a new gamemanager
GameManager* GameManager::getInstance() {
   	if (inst_ == NULL)
   		inst_ = new GameManager();
   	return(inst_);
}

//call the initilization
GameManager::GameManager(){
	init();
}

//initilize the game manager
void GameManager::init(){

}

//main game loop
void GameManager::update(){
	//for now the left key on the keyboard plays the map automatically
	//this is pretty useful when you need to quickly test the timings
	//since the bot is always pressing in the correct time window
	if(IsKeyDown(Global.AUTO_KEY)){
		Global.useAuto = true;
	}
	else{
		Global.useAuto = false;
	}

	
	//current implementation of the backgrounds
	int eventSize = gameFile.events.size();
	for(int i = eventSize-1; i >= 0; i--){
		if(gameFile.events[i].startTime <= currentTime*1000.0f){
			if(gameFile.events[i].eventType == 0){
				currentBackgroundTexture = gameFile.events[i].filename;
			}
			gameFile.events.pop_back();
		}
		else
			break;
	}

	for(int i = lastCurrentTiming; i >= 0; i--){
		if(gameFile.timingPoints[i].time <= currentTime*1000.0f + 30.0f){
			currentTimingSettings.renderTicks = gameFile.timingPoints[i].renderTicks;
			currentTimingSettings.sliderSpeedOverride = 1;
			currentTimingSettings.time = gameFile.timingPoints[i].time;
			double tempBeatLength;
			tempBeatLength = gameFile.timingPoints[i].beatLength;
			//std::cout << "beatLength: " << tempBeatLength << std::endl;
			if(tempBeatLength >= 0){
				currentTimingSettings.beatLength = tempBeatLength;
				verytempbeat2 = tempBeatLength;
				currentTimingSettings.sliderSpeedOverride = 1;
			}
			if(tempBeatLength < 0){
				currentTimingSettings.sliderSpeedOverride = (100 / tempBeatLength * (-1));
				currentTimingSettings.beatLength = verytempbeat2;
			}
			currentTimingSettings.meter = gameFile.timingPoints[i].meter;
			currentTimingSettings.sampleSet = gameFile.timingPoints[i].sampleSet;
			currentTimingSettings.sampleIndex = gameFile.timingPoints[i].sampleIndex;
			currentTimingSettings.volume = gameFile.timingPoints[i].volume;
			currentTimingSettings.uninherited = gameFile.timingPoints[i].uninherited;
			currentTimingSettings.effects = gameFile.timingPoints[i].effects;
			//std::cout << "Current Timing Settings: " << currentTimingSettings.time << " " << currentTimingSettings.sampleSet << " " << currentTimingSettings.sampleIndex << std::endl;
			lastCurrentTiming = i - 1; 
		}
		else
			break;
	}

	timingSettings tempTiming;
	for(int i = lastTimingLoc; i >= 0; i--){
		if(gameFile.timingPoints[i].time - gameFile.preempt <= currentTime*1000.0f){
			tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
			tempTiming.sliderSpeedOverride = 1;
			tempTiming.time = gameFile.timingPoints[i].time;
			double tempBeatLength;
			tempBeatLength = gameFile.timingPoints[i].beatLength;
			//std::cout << "beatLength: " << tempBeatLength << std::endl;
			if(tempBeatLength >= 0){
				tempTiming.beatLength = tempBeatLength;
				verytempbeat = tempBeatLength;
				tempTiming.sliderSpeedOverride = 1;
			}
			if(tempBeatLength < 0){
				tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
				tempTiming.beatLength = verytempbeat;
			}
			tempTiming.meter = gameFile.timingPoints[i].meter;
			tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
			tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
			tempTiming.volume = gameFile.timingPoints[i].volume;
			tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
			tempTiming.effects = gameFile.timingPoints[i].effects;
			timingSettingsForHitObject.push_back(tempTiming);
			lastTimingLoc = i - 1; 
		}
		else
			break;
	}
	if(timingSettingsForHitObject.size() == 0){
		int i = lastTimingLoc;
		if(gameFile.timingPoints.size() == 0){
			std::cout << "missing timingpoints? cant do much. crash imminent." << std::endl;
		}
		else{
			tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
			tempTiming.sliderSpeedOverride = 1;
			tempTiming.time = gameFile.timingPoints[i].time;
			double tempBeatLength;
			tempBeatLength = gameFile.timingPoints[i].beatLength;
			if(tempBeatLength >= 0){
				tempTiming.beatLength = tempBeatLength;
				verytempbeat = tempBeatLength;
				tempTiming.sliderSpeedOverride = 1;
			}
			if(tempBeatLength < 0){
				tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
				tempTiming.beatLength = verytempbeat;
			}
			tempTiming.meter = gameFile.timingPoints[i].meter;
			tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
			tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
			tempTiming.volume = gameFile.timingPoints[i].volume;
			tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
			tempTiming.effects = gameFile.timingPoints[i].effects;
			timingSettingsForHitObject.push_back(tempTiming);
			lastTimingLoc = i - 1; 
		}
	}
	

	//spawn the hitobjects when their time comes
	/*int size = gameFile.hitObjects.size();	
	for(int i = size-1; i >= 0; i--){
		if(gameFile.hitObjects[i].time - gameFile.preempt <= currentTime*1000.0f){
			//if(gameFile.hitObjects[i].type == 2 and gameFile.hitObjects[i].totalLength > Global.maxSliderSize){
			//	std::cout << "well fuck this long slider i guess. \n";
			//	gameFile.hitObjects.pop_back();
			//}
			//else{
				spawnHitObject(gameFile.hitObjects[i]);
				if(objects[objects.size()-1]->data.startingACombo){
					currentComboIndex++;
					if(gameFile.comboColours.size()) currentComboIndex = (currentComboIndex + objects[objects.size()-1]->data.skipComboColours) % gameFile.comboColours.size();
					combo = 1;
				}
				if(gameFile.comboColours.size()) objects[objects.size()-1]->data.colour = gameFile.comboColours[currentComboIndex];
				objects[objects.size()-1]->data.comboNumber = combo;
				combo++;
				int index = 0;
				for(int amog = 0; amog < timingSettingsForHitObject.size(); amog++){
					if(timingSettingsForHitObject[amog].time > gameFile.hitObjects[i].time)
						break;
					index = amog;
				}

				// o şejkilde lişğaksda başka ne uzun biliyor musun bence benim akıllığım terinde
				// -ömer 2022
				objects[objects.size()-1]->data.timing.beatLength = timingSettingsForHitObject[index].beatLength;
				objects[objects.size()-1]->data.timing.meter = timingSettingsForHitObject[index].meter;
				objects[objects.size()-1]->data.timing.sampleSet = timingSettingsForHitObject[index].sampleSet;
				objects[objects.size()-1]->data.timing.sampleIndex = timingSettingsForHitObject[index].sampleIndex;
				objects[objects.size()-1]->data.timing.volume = timingSettingsForHitObject[index].volume;
				objects[objects.size()-1]->data.timing.uninherited = timingSettingsForHitObject[index].uninherited;
				objects[objects.size()-1]->data.timing.effects = timingSettingsForHitObject[index].effects;
				objects[objects.size()-1]->data.timing.sliderSpeedOverride = timingSettingsForHitObject[index].sliderSpeedOverride;
				objects[objects.size()-1]->data.index = objects.size()-1;
				objects[objects.size()-1]->data.textureReady = false;
				objects[objects.size()-1]->data.textureLoaded = false;
				objects[objects.size()-1]->data.timing.renderTicks = timingSettingsForHitObject[index].renderTicks;
				//std::cout << "Time:" << timingSettingsForHitObject[index].time << " Beat:" << objects[objects.size()-1]->data.timing.beatLength <<
				//" Meter:" << objects[objects.size()-1]->data.timing.meter << " SV:" << objects[objects.size()-1]->data.timing.sliderSpeedOverride <<
				//" SS:" << sliderSpeed << " RT:" << objects[objects.size()-1]->data.timing.renderTicks;
				

				objects[objects.size()-1]->init();
				//Vector2 templastCords = {objects[objects.size()-1]->data.ex, objects[objects.size()-1]->data.ey};
				//objects[objects.size()-1]->data.ex = lastCords.x;
				//objects[objects.size()-1]->data.ey = lastCords.y;
				//objects[objects.size()-1]->data.lastTime = lastHitTime;
				lastHitTime = objects[objects.size()-1]->data.time;
				if(objects[objects.size()-1]->data.type == 2){
					objects[objects.size()-1]->data.time + (objects[objects.size()-1]->data.length/100) * (objects[objects.size()-1]->data.timing.beatLength) / (sliderSpeed * objects[objects.size()-1]->data.timing.sliderSpeedOverride) * objects[objects.size()-1]->data.slides;
				}
				//lastCords = templastCords;

				//std::thread objectThread(std::bind(&HitObject::init, objects[objects.size()-1]));
				//objectThread.join();
				gameFile.hitObjects.pop_back();
				spawnedHitObjects++;
				for(int amog = 0; amog < index - 1; amog++){
					timingSettingsForHitObject.erase(timingSettingsForHitObject.begin());
				}
			//}
		}
		else
			break;
	}
	*/
	//Global.mutex2.lock();
	int size = gameFile.hitObjects.size();	
	for(int i = size-1; i >= 0; i--){
		if(gameFile.hitObjects[i].time - gameFile.preempt <= currentTime*1000.0f){

			spawnHitObject(gameFile.hitObjects[i]);

			Node* hitObjectNode = objectsLinkedList.getTail();
			HitObject* hitObject = (HitObject*)hitObjectNode->object;

			if(hitObject->data.startingACombo){
				currentComboIndex++;
				if(gameFile.comboColours.size())
					currentComboIndex = (currentComboIndex + hitObject->data.skipComboColours) % gameFile.comboColours.size();
				combo = 1;
			}
			if(gameFile.comboColours.size())
				hitObject->data.colour = gameFile.comboColours[currentComboIndex];
			hitObject->data.comboNumber = combo;
			combo++;

			int index = 0;
			for(int amog = 0; amog < timingSettingsForHitObject.size(); amog++){
				if(timingSettingsForHitObject[amog].time > gameFile.hitObjects[i].time)
					break;
				index = amog;
			}

			hitObject->data.timing.beatLength = timingSettingsForHitObject[index].beatLength;
			hitObject->data.timing.meter = timingSettingsForHitObject[index].meter;
			hitObject->data.timing.sampleSet = timingSettingsForHitObject[index].sampleSet;
			hitObject->data.timing.sampleIndex = timingSettingsForHitObject[index].sampleIndex;
			hitObject->data.timing.volume = timingSettingsForHitObject[index].volume;
			hitObject->data.timing.uninherited = timingSettingsForHitObject[index].uninherited;
			hitObject->data.timing.effects = timingSettingsForHitObject[index].effects;
			hitObject->data.timing.sliderSpeedOverride = timingSettingsForHitObject[index].sliderSpeedOverride;
			hitObject->data.index = 0;
			hitObject->data.textureReady = false;
			hitObject->data.textureLoaded = false;
			hitObject->data.timing.renderTicks = timingSettingsForHitObject[index].renderTicks;

			hitObject->init();
			lastHitTime = hitObject->data.time;
			if(hitObject->data.type == 2){
				hitObject->data.time + (hitObject->data.length/100) * (hitObject->data.timing.beatLength) / (sliderSpeed * hitObject->data.timing.sliderSpeedOverride) * hitObject->data.slides;
			}
			gameFile.hitObjects.pop_back();
			spawnedHitObjects++;
			for(int amog = 0; amog < index - 1; amog++){
				timingSettingsForHitObject.erase(timingSettingsForHitObject.begin());
			}
			//std::cout << "spawned\n";
		}
	}

	
	Node * hitObjectNode = objectsLinkedList.getHead();
	Node * hitObjectNodeNext;
	HitObject* hitObject;
	bool stop = true;
	int processed = 0;
	int sizeOfList = objectsLinkedList.getSize();
	while(true){
		if(hitObjectNode == NULL){
			break;
		}

		hitObject = (HitObject*)hitObjectNode->object;
		hitObjectNodeNext = hitObjectNode->next;
		processed++;

		if(IsKeyPressed(Global.AUTO_KEY)){
			Global.AutoMouseStartTime = currentTime*1000.0f;
			Global.AutoMousePositionStart = {320, 240};
		}

		if(hitObjectNode->prev == NULL){
			hitObject->data.touch = true;
			Global.AutoMousePosition = lerp(Global.AutoMousePositionStart, {hitObject->data.x, hitObject->data.y}, clip((currentTime*1000.0f-Global.AutoMouseStartTime) / (hitObject->data.time-Global.AutoMouseStartTime), 0, 1));
		}

		if (stop && hitObjectNode->prev == NULL && (Global.Key1P or Global.Key2P)){
			if (hitObject->data.type != 2){
				if (CheckCollisionPointCircle(Global.MousePosition,Vector2{hitObject->data.x,(float)hitObject->data.y}, circlesize/2.0f)){
					if(std::abs(currentTime*1000.0f - hitObject->data.time) > gameFile.p50Final + Global.extraJudgementTime/2.0f){
						hitObject->data.point = 0;
						if(clickCombo > 30){
							SetSoundVolume(&SoundFilesAll.data["combobreak"], 1.0f);
							PlaySound(&SoundFilesAll.data["combobreak"]);
						}
						maxCombo = std::max(maxCombo, clickCombo);
						clickCombo = 0;
						hit0s++;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					else if(std::abs(currentTime*1000.0f - hitObject->data.time) > gameFile.p100Final + Global.extraJudgementTime/2.0f){
						hitObject->data.point = 1;
						score+= 50 + (50 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
						hit50s++;
						Global.errorDiv++;
						Global.errorLast = (long long)((currentTime*1000.0f - hitObject->data.time) * 1000.0f);
						Global.errorSum += Global.errorLast;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					else if(std::abs(currentTime*1000.0f - hitObject->data.time) > gameFile.p300Final + Global.extraJudgementTime/2.0f){
						hitObject->data.point = 2;
						score+= 100 + (100 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
						hit100s++;
						Global.errorDiv++;
						Global.errorLast = (long long)((currentTime*1000.0f - hitObject->data.time) * 1000.0f);
						Global.errorSum += Global.errorLast;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					else{
						hitObject->data.point = 3;
						score+= 300 + (300 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
						hit300s++;
						Global.errorDiv++;
						Global.errorLast = (long long)((currentTime*1000.0f - hitObject->data.time) * 1000.0f);
						Global.errorSum += Global.errorLast;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					
					int volume = hitObject->data.volume;
					if(volume == 0){
						hitObject->data.volume = hitObject->data.timing.volume;
						volume = hitObject->data.volume;
					}
					std::vector<std::string> sounds = getAudioFilenames(currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex, defaultSampleSet, hitObject->data.normalSet, hitObject->data.additionSet, hitObject->data.hitSound, hitObject->data.hindex, hitObject->data.filename);
					for(int soundIndex = 0; soundIndex < sounds.size(); soundIndex+=2){
						if(SoundFilesAll.data.count(sounds[soundIndex]) == 1 and SoundFilesAll.loaded[sounds[soundIndex]].value){
							SetSoundPan(&SoundFilesAll.data[sounds[soundIndex]], 1-clip(hitObject->data.x / 640.0, 0, 1));
							SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex]], Global.hitVolume * ((float)volume/100.0f));
							PlaySound(&SoundFilesAll.data[sounds[soundIndex]]);
							//std::cout << sounds[0] << " played \n";
						}
						else if(SoundFilesAll.data.count(sounds[soundIndex+1]) == 1 and SoundFilesAll.loaded[sounds[soundIndex+1]].value){
							SetSoundPan(&SoundFilesAll.data[sounds[soundIndex+1]], 1-clip(hitObject->data.x / 640.0, 0, 1));
							SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex+1]], Global.hitVolume * ((float)volume/100.0f));
							PlaySound(&SoundFilesAll.data[sounds[soundIndex+1]]);
							//std::cout << sounds[1] << " played \n";
						}
					}

					hitObject->data.time = currentTime*1000.0f;
					destroyHitObject(hitObjectNode);
					stop = false;
				}
				else{
					hitObject->data.touch = true;
					hitObject->update();
					if(hitObject->data.destruct == true){
						destroyHitObject(hitObjectNode);
					}
				}
			}
			else if (hitObject->data.type == 2){
				if(Slider* tempslider = dynamic_cast<Slider*>(hitObject)){
					if(CheckCollisionPointCircle(Global.MousePosition,Vector2{hitObject->data.x,(float)hitObject->data.y}, circlesize/2.0f) && currentTime*1000.0f < tempslider->data.time + gameFile.p50Final){
						if(std::abs(currentTime*1000.0f - tempslider->data.time) > gameFile.p50Final + Global.extraJudgementTime/2.0f){
							tempslider->is_hit_at_first = true;
							stop = false;
							tempslider->earlyhit = true;
							if(clickCombo > 30){
								SetSoundVolume(&SoundFilesAll.data["combobreak"], 1.0f);
								PlaySound(&SoundFilesAll.data["combobreak"]);
							}
							maxCombo = std::max(maxCombo, clickCombo);
							clickCombo = 0;
							Global.Key1P = false;
							Global.Key2P = false;
						}
						else{
							tempslider->is_hit_at_first = true;
							stop = false;
							clickCombo++;
							Global.Key1P = false;
							Global.Key2P = false;
						}
						int volume = tempslider->data.volume;
						if(volume == 0){
							tempslider->data.volume = tempslider->data.timing.volume;
							volume = tempslider->data.volume;
						}

						std::vector<std::string> sounds = getAudioFilenames(currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex, defaultSampleSet, hitObject->data.edgeSets[0].first, hitObject->data.edgeSets[0].second, hitObject->data.edgeSounds[0], hitObject->data.hindex, hitObject->data.filename);

						for(int soundIndex = 0; soundIndex < sounds.size(); soundIndex+=2){
							if(SoundFilesAll.data.count(sounds[soundIndex]) == 1 and SoundFilesAll.loaded[sounds[soundIndex]].value){
								SetSoundPan(&SoundFilesAll.data[sounds[soundIndex]], 1-clip(hitObject->data.x / 640.0, 0, 1));
								SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex]], Global.hitVolume * ((float)volume/100.0f) );
								PlaySound(&SoundFilesAll.data[sounds[soundIndex]]);
								//std::cout << sounds[0] << " played \n";
							}
							else if(SoundFilesAll.data.count(sounds[soundIndex+1]) == 1 and SoundFilesAll.loaded[sounds[soundIndex+1]].value){
								SetSoundPan(&SoundFilesAll.data[sounds[soundIndex+1]], 1-clip(hitObject->data.x / 640.0, 0, 1));
								SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex+1]], Global.hitVolume * ((float)volume/100.0f));
								PlaySound(&SoundFilesAll.data[sounds[soundIndex+1]]);
								//std::cout << sounds[1] << " played \n";
							}
						}

					}
				}
				//this cursed else train is nothing to worry about...
				hitObject->update();
				if(hitObject->data.destruct == true){
					destroyHitObject(hitObjectNode);
				}
			}
			else{
				hitObject->update();
				if(hitObject->data.destruct == true){
					destroyHitObject(hitObjectNode);
				}
			}
			Global.Key1P = false;
			Global.Key2P = false;
		}
		else{
			bool debugf = IsKeyDown(Global.AUTO_KEY);
			if(debugf){
				if(hitObject->data.point != 3 && currentTime*1000.0f > hitObject->data.time){
					if (hitObject->data.type != 2){
						//Global.MousePosition = {objects[i]->data.x, objects[i]->data.y};
						hitObject->data.point = 3;
						score+= 300 + (300 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
						hit300s++;
						int volume = hitObject->data.volume;
						if(volume == 0){
							hitObject->data.volume = hitObject->data.timing.volume;
							volume = hitObject->data.volume;
						}

						std::vector<std::string> sounds = getAudioFilenames(currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex, defaultSampleSet, hitObject->data.normalSet, hitObject->data.additionSet, hitObject->data.hitSound, hitObject->data.hindex, hitObject->data.filename);
						
						//std::cout << sounds.size() << std::endl;

						for(int soundIndex = 0; soundIndex < sounds.size(); soundIndex+=2){
							if(SoundFilesAll.data.count(sounds[soundIndex]) == 1 and SoundFilesAll.loaded[sounds[soundIndex]].value){
								SetSoundPan(&SoundFilesAll.data[sounds[soundIndex]], 1-clip(hitObject->data.x / 640.0, 0, 1));
								SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex]], Global.hitVolume * ((float)volume/100.0f));
								PlaySound(&SoundFilesAll.data[sounds[soundIndex]]);
								//std::cout << sounds[0] << " played \n";
							}
							else if(SoundFilesAll.data.count(sounds[soundIndex+1]) == 1 and SoundFilesAll.loaded[sounds[soundIndex+1]].value){
								SetSoundPan(&SoundFilesAll.data[sounds[soundIndex+1]], 1-clip(hitObject->data.x / 640.0, 0, 1));
								SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex+1]], Global.hitVolume * ((float)volume/100.0f));
								PlaySound(&SoundFilesAll.data[sounds[soundIndex+1]]);
								//std::cout << sounds[1] << " played \n";
							}
						}

						//std::cout << 3.5*easeInOutCubic((1-(currentTime*1000.0f - objects[i]->data.time + gameFile.preempt)/gameFile.preempt))+1 << std::endl;
						hitObject->data.time = currentTime*1000.0f;

						Global.AutoMousePositionStart = {hitObject->data.x, hitObject->data.y};
						Global.AutoMouseStartTime = currentTime*1000.0f;

						destroyHitObject(hitObjectNode);

						stop = false;
					}
					else if (hitObject->data.type == 2){
						Slider* tempslider = dynamic_cast<Slider*>(hitObject);
						tempslider->is_hit_at_first = true;
						hitObject->data.point = 3;
						stop = false;
						clickCombo++;
						int volume = tempslider->data.volume;
						if(volume == 0){
							tempslider->data.volume = tempslider->data.timing.volume;
							volume = tempslider->data.volume;
						}

						std::vector<std::string> sounds = getAudioFilenames(currentTimingSettings.sampleSet, currentTimingSettings.sampleIndex, defaultSampleSet, hitObject->data.edgeSets[0].first, hitObject->data.edgeSets[0].second, hitObject->data.edgeSounds[0], hitObject->data.hindex, hitObject->data.filename);
						

						for(int soundIndex = 0; soundIndex < sounds.size(); soundIndex+=2){
							if(SoundFilesAll.data.count(sounds[soundIndex]) == 1 and SoundFilesAll.loaded[sounds[soundIndex]].value){
								SetSoundPan(&SoundFilesAll.data[sounds[soundIndex]], 1-clip(hitObject->data.x / 640.0, 0, 1));
								SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex]], Global.hitVolume * ((float)volume/100.0f));
								PlaySound(&SoundFilesAll.data[sounds[soundIndex]]);
								//std::cout << sounds[0] << " played \n";
							}
							else if(SoundFilesAll.data.count(sounds[soundIndex+1]) == 1 and SoundFilesAll.loaded[sounds[soundIndex+1]].value){
								SetSoundPan(&SoundFilesAll.data[sounds[soundIndex+1]], 1-clip(hitObject->data.x / 640.0, 0, 1));
								SetSoundVolume(&SoundFilesAll.data[sounds[soundIndex+1]], Global.hitVolume * ((float)volume/100.0f));
								PlaySound(&SoundFilesAll.data[sounds[soundIndex+1]]);
								//std::cout << sounds[1] << " played \n";
							}
						}
						hitObject->update();
						if(hitObject->data.destruct == true){
							destroyHitObject(hitObjectNode);
						}
					}
				}
				else{
					if (hitObject->data.type == 2){
						hitObject->update();
						if(hitObject->data.destruct == true){
							destroyHitObject(hitObjectNode);
						}
					}
				}
			}
			else{
				hitObject->update();
				if(hitObject->data.destruct == true){
					destroyHitObject(hitObjectNode);
				}
			}
		}

		hitObjectNode = hitObjectNodeNext;
	}

	if(processed != sizeOfList){
		//std::cout << "what the actual fuck\n";
	}

	int deadoldsize = dead_objects.size();
	int deadnewsize = dead_objects.size();

	/*for(int i = 0; i < dead_objects.size(); i++){
		dead_objects[i]->data.index = i;
		dead_objects[i]->dead_update();
		if(dead_objects[i]->data.expired == true){
			destroyDeadHitObject(i);
			//std::cout << "deleted an object\n";
		}
		deadnewsize = dead_objects.size();
		if(deadnewsize != deadoldsize){
			i--;
			deadoldsize = deadnewsize;
		}
	}*/

	Node * deadHitObjectNode = deadObjectsLinkedList.getHead();
	Node * deadHitObjectNodeNext;
	HitObject* deadHitObject;
	while(true){
		if(deadHitObjectNode == NULL){
			break;
		}
		deadHitObject = (HitObject*)deadHitObjectNode->object;
		deadHitObjectNodeNext = deadHitObjectNode->next;

		deadHitObject->dead_update();
		if(deadHitObject->data.expired == true){
			destroyDeadHitObject(deadHitObjectNode);
		}
		deadHitObjectNode = deadHitObjectNodeNext;
	}


	for(int i = followLines.size()-1; i >= 0; i--){
		if(followLines[i].startTime > currentTime*1000.0f){
			break;
		}
		followLines[i].update();
	}

	for(int i = followLines.size()-1; i >= 0; i--){
		if(!followLines[i].shouldDelete){
			break;
		}
		followLines.pop_back();
	}

	//Global.mutex2.unlock();
}

//main rendering loop
void GameManager::unloadSliderTextures(){
	/*for(int i = dead_objects.size() - 1; i >= 0; i--){
		////Global.mutex.lock();
		if(dead_objects[i]->data.type == 2){
			if(Slider* tempslider = dynamic_cast<Slider*>(dead_objects[i])){
				tempslider->unloadTextures();
			}
		}
		////Global.mutex.unlock();
	}*/
	//Global.mutex.lock();
    //LightLock_Lock(&Global.lightlock);
	Node * deadHitObjectNode = deadObjectsLinkedList.getHead();
	Node * deadHitObjectNodeNext;
	HitObject* deadHitObject;
	while(true){
		if(deadHitObjectNode == NULL){
			break;
		}
		deadHitObject = (HitObject*)deadHitObjectNode->object;
		deadHitObjectNodeNext = deadHitObjectNode->next;

		if(deadHitObject->data.type == 2){
			if(Slider* tempslider = dynamic_cast<Slider*>(deadHitObject)){
				tempslider->unloadTextures();
			}
		}
		
		deadHitObjectNode = deadHitObjectNodeNext;
	}
	Node * HitObjectNode = objectsLinkedList.getHead();
	Node * HitObjectNodeNext;
	HitObject* hitObject;
	while(true){
		if(HitObjectNode == NULL){
			break;
		}
		hitObject = (HitObject*)HitObjectNode->object;
		HitObjectNodeNext = HitObjectNode->next;

		if(hitObject->data.type == 2){
			if(Slider* tempslider = dynamic_cast<Slider*>(hitObject)){
				tempslider->unloadTextures();
			}
		}
		
		HitObjectNode = HitObjectNodeNext;
	}

	//Global.mutex.unlock();
    //LightLock_Unlock(&Global.lightlock);
	Global.sliderTexNeedDeleting = false;
}

void GameManager::render(){
	if(Global.GameTextures == 15){
		return;
	}
	if(Global.GameTextures == -1){
		return;
	}
	Global.NeedForBackgroundClear = true;
	if(currentBackgroundTexture.length() > 0 && backgroundTextures.loaded[currentBackgroundTexture].value){
		//std::cout << currentBackgroundTexture << std::endl;
		Global.NeedForBackgroundClear = false;
		DrawTextureCenter(&backgroundTextures.data[currentBackgroundTexture], 320, 240, (double)std::max((double)GetScreenWidth()/(double)backgroundTextures.data[currentBackgroundTexture].width, (double)GetScreenHeight()/(double)backgroundTextures.data[currentBackgroundTexture].height) / (double)Global.Scale , WHITE);
		//DrawRectangle(-5, -5, GetScreenWidth() + 10, GetScreenHeight() + 10, Fade(BLUE, 1.0f));
	}
	
	for(int i = followLines.size()-1; i >= 0; i--){
		if(followLines[i].startTime > currentTime*1000.0f){
			break;
		}
		followLines[i].render();
	}




	/*for(int i = objects.size() - 1; i >= 0; i--){
		objects[i]->render();
	}*/
	//Global.mutex2.lock();
	Node * hitObjectNode = objectsLinkedList.getTail();
	Node * hitObjectNodeNext;
	HitObject* hitObject;
	
	while(true){
		if(hitObjectNode == NULL){
			break;
		}
		hitObject = (HitObject*)hitObjectNode->object;
		hitObjectNodeNext = hitObjectNode->prev;
		if(hitObject == NULL){
			break;
		}
		hitObject->render();

		hitObjectNode = hitObjectNodeNext;
	}

	Node * deadHitObjectNode = deadObjectsLinkedList.getTail();
	Node * deadHitObjectNodeNext;
	HitObject* deadHitObject;
	while(true){
		if(deadHitObjectNode == NULL){
			break;
		}
		deadHitObject = (HitObject*)deadHitObjectNode->object;
		deadHitObjectNodeNext = deadHitObjectNode->prev;
		if(deadHitObject == NULL){
			break;
		}
		deadHitObject->dead_render();
		
		deadHitObjectNode = deadHitObjectNodeNext;
	}
	//Global.mutex2.unlock();
	DrawCNumbersCenter(score, 320, 10, 0.4f, WHITE);
	DrawCNumbersLeft(clickCombo, 15, 460, 0.6f, WHITE);

	
	if(spawnedHitObjects == 0 && gameFile.hitObjects[gameFile.hitObjects.size() - 1].time > 6000 + currentTime*1000.0f){
		////Global.mutex.lock();
		DrawTextEx(&Global.DefaultFont, TextFormat("TO SKIP PRESS \"S\"\n(Keep in mind that this can affect the offset\nbecause of how the raylib sounds system works)"), {(int)ScaleCordX(5), (int)ScaleCordY(400)}, Scale(20.05), Scale(2), WHITE);
		////Global.mutex.unlock();
	}
	//render the points and the combo
	
}

void GameManager::run(){
	//start playing the music and set the volume, it gets quite loud
	double Time = 0;
	struct timeval tp;
	gettimeofday(&tp, NULL);
	long int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	//ms = getTimer() / 1000.0;

	if(Global.startTime < 0){
		if(Global.FrameTime < 10.0f)
			Global.startTime += Global.FrameTime;
		Time = Global.startTime;
		//std::cout << Time << std::endl;
	}
	if(Global.startTime >= 0 and startMusic){
		std::cout << "Starting Music Playback (only mp3 supported)" << std::endl;
		PlayMusicStream(&backgroundMusic);
		//Global.volume = 1.0f;
		//std::cout << Global.volume << std::endl;
    	SetMusicVolume(&backgroundMusic, Global.volume); //Global.volume
		//SeekMusicStream(&backgroundMusic, 0.0f);
		UpdateMusicStream(&backgroundMusic);
		initTimer();
		//std::cout << "started music" << std::endl;
		//std::cout << "first update" << std::endl;
		//std::cout << sizeof(HitObjectData) << std::endl;
		//std::cout << sizeof(HitObjectData) * gameFile.hitObjects.size() << std::endl;
		//std::cout << "sizes given in bytes\n";
		Global.CurrentInterpolatedTime = 0;
		Global.LastOsuTime = 0;
		TimeLast = ms;
		startMusic = false;
		Global.startTime2 = ms;
		double Time = (double)GetMusicTimePlayed(&backgroundMusic) * 1000.0;
		double amog = getTimer();
		std::cout << "Extra Judgement Time in ms " << Global.extraJudgementTime << std::endl;
		//std::cout << "Time:" << Time << std::endl;
		Global.avgSum = 0;
    	Global.avgNum = 0;
    	Global.avgTime = 0;

		Global.LastFrameTime = getTimer();
	}
	if(true){
		if(Global.volumeChanged){
			SetMusicVolume(&backgroundMusic, Global.volume);
			Global.volumeChanged = false;
		}
		UpdateMusicStream(&backgroundMusic);
		if(spawnedHitObjects == 0 && gameFile.hitObjects[gameFile.hitObjects.size() - 1].time > 6000 + currentTime*1000.0f){
			//DrawTextEx(Global.DefaultFont, TextFormat("TO SKIP PRESS \"S\"\n(Keep in mind that this can affect the offset\nbecause of how the raylib sounds system works)"), {ScaleCordX(5), ScaleCordY(420)}, Scale(15), Scale(1), WHITE);
			if(IsKeyPressed(SDLK_y ) && false){
				SeekMusicStream(&backgroundMusic, (gameFile.hitObjects[gameFile.hitObjects.size() - 1].time - 3000.0f) / 1000.0f);
			}
		}
		//if(GetMusicTimeLength(&backgroundMusic) - GetMusicTimePlayed(&backgroundMusic) < 0.1f)
		//	stop = true;
		if(_music_check_if_ended(&backgroundMusic)){
			maxCombo = std::max(maxCombo, clickCombo);
			StopMusicStream(&backgroundMusic);
			TimerLast = (double)GetMusicTimeLength(&backgroundMusic) * 1000.0;
			TimeLast = getTimer();
			std::cout << "waiting for 1.5 secs for the song to end\n";
			while(true){
				currentTime = (double)(TimerLast + (getTimer() - TimeLast)) / 1000.0;
				GameManager::update();
				SleepInMs(5);
				if((getTimer() - TimeLast) > 1500.0)
					break;
			}
			//std::cout << "waiting done\n";
			/*MutexUnlock(ACCESSING_OBJECTS);
			MutexLock(SWITCHING_STATE);
			MutexLock(ACCESSING_OBJECTS);
			MutexUnlock(SWITCHING_STATE);
			MutexLock(SWITCHING_STATE);*/
			Global.CurrentState->initDone = 3;
            MutexUnlock(ACCESSING_OBJECTS);

			MutexLock(RENDER_BLOCK);
            MutexLock(ACCESSING_OBJECTS);
            MutexUnlock(RENDER_BLOCK);

			MutexLock(SWITCHING_STATE);
			Global.CurrentState->unload();
			MutexUnlock(ACCESSING_OBJECTS);
            Global.CurrentState.reset(new ResultsMenu());
            Global.CurrentState->init();
			MutexLock(ACCESSING_OBJECTS);
			MutexUnlock(SWITCHING_STATE);
			return;
		}
		
		
		if (IsMusicStreamPlaying(&backgroundMusic)){
			Time = (double)GetMusicTimePlayed(&backgroundMusic) * 1000.0;
			if(!AreSame(TimerLast, Time)){
				//Global.extraJudgementTime = std::abs((Time - TimerLast) / 1.5f);
				TimerLast = (double)GetMusicTimePlayed(&backgroundMusic) * 1000.0;
				TimeLast = ms;
			}
			else{
				Time += ms - TimeLast;
			}
		}
		else{
			TimeLast = ms;
		}


		Global.curTime = Time;
		double LastInterpolatedTime = Global.currentOsuTime;

		bool IsInterpolating;

		if (IsMusicStreamPlaying(&backgroundMusic)){
			if (GetMusicTimePlayed(&backgroundMusic) * 1000.0 != 0)
				IsInterpolating = true;
			double ElapsedTime = getTimer() - Global.LastOsuTime;
			Global.LastOsuTime = getTimer();
			Global.CurrentInterpolatedTime += ElapsedTime;
			if (!IsInterpolating || std::abs(GetMusicTimePlayed(&backgroundMusic) * 1000.0 - Global.CurrentInterpolatedTime) > 8){
				Global.CurrentInterpolatedTime = ElapsedTime < 0 ? GetMusicTimePlayed(&backgroundMusic) * 1000.0: std::max(LastInterpolatedTime, GetMusicTimePlayed(&backgroundMusic) * 1000.0);
				IsInterpolating = false;
				//std::cout << "failed interpolation at time " << Global.CurrentInterpolatedTime << "\n";
			}
			else{
				Global.CurrentInterpolatedTime += (GetMusicTimePlayed(&backgroundMusic) * 1000.0 - Global.CurrentInterpolatedTime) / 5;
				Global.CurrentInterpolatedTime = std::max(LastInterpolatedTime, Global.CurrentInterpolatedTime);
			}
		}

		Global.currentOsuTime = IsMusicStreamPlaying(&backgroundMusic) ? Global.CurrentInterpolatedTime : GetMusicTimePlayed(&backgroundMusic);

		currentTime = (double)Time / 1000.0;
		#ifndef THREEDS_BUILD
			if(IsMusicStreamPlaying(&backgroundMusic)){
				//currentTime = (Global.currentOsuTime + Global.offsetTime) / 1000.0;
				//currentTime = GetMusicTimePlayed(&backgroundMusic);
				//std::cout << "music playin\n";
			}
		#endif
		//currentTime -= 8/1000.0f;
		//currentTime *= 2;
		//std::cout << "update\n";
		GameManager::update();
		//std::cout << "update done\n";
		//std::cout << "called update at time " << currentTime << "\n";
		//currentTime += 8/1000.0f;
	}
	
}

std::pair<Vector2, int> get2PerfectCircle(Vector2 &p1, Vector2 &p2, Vector2 &p3){
    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;
    int x3 = p3.x;
    int y3 = p3.y;
    int a = x1 * (y2 - y3) - y1 * (x2 - x3) + x2 * y3 - x3 * y2;
    int b = (x1 * x1 + y1 * y1) * (y3 - y2) + (x2 * x2 + y2 * y2) * (y1 - y3) + (x3 * x3 + y3 * y3) * (y2 - y1);
    int c = (x1 * x1 + y1 * y1) * (x2 - x3) + (x2 * x2 + y2 * y2) * (x3 - x1) + (x3 * x3 + y3 * y3) * (x1 - x2);
    float x = (float)-b / (2.0f * (float)a);
    float y = (float)-c / (2.0f * (float)a);
    return std::make_pair(Vector2{x,y}, sqrt((x - x1) * (x - x1) + (y - y1) *(y - y1)));
}

Vector2 get2BezierPoint(std::vector<Vector2> &points, int numPoints, float t){
    Vector2* tmp = new Vector2[numPoints];
    for(size_t i = 0; i < points.size(); i++){
        tmp[i] = points[i];
    }
    int i = numPoints - 1;
    while (i > 0) {
        for (int k = 0; k < i; k++)
            tmp[k] = Vector2{tmp[k].x + t *(tmp[k+1].x - tmp[k].x),tmp[k].y + t *(tmp[k+1].y - tmp[k].y)};
        i--;
    }
    Vector2 answer = tmp[0];
    delete[] tmp;
    return answer;
}

//load the beatmap
void GameManager::loadDefaultSkin(std::string filename){
	currentComboIndex = 0;
	std::vector<std::string> files;
	files.clear();
	Global.Path = Global.GamePath + "/resources/default_skin/";
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0){
				UnloadTexture(&hitCircleOverlay);
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hitcircleselect.png", 0) == 0){
				UnloadTexture(&selectCircle);
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hitcircle.png", 0) == 0){
				UnloadTexture(&hitCircle);
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("approachcircle.png", 0) == 0){
				UnloadTexture(&approachCircle);
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit300k.png", 0) == 0)
				;
			else if(files[i].rfind("hit300.png", 0) == 0){
				UnloadTexture(&hit300);
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit100k.png", 0) == 0)
				;
			else if(files[i].rfind("hit100.png", 0) == 0){
				UnloadTexture(&hit100);
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit50k.png", 0) == 0)
				;
			else if(files[i].rfind("hit50.png", 0) == 0){
				UnloadTexture(&hit50);
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit0.png", 0) == 0){
				UnloadTexture(&hit0);
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderscorepoint.png", 0) == 0){
				UnloadTexture(&sliderscorepoint);
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderfollowcircle.png", 0) == 0){
				UnloadTexture(&sliderfollow);
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderb0.png", 0) == 0){
				UnloadTexture(&sliderb);
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("reversearrow.png", 0) == 0){
				UnloadTexture(&reverseArrow);
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-circle.png", 0) == 0){
				UnloadTexture(&spinnerCircle);
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre.png", 0) == 0){
				UnloadTexture(&spinnerMetre);
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-bottom.png", 0) == 0){
				UnloadTexture(&spinnerBottom);
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-top.png", 0) == 0){
				UnloadTexture(&spinnerTop);
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-approachcircle.png", 0) == 0){
				UnloadTexture(&spinnerApproachCircle);
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j)) + ".").c_str(), 0) == 0){
						UnloadTexture(&numbers[j]);
    					numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
					}
				}
			}
		}
	}
}

void GameManager::loadDefaultSound(std::string filename){

}

void GameManager::loadGameSkin(std::string filename){
	temprenderSpinnerCircle = false;
	temprenderSpinnerMetre = false;
	temprenderSpinnerBack = false;

	std::vector<std::string> files;
	files.clear();
	Global.Path = Global.GamePath + "/resources/skin/";
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0){
				UnloadTexture(&hitCircleOverlay);
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hitcircleselect.png", 0) == 0){
				UnloadTexture(&selectCircle);
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hitcircle.png", 0) == 0){
				UnloadTexture(&hitCircle);
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("approachcircle.png", 0) == 0){
				UnloadTexture(&approachCircle);
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit300k.png", 0) == 0)
				;
			else if(files[i].rfind("hit300.png", 0) == 0){
				UnloadTexture(&hit300);
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit100k.png", 0) == 0)
				;
			else if(files[i].rfind("hit100.png", 0) == 0){
				UnloadTexture(&hit100);
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit50k.png", 0) == 0)
				;
			else if(files[i].rfind("hit50.png", 0) == 0){
				UnloadTexture(&hit50);
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit0.png", 0) == 0){
				UnloadTexture(&hit0);
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderscorepoint.png", 0) == 0){
				UnloadTexture(&sliderscorepoint);
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderfollowcircle.png", 0) == 0){
				UnloadTexture(&sliderfollow);
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderb0.png", 0) == 0){
				UnloadTexture(&sliderb);
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("reversearrow.png", 0) == 0){
				UnloadTexture(&reverseArrow);
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-circle.png", 0) == 0){
				UnloadTexture(&spinnerCircle);
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre.png", 0) == 0){
				UnloadTexture(&spinnerMetre);
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-bottom.png", 0) == 0){
				UnloadTexture(&spinnerBottom);
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-top.png", 0) == 0){
				UnloadTexture(&spinnerTop);
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-approachcircle.png", 0) == 0){
				UnloadTexture(&spinnerApproachCircle);
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j)) + ".").c_str(), 0) == 0){
						UnloadTexture(&numbers[j]);
    					numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
					}
				}
			}
		}
	}
}

void GameManager::loadGameSound(std::string filename){

}

void GameManager::loadBeatmapSkin(std::string filename){
	std::vector<std::string> files;
	files.clear();
	Global.Path = lastPath + '/';
	
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());
	temprenderSpinnerCircle = false;
	temprenderSpinnerMetre = false;
	temprenderSpinnerBack = false;
	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0){
				UnloadTexture(&hitCircleOverlay);
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hitcircleselect.png", 0) == 0){
				UnloadTexture(&selectCircle);
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hitcircle.png", 0) == 0){
				UnloadTexture(&hitCircle);
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("approachcircle.png", 0) == 0){
				UnloadTexture(&approachCircle);
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit300k.png", 0) == 0)
				;
			else if(files[i].rfind("hit300.png", 0) == 0){
				UnloadTexture(&hit300);
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit100k.png", 0) == 0)
				;
			else if(files[i].rfind("hit100.png", 0) == 0){
				UnloadTexture(&hit100);
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit50k.png", 0) == 0)
				;
			else if(files[i].rfind("hit50.png", 0) == 0){
				UnloadTexture(&hit50);
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("hit0.png", 0) == 0){
				UnloadTexture(&hit0);
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderscorepoint.png", 0) == 0){
				UnloadTexture(&sliderscorepoint);
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderfollowcircle.png", 0) == 0){
				UnloadTexture(&sliderfollow);
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("sliderb0.png", 0) == 0){
				UnloadTexture(&sliderb);
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("reversearrow.png", 0) == 0){
				UnloadTexture(&reverseArrow);
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-circle.png", 0) == 0){
				UnloadTexture(&spinnerCircle);
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre.png", 0) == 0){
				UnloadTexture(&spinnerMetre);
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-bottom.png", 0) == 0){
				UnloadTexture(&spinnerBottom);
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-top.png", 0) == 0){
				UnloadTexture(&spinnerTop);
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			}
			else if(files[i].rfind("spinner-approachcircle.png", 0) == 0){
				UnloadTexture(&spinnerApproachCircle);
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			}
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j)) + ".").c_str(), 0) == 0){
						UnloadTexture(&numbers[j]);
    					numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
					}
				}
			}
		}
	}
}

void GameManager::loadBeatmapSound(std::string filename){

}

void GameManager::loadGame(std::string filename){
	//create a parser and parse the file
	currentBackgroundTexture = "";

	//misc variables
	spawnedHitObjects = 0;
	Parser parser = Parser();
	Global.loadingState = 5;
	gameFile.configGeneral["SampleSet"] = "Normal";
	std::cout << "Parsing game!" << std::endl;
	gameFile = parser.parse(filename);
    std::cout << "Found " << gameFile.hitObjects.size() << " HitObjects and " << gameFile.timingPoints.size() << " Timing Points!" << std::endl;
	Global.loadingState = 1;
	Global.numberLines = gameFile.hitObjects.size();
    Global.parsedLines = 0;

	//reverse the hitobject array because we need it reversed for it to make sense (and make it faster because pop_back)
	lastTimingLoc = gameFile.timingPoints.size() - 1; 
	lastCurrentTiming = gameFile.timingPoints.size() - 1; 

	std::string temporaryPath = filename;
	while(true){
		if(temporaryPath.size() == 0)
			break;
		if(temporaryPath[temporaryPath.size() - 1] == '/')
			break;
		temporaryPath.pop_back();
	}
	if(temporaryPath.size() != 0){
		temporaryPath.pop_back();
	}

	GamePathWithSlash = temporaryPath + '/';

	lastPath = temporaryPath;

	if(std::stof(gameFile.configDifficulty["ApproachRate"]) < 5.0f){
		gameFile.preempt = 1200.0f + 600.0f * (5.0f - std::stof(gameFile.configDifficulty["ApproachRate"])) / 5.0f;
		gameFile.fade_in = 800.0f + 400.0f * (5.0f - std::stof(gameFile.configDifficulty["ApproachRate"])) / 5.0f;
	}
	else if(std::stof(gameFile.configDifficulty["ApproachRate"]) > 5.0f){
		gameFile.preempt = 1200.0f - 750.0f * (std::stof(gameFile.configDifficulty["ApproachRate"]) - 5.0f) / 5.0f;
		gameFile.fade_in = 800.0f - 500.0f * (std::stof(gameFile.configDifficulty["ApproachRate"]) - 5.0f) / 5.0f;
	}
	else{
		gameFile.preempt = 1200.0f;
		gameFile.fade_in = 800.0f;
	}

	float stackL = std::stof(gameFile.configGeneral["StackLeniency"]);
	stackL = clip(stackL, 0.0f, 1.0f);

	//---------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------|LOADING BACKGROUNDS|------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------

	//clear all the backgrounds we have
	backgroundTextures.data.clear();
	backgroundTextures.pos.clear();
	backgroundTextures.loaded.clear();

	//check for png backgrounds
	std::vector<std::string> files;
	files.clear();
	Global.Path = lastPath + '/';
	files = ls(".png");

	float circlesize2 = 54.4f - (4.48f * std::stof(gameFile.configDifficulty["CircleSize"]));

	//precalculate all the sliders and check how long we need to wait for it
	std::cout << "Loading sliders..." << std::endl;
	double start = getTimer();
	int amountOfSliders = 0;
	for(int i = 0; i < gameFile.hitObjects.size(); i++){
		if(gameFile.hitObjects[i].type == 2){
			std::vector<Vector2> edgePoints; 
			edgePoints.reserve(gameFile.hitObjects[i].curvePoints.size() + 1);
			edgePoints.push_back(Vector2{(float)gameFile.hitObjects[i].x, (float)gameFile.hitObjects[i].y});
			float resolution = gameFile.hitObjects[i].length;
    		float currentResolution = 0;
    		float lengthScale, totalLength = 0;
			
			for(size_t j = 0; j < gameFile.hitObjects[i].curvePoints.size(); j++)
        		edgePoints.push_back(Vector2{(float)gameFile.hitObjects[i].curvePoints[j].first, (float)gameFile.hitObjects[i].curvePoints[j].second});
			
			if(edgePoints.size() == 2 and gameFile.hitObjects[i].curveType == 'B'){
				gameFile.hitObjects[i].curveType == 'L';
			}


			

			if(gameFile.hitObjects[i].curveType == 'L'){
				std::vector<float> lineLengths;
				//std::cout << "will calculate linear slider id " << i << " at time " << gameFile.hitObjects[i].time << std::endl;
				lineLengths.reserve(edgePoints.size());
				for(size_t j = 0; j < edgePoints.size()-1; j++)
					lineLengths.push_back(std::sqrt(std::pow(std::abs(edgePoints[j].x - edgePoints[j+1].x),2)+std::pow(std::abs(edgePoints[j].y - edgePoints[j+1].y),2)));
				for(size_t j = 0; j < lineLengths.size(); j++)
					totalLength += lineLengths[j];
				float angle = atan2(edgePoints[edgePoints.size()-1].y - edgePoints[edgePoints.size()-2].y, edgePoints[edgePoints.size()-1].x - edgePoints[edgePoints.size()-2].x) * 180 / 3.14159265;
				float hipotenus = gameFile.hitObjects[i].length - totalLength;
				float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
				float ydiff = sqrt(std::abs(hipotenus*hipotenus-xdiff*xdiff));
				int ything = 1;
				if(angle < 0.0f){
					ything = -1;
				}
				else if(angle == 0.0f){
					ything = 0;
				}

				Vector2 extraPosition = {edgePoints[edgePoints.size()-1].x + xdiff, edgePoints[edgePoints.size()-1].y - ydiff * (float)ything};
				
				gameFile.hitObjects[i].totalLength = totalLength;
				gameFile.hitObjects[i].lengths = lineLengths;
				gameFile.hitObjects[i].extraPos = extraPosition;
			}
			if(gameFile.hitObjects[i].curveType == 'P'){
				if((edgePoints[0].x == edgePoints[2].x and edgePoints[0].y == edgePoints[2].y) or (edgePoints[1].x == edgePoints[2].x and edgePoints[1].y == edgePoints[2].y) or (edgePoints[0].x == edgePoints[1].x and edgePoints[0].y == edgePoints[1].y)){
					gameFile.hitObjects[i].curveType = 'L';
					std::vector<float> lineLengths;
					//std::cout << "will calculate linear slider id " << i << " at time " << gameFile.hitObjects[i].time << std::endl;
					lineLengths.reserve(edgePoints.size());
					for(size_t j = 0; j < edgePoints.size()-1; j++)
						lineLengths.push_back(std::sqrt(std::pow(std::abs(edgePoints[j].x - edgePoints[j+1].x),2)+std::pow(std::abs(edgePoints[j].y - edgePoints[j+1].y),2)));
					for(size_t j = 0; j < lineLengths.size(); j++)
						totalLength += lineLengths[j];
					float angle = atan2(edgePoints[edgePoints.size()-1].y - edgePoints[edgePoints.size()-2].y, edgePoints[edgePoints.size()-1].x - edgePoints[edgePoints.size()-2].x) * 180 / 3.14159265;
					float hipotenus = gameFile.hitObjects[i].length - totalLength;
					float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
					float ydiff = sqrt(std::abs(hipotenus*hipotenus-xdiff*xdiff));
					int ything = 1;
					if(angle < 0.0f){
						ything = -1;
					}
					else if(angle == 0.0f){
						ything = 0;
					}

					Vector2 extraPosition = {edgePoints[edgePoints.size()-1].x + xdiff, edgePoints[edgePoints.size()-1].y - ydiff * (float)ything};
					
					gameFile.hitObjects[i].totalLength = totalLength;
					gameFile.hitObjects[i].lengths = lineLengths;
					gameFile.hitObjects[i].extraPos = extraPosition;
				}
				else{
					std::pair<Vector2, float> circleData = get2PerfectCircle(edgePoints[0], edgePoints[1], edgePoints[2]);
					float inf = std::numeric_limits<float>::infinity();
					if(circleData.first.x == -inf or circleData.first.x == inf or circleData.first.y == -inf or circleData.first.y == inf){
						std::vector<float> lineLengths;
						//std::cout << "will calculate linear slider id " << i << " at time " << gameFile.hitObjects[i].time << std::endl;
						for(size_t j = 0; j < edgePoints.size()-1; j++)
							lineLengths.push_back(std::sqrt(std::pow(std::abs(edgePoints[j].x - edgePoints[j+1].x),2)+std::pow(std::abs(edgePoints[j].y - edgePoints[j+1].y),2)));
						for(size_t j = 0; j < lineLengths.size(); j++)
							totalLength += lineLengths[j];
						float angle = atan2(edgePoints[edgePoints.size()-1].y - edgePoints[edgePoints.size()-2].y, edgePoints[edgePoints.size()-1].x - edgePoints[edgePoints.size()-2].x) * 180 / 3.14159265;
						float hipotenus = gameFile.hitObjects[i].length - totalLength;
						float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
						float ydiff = sqrt(std::abs(hipotenus*hipotenus-xdiff*xdiff));
						int ything = 1;
						if(angle < 0.0f){
							ything = -1;
						}
						else if(angle == 0.0f){
							ything = 0;
						}

						Vector2 extraPosition = {edgePoints[edgePoints.size()-1].x + xdiff, edgePoints[edgePoints.size()-1].y - ydiff * (float)ything};
						
						gameFile.hitObjects[i].curveType = 'L';

						gameFile.hitObjects[i].totalLength = totalLength;
						gameFile.hitObjects[i].lengths = lineLengths;
						gameFile.hitObjects[i].extraPos = extraPosition;
					}
					else{
						//std::cout << "will NOT calculate perfect circle slider id " << i << " at time " << gameFile.hitObjects[i].time << std::endl;
					}
				}
			}
			if(gameFile.hitObjects[i].curveType == 'B'){
				bool old = true;
				if(old){
					//std::cout << "will calculate bezier slider id " << i << " at time " << gameFile.hitObjects[i].time << std::endl;
					std::vector<Vector2> tempEdges;
					std::vector<Vector2> tempRender;
					std::vector<float> curveLengths;
					double totalCalculatedLength = 0;
					int curves = 0;
					for(size_t j = 0; j < edgePoints.size(); j++){
						if(j == edgePoints.size()-1 || (edgePoints[j].x == edgePoints[j+1].x && edgePoints[j].y == edgePoints[j+1].y)){
							curves++;
						}
					}
					curveLengths.reserve(curves);
					for(size_t k = 0; k < edgePoints.size(); k++){
						tempEdges.push_back(edgePoints[k]);
						if(k == edgePoints.size()-1 || (edgePoints[k].x == edgePoints[k+1].x && edgePoints[k].y == edgePoints[k+1].y)){
							currentResolution = 0;
							int num = tempEdges.size();
							num = std::min((int)(gameFile.hitObjects[i].length/curves), 50);
							num = std::max(num, 2);
							int m = 0;
							float tempLength = 0;
							Vector2 lasttmp;
							while(true){
								if(currentResolution > num)
									break;
								float j = (float)currentResolution / (float)num;
								
								Vector2 tmp = get2BezierPoint(tempEdges, tempEdges.size(), j);
								if(m >= 1)
									tempLength += std::sqrt(std::pow(lasttmp.x - tmp.x,2) + std::pow(lasttmp.y - tmp.y,2));
								lasttmp = tmp;
								currentResolution++;
								m++;
							}
							curveLengths.push_back(tempLength);
							//if(temp)
							// sometimes + 1 is better?????
							totalCalculatedLength += tempLength;
							
							/*if(gameFile.hitObjects[i].time == 436){
								std::cout << "calculated curves: " << curveLengths.size() << " length: " << tempLength << "edges: ";
								for(int p = 0; p < tempEdges.size(); p++){
									std::cout << tempEdges[p].y << " ";
								}
								std::cout << std::endl;
								//SleepInMs(100);
							}*/
							tempEdges.clear();
						}
					}
					
					gameFile.hitObjects[i].totalLength = totalCalculatedLength;
					gameFile.hitObjects[i].lengths = curveLengths;
					curveLengths.clear();
					tempEdges.clear();
					tempRender.clear();
				}
				else{

				}
			}
			edgePoints.clear();
		}
		/*if(i > 0){
			if(std::abs(gameFile.hitObjects[i - 1].time - gameFile.hitObjects[i].time) < gameFile.preempt * stackL){
				int number = 1;
				int currentX = gameFile.hitObjects[i].x;
				int currentY = gameFile.hitObjects[i].y;
				while(true){
					if(i - number < 0){
						break;
					}
					if(std::abs(gameFile.hitObjects[i - number].time - gameFile.hitObjects[i - (number - 1)].time) >= gameFile.preempt * stackL){
						break;
					}
					if((int)gameFile.hitObjects[i - number].x != currentX or (int)gameFile.hitObjects[i - number].t != currentY){

					}
					gameFile.hitObjects[i - number].x = gameFile.hitObjects[i - number].x - number * (circlesize2 / 6.0f);
					gameFile.hitObjects[i - number].y = gameFile.hitObjects[i - number].y - number * (circlesize2 / 6.0f);
					number++;
				}
			}
		}
		//THIS IS A REALLY BAD WAY OF DOING THIS, NEEDS REWRITING
		*/
		//std::cout << "Calculated object at time: " << gameFile.hitObjects[i].time << std::endl;
		amountOfSliders++;
	}
	std::cout << amountOfSliders << " Sliders precalculated in " << getTimer() - start << "ms" << std::endl;
	Global.loadingState = 2;
	
	

	
	//calculate all the variables for the game (these are mathematically correct but they feel weird?)
	
	//TODO: Spinners are still in their initial state, this is probably pretty wrong
	float od = std::stoi(gameFile.configDifficulty["OverallDifficulty"]);
	if(od < 5){
		spinsPerSecond = 5.0f - 2.0f * (5.0f - od) / 5.0f;
	}
	else if(od > 5){
		spinsPerSecond = 5.0f - 2.0f * (od - 5.0f) / 5.0f;
	}
	else{
		spinsPerSecond = 5.0f;
	}
	
	//calculate the time windows for 300-100-50 points
	gameFile.p300Final = gameFile.p300 - std::stof(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p300Change;
	gameFile.p100Final = gameFile.p100 - std::stof(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p100Change;
	gameFile.p50Final = gameFile.p50 - std::stof(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p50Change;

	//debug lines
	//std::cout << gameFile.p300Final << " " << gameFile.p100Final << " " << gameFile.p50Final << " " << gameFile.preempt << std::endl;
	//std::cout << std::stof(gameFile.configDifficulty["OverallDifficulty"]) << " " << gameFile.configDifficulty["OverallDifficulty"] << std::endl;
	//std::cout << std::stof(gameFile.configDifficulty["ApproachRate"]) << " " << gameFile.configDifficulty["ApproachRate"] << std::endl;
	
	//debug, just say what the name of the music file is and load it
	

	//get the file size of the music file and allocate memory for it
	/*FILE *music = fopen((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str(), "rb");
	fseek(music, 0, SEEK_END);
	musicSize = ftell(music);
	fseek(music, 0, SEEK_SET);  
	musicData = (char *)malloc(musicSize + 1);
	fread(musicData, musicSize, 1, music);
	fclose(music);
	musicData[musicSize] = 0;*/
	//load the music as a raylib music file


	//backgroundMusic = LoadMusicStreamFromMemory(GetFileExtension((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str()), (const unsigned char *)musicData, musicSize);
	
	//free(musicData);


	Global.loadingState = 6;

	//reset the score and the combo
	score = 0;
	clickCombo = 0;
	maxCombo = 0;
	hit300s = 0;
	hit100s = 0;
	hit50s = 0;
	hit0s = 0;


    //TODO: these are not used right now, USE THEM
	float hpdrainrate = std::stof(gameFile.configDifficulty["HPDrainRate"]);

	//circle size calculations
	circlesize = 54.4f - (4.48f * std::stof(gameFile.configDifficulty["CircleSize"]));
	slidertickrate = std::stof(gameFile.configDifficulty["SliderTickRate"]);
	circlesize *= 2.0f;
	
	//more difficulty stuff, may also be wrong
	float overalldifficulty = std::stof(gameFile.configDifficulty["OverallDifficulty"]);
	difficultyMultiplier = ((hpdrainrate + std::stof(gameFile.configDifficulty["CircleSize"]) + std::stof(gameFile.configDifficulty["OverallDifficulty"]) + clip((float)gameFile.hitObjects.size() / GetMusicTimeLength(&backgroundMusic) * 8.f, 0.f, 16.f)) / 38.f * 5.f);
	if (gameFile.configDifficulty.find("SliderMultiplier") != gameFile.configDifficulty.end())
		sliderSpeed = std::stof(gameFile.configDifficulty["SliderMultiplier"]);

	
	



	
	Global.GameTextures = 2;
	/*GameManager::loadDefaultSkin(filename); // LOADING THE DEFAULT SKIN USING A SEPERATE FUNCTION
	GameManager::loadGameSkin(filename); // LOADING THE GAME SKIN USING A SEPERATE FUNCTION
	if(!IsKeyDown(KEY_S)){
		GameManager::loadBeatmapSkin(filename); // LOADING THE BEATMAP SKIN USING A SEPERATE FUNCTION
	}*/



	//spinner debug lines
	if(temprenderSpinnerCircle == true){
		renderSpinnerCircle = true;
		//std::cout << "================================== RENDERING THE SPINNER CIRCLE ==================================\n";
	}
	else{
		renderSpinnerCircle = false;
	}
	if(temprenderSpinnerMetre == true){
		renderSpinnerMetre = true;
		//std::cout << "=================================== RENDERING THE SPINNER METRE ==================================\n";
	}
	else{
		renderSpinnerMetre = false;
	}
	if(temprenderSpinnerBack == true){
		renderSpinnerBack = true;
		//std::cout << "================================ RENDERING THE SPINNER BACKGROUND ================================\n";
	}
	else{
		renderSpinnerBack = false;
	}

	//load all of the hitsounds into memory


	//followpoint precalculations/creation. These values are wrong.
	//TODO: fix this bs
	std::reverse(gameFile.timingPoints.begin(),gameFile.timingPoints.end());
	std::reverse(gameFile.events.begin(),gameFile.events.end());

	//create temporary timing points for the follow points (useful for sliders)
    timingSettings tempTiming;
    std::vector<timingSettings> times;
    double preCalcLength;
    for(int i = gameFile.timingPoints.size()-1; i >= 0; i--){
        tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
        tempTiming.sliderSpeedOverride = 1;
        tempTiming.time = gameFile.timingPoints[i].time;
        double tempBeatLength;
        tempBeatLength = gameFile.timingPoints[i].beatLength;
        if(tempBeatLength >= 0){
            tempTiming.beatLength = tempBeatLength;
            preCalcLength = tempBeatLength;
            tempTiming.sliderSpeedOverride = 1;
        }
        if(tempBeatLength < 0){
            tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
            tempTiming.beatLength = preCalcLength;
        }
        tempTiming.meter = gameFile.timingPoints[i].meter;
        tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
        tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
        tempTiming.volume = gameFile.timingPoints[i].volume;
        tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
        tempTiming.effects = gameFile.timingPoints[i].effects;
		//std::cout << "push timing point at: " << tempTiming.time << "\n";
        times.push_back(tempTiming);
    }
	
	int index = 0;

	//fade in time for followpoints, this part i dont really understand since its not really documented
	
	float followPointFadeTime = gameFile.preempt - gameFile.fade_in;
	followLines.clear();
	for(int i = 1; i < gameFile.hitObjects.size(); i++){
		//float templength = (data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed * data.timing.sliderSpeedOverride) * data.slides; //slider length
		if(gameFile.hitObjects[i].startingACombo == false and gameFile.hitObjects[i - 1].type != 3){
			FollowPoint tempPoint;
			tempPoint.endTime = gameFile.hitObjects[i].time;
			tempPoint.endTime2 = gameFile.hitObjects[i].time + followPointFadeTime;
			tempPoint.endX = gameFile.hitObjects[i].x;
			tempPoint.endY = gameFile.hitObjects[i].y;
			
			if(gameFile.hitObjects[i - 1].type == 2){
				/*if(data.slides % 2 == 0){
					data.ex = data.x;
					data.ey = data.y;
				}
				else{
					data.ex = renderPoints[renderPoints.size() - 1].x;
					data.ey = renderPoints[renderPoints.size() - 1].y;
				}*/
				HitObjectData tempData = gameFile.hitObjects[i - 1];
				while(true){
					if(index + 1 > times.size() - 1)
						break;
					if(times[index + 1].time > tempData.time)
						break;
					index++;
				}

				//--------------------------------------------- STANDART SLIDER PROCEDURE ---------------------------------------------
				
				tempData.timing.beatLength = times[index].beatLength;
				tempData.timing.sliderSpeedOverride = times[index].sliderSpeedOverride;
				//std::vector<int> output = sliderPreInit(tempData);
				int *output = sliderPreInit(tempData);

				tempPoint.startTime = output[2] - followPointFadeTime;
				tempPoint.startTime2 = output[2];
				tempPoint.startX = output[0];
				tempPoint.startY = output[1];

				free(output);

				//std::cout << "done calculation of follow line starting from slider at time: " << tempData.time << "\n";
				//--------------------------------------------- STANDART SLIDER PROCEDURE ---------------------------------------------

			}
			else{
				tempPoint.startTime = gameFile.hitObjects[i - 1].time - followPointFadeTime;
				tempPoint.startTime2 = gameFile.hitObjects[i - 1].time;
				tempPoint.startX = gameFile.hitObjects[i - 1].x;
				tempPoint.startY = gameFile.hitObjects[i - 1].y;
			}
			tempPoint.distance = std::sqrt(std::pow(std::abs(tempPoint.startX - tempPoint.endX),2) + std::pow(std::abs(tempPoint.startY - tempPoint.endY),2));
			int numberOfPoints = tempPoint.distance / 32.0f;
			float offset = (tempPoint.distance - numberOfPoints * 32.0f) / 2.0f;
			for(int i = 0; i < numberOfPoints; i++){
				float loc = offset + 16.0f + i * 32.0f;
				loc = loc / tempPoint.distance;
				Vector2 tempData = lerp({tempPoint.startX, tempPoint.startY}, {tempPoint.endX, tempPoint.endY}, loc);
				tempPoint.points.push_back({tempData.x, tempData.y, loc});
			}
			followLines.push_back(tempPoint);
			//std::cout << "followLine from between times " << tempPoint.startTime << " - " << tempPoint.endTime << " and between cords " << tempPoint.startX << ", " << tempPoint.startY << " - " << tempPoint.endX << ", " << tempPoint.endY << "\n";
		}
	}


	std::reverse(followLines.begin(),followLines.end());
	std::reverse(gameFile.hitObjects.begin(),gameFile.hitObjects.end());

	//reverse(times.begin(), times.end());

	defaultSampleSet = 0;
	if(gameFile.configGeneral["SampleSet"] == "Soft"){
		defaultSampleSet = 1;
	}
	else if(gameFile.configGeneral["SampleSet"] == "Drum"){
		defaultSampleSet = 2;
	}

	SoundFiles.data.clear();
	SoundFiles.loaded.clear();
	loadGameSounds();

	Global.Path.pop_back();
	backgroundMusic = LoadMusicStream((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str());



	std::cout << "Free Vram: " << _os_get_free_vram() << std::endl;
	std::cout << "Free M_ALL: " << _os_get_free_ram(MEMREGION_ALL) << "/" << _os_get_size_ram(MEMREGION_ALL) << std::endl;
	std::cout << "Free M_APP: " << _os_get_free_ram(MEMREGION_APPLICATION) << "/" << _os_get_size_ram(MEMREGION_APPLICATION) << std::endl;
	std::cout << "Free M_SYS: " << _os_get_free_ram(MEMREGION_SYSTEM) << "/" << _os_get_size_ram(MEMREGION_SYSTEM) << std::endl;
	std::cout << "Free M_BSE: " << _os_get_free_ram(MEMREGION_BASE) << "/" << _os_get_size_ram(MEMREGION_BASE) << std::endl;
	std::cout << "Free M_LIN: " << _os_get_free_linear_ram() << "/" << Global.linearSpaceFree << std::endl;




	//SleepInMs(5000);

	Global.loadingState = 3;

	Global.loadingState = 7;

	Global.Path = lastPath;
	Global.GameTextures = 1;
	
	startMusic = true;
	stop = false;
}

void GameManager::unloadGame(){
	//std::cout << "UnloadingGame" << std::endl;
	currentComboIndex = 0;
	Global.GameTextures = -1;
	SleepInMs(20);
	for(auto& pair : SoundFiles.data) {
    	UnloadSound(&pair.second);
  	}

	for(auto& pair : SoundFilesAll.data) {
    	UnloadSound(&pair.second);
  	}
	/*for(int i = objects.size()-1; i >= 0; i--){
		destroyHitObject(i);
	}*/
	
	UnloadMusicStream(&backgroundMusic);
	Global.numberLines = -1;
    Global.parsedLines = -1;

	SoundFiles.data.clear();
	SoundFiles.loaded.clear();
	//Global.mutex.unlock();
	//LightLock_Unlock(&Global.lightlock);

	MutexUnlock(ACCESSING_OBJECTS);
	MutexUnlock(RENDER_BLOCK);
	std::cout << "unlocking the hold access lock\n";
	//MutexUnlock(SWITCHING_STATE);
	while(true){
		SleepInMs(500);
		std::cout << "waiting for textures to unload\n";
		if(Global.GameTextures == 15)
			break;
		
	}
	//MutexLock(SWITCHING_STATE);
	MutexLock(ACCESSING_OBJECTS);

	//Global.mutex.lock();
	//LightLock_Lock(&Global.lightlock);


	gameFile.hitObjects.clear();
	gameFile.timingPoints.clear();
	gameFile.followPoints.clear();
	gameFile.events.clear();
	
	gameFile.hitObjects = std::vector<HitObjectData>();
	gameFile.timingPoints = std::vector<TimingPoint>();
	gameFile.followPoints = std::vector<FollowPoint>();
	gameFile.events = std::vector<Event>();


	followLines.clear();

	followLines = std::vector<FollowPoint>();

	
	//Global.mutex2.lock();
	while(true){
		if(objectsLinkedList.getHead() == NULL)
			break;
		((HitObject*)objectsLinkedList.getHead()->object)->deinit();
		delete objectsLinkedList.getHead()->object;
		objectsLinkedList.deleteHead();
	}
	while(true){
		if(deadObjectsLinkedList.getHead() == NULL)
			break;
		((HitObject*)deadObjectsLinkedList.getHead()->object)->deinit();
		delete deadObjectsLinkedList.getHead()->object;
		deadObjectsLinkedList.deleteHead();
	}
	
	//Global.mutex2.unlock();
}

void GameManager::spawnHitObject(HitObjectData data){
	//spawn a new hitobject, the cool way
	HitObject *temp;
	if(data.type == 1){
		temp = new Circle(data);
	}
	else if(data.type == 2){
		temp = new Slider(data);
	}
	else if(data.type == 3){
		temp = new Spinner(data);
	}
	else {
		temp = new Circle(data);
	}

	objectsLinkedList.insertTail((void*)temp);
	//std::cout << " + size of linked list: " << objectsLinkedList.getSize() << std::endl;
    //objects.push_back(temp);
}

void GameManager::destroyHitObject(Node *node){
	//declare a hitobject dead
	((HitObject*)node->object)->data.destruct = false;
	//dead_objects.push_back((HitObject*)node->object);
	deadObjectsLinkedList.insertTail((void*)node->object);
	objectsLinkedList.deleteNodeUnsafe(node);
	//std::cout << " - size of linked list: " << objectsLinkedList.getSize() << std::endl;
	//objects.erase(objects.begin()+index);
}

void GameManager::destroyDeadHitObject(Node *node){
	//somehow "kill" the "dead" object
	/*if(dead_objects[index]->data.type == 2){
		Slider* tempslider = dynamic_cast<Slider*>(dead_objects[index]);
		if(tempslider->data.textureLoaded == true and tempslider->data.textureReady == false){
			tempslider->renderedLocations.clear();
        	tempslider->renderPoints.clear();
			delete dead_objects[index];
			dead_objects.erase(dead_objects.begin()+index);
		}
	}*/
	//else{
	((HitObject*)node->object)->deinit();
	delete node->object;
	deadObjectsLinkedList.deleteNodeUnsafe(node);
	//delete dead_objects[index];
	//dead_objects.erase(dead_objects.begin()+index);
	//}
}

void GameManager::render_points(){
	//garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
    
}

void GameManager::render_combo(){
	//garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
    
}

void GameManager::loadGameTextures(){
	
	Image tempImage = LoadImage((Global.GamePath + "/resources/sliderin.png").c_str());
	ImageColorReplace(&tempImage, {255,255,255,159}, {255,255,255,0});
	sliderin = LoadTextureFromImage(&tempImage);
	UnloadImage(&tempImage);
	std::cout << "loaded tempImage to memory done, press select to continue" << std::endl;
	Image tempImage2 = GenImageGradientRadial(sliderin.width, sliderin.height, 0.1, {255,0,0,255}, {0,0,0,0});
	sliderblank = LoadTexture((Global.GamePath + "/resources/SliderBlank.png").c_str());
	//sliderout = LoadTexture((Global.GamePath + "/resources/SliderBlank.png").c_str());
	sliderout = LoadTextureFromImage(&tempImage2);
	UnloadImage(&tempImage2);
    //sliderout = LoadTexture((Global.GamePath + "/resources/sliderout.png").c_str());
	
	//C2D_Prepare();
    //C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    //C2D_SceneBegin(Global.window);
	
	_gpu_start_drawing(Global.window);



	sliderInnerBall = LoadRenderTexture(64, 64);
	sliderOuterBall = LoadRenderTexture(64, 64);	
	
	BeginTextureMode(&sliderOuterBall);
    ClearBackground({0,0,0,0});
	EndBlendMode();
	//rlEnableDepthTest2();
	//rlDisableDepthTest();
	rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_MAX, RL_MAX);
	DrawCircleWithDepth((Vector2){32, 32}, 30.5, 64, 0.5f, {255, 255, 255, 255});
	DrawCircleWithDepth((Vector2){32, 32}, 31.7, 64, 0.4f, {255, 255, 255, 200});
	DrawCircleWithDepth((Vector2){32, 32}, 31.0, 64, 0.3f, {255, 255, 255, 150});
	DrawCircleWithDepth((Vector2){32, 32}, 31.3, 64, 0.2f, {255, 255, 255, 50});
	//rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_MIN, RL_MIN);
	//DrawCircleWithDepth((Vector2){32, 32}, 30*0.85f, 60, 0.5f, {0, 0, 0, 0});
	EndBlendMode();
	//rlDisableDepthTest();
    EndTextureMode();

    SetTextureFilter(&sliderOuterBall.texture, TEXTURE_FILTER_BILINEAR);
	
	BeginTextureMode(&sliderInnerBall);
    ClearBackground({0,0,0,0});
	EndBlendMode();
	rlEnableDepthTest();
	DrawCircleWithDepth((Vector2){32, 32}, 30, 60, 0.5f, {0, 0, 0, 255});
	rlDisableDepthTest();
    EndTextureMode();

    SetTextureFilter(&sliderInnerBall.texture, TEXTURE_FILTER_BILINEAR);


	//C2D_Flush();  //test
    //C3D_FrameEnd(0);
	_gpu_end_drawing();

	followPoint = LoadTexture((Global.GamePath + "/resources/followpoint.png").c_str());
    loadDefaultSkin(Global.selectedPath); // LOADING THE DEFAULT SKIN USING A SEPERATE FUNCTION
    loadGameSkin(Global.selectedPath); // LOADING THE GAME SKIN USING A SEPERATE FUNCTION
    if(!Global.settings.useDefaultSkin){
        loadBeatmapSkin(Global.selectedPath); // LOADING THE BEATMAP SKIN USING A SEPERATE FUNCTION
    }
    SetTextureFilter(&hit0, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&hit50, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&hit100, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&hit300, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&approachCircle, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&selectCircle, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&hitCircleOverlay, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&hitCircle, TEXTURE_FILTER_BILINEAR );
    SetTextureFilter(&sliderb, TEXTURE_FILTER_BILINEAR );


    SetTextureFilter(&sliderin, TEXTURE_FILTER_POINT);
	SetTextureFilter(&sliderblank, TEXTURE_FILTER_POINT);
    SetTextureFilter(&sliderout, TEXTURE_FILTER_POINT);
    SetTextureFilter(&sliderscorepoint, TEXTURE_FILTER_BILINEAR );

    SetTextureFilter(&reverseArrow, TEXTURE_FILTER_BILINEAR );
    for(int i = 0; i < 10; i++){
        SetTextureFilter(&numbers[i], TEXTURE_FILTER_BILINEAR );  //OPENGL1.1 DOESNT SUPPORT THIS
    }

    backgroundTextures.data.clear();
	backgroundTextures.pos.clear();
	backgroundTextures.loaded.clear();

	std::vector<std::string> files;
	files.clear();
    
	Global.Path = lastPath + '/';
	files = ls(".png");
	std::vector<std::string> files2 = ls(".jpg");
	std::vector<std::string> files3 = ls(".jpeg");
	files.insert(files.end(), files2.begin(), files2.end());
	files.insert(files.end(), files3.begin(), files3.end());

	for(int i = 0; i < (int)gameFile.events.size(); i++){
		if(gameFile.events[i].eventType == 0){
			std::cout << "Time: " << gameFile.events[i].startTime << "ms - Filename: " << gameFile.events[i].filename << '.' << std::endl;
			if(gameFile.events[i].startTime < 10000){
				gameFile.events[i].startTime -= 87000;
				std::cout << "Time changed to: " << gameFile.events[i].startTime << std::endl;
			}
		}
	}
	

	/*std::cout << "Found this many files: " << files.size() << std::endl;
	for(int i = 0; i < files.size(); i++){
		std::cout << files[i] << std::endl;
	}*/

	for(int i = 0; i < files.size(); i++){
		for(int j = 0; j < (int)gameFile.events.size(); j++){
			if(gameFile.events[j].eventType == 0){
				std::cout << "attempting to load a background\n";
				int t = gameFile.events[j].filename.size() - 1;
				while(gameFile.events[j].filename[t] == ' ' and t >= 0){
					gameFile.events[j].filename.pop_back();
					t--;
				}
				t = 0;
				while(gameFile.events[j].filename[t] == ' ' and gameFile.events[j].filename.size() > 0){
					gameFile.events[j].filename.erase(gameFile.events[j].filename.begin());
				}
				//std::cout << "finding function returned: " << files[i].rfind(gameFile.events[j].filename, 0) << " for: " << gameFile.events[j].filename << " and " << files[i] << std::endl;
				if(files[i].rfind(gameFile.events[j].filename, 0) == 0){
					//std::cout << "WHAT DA HEEEEEEEEEELLLLLLLLLLLLL" << std::endl;
					Image image = LoadImage((Global.Path + files[i]).c_str());
					//ImageColorBrightness(&image, -128);
					
					int divider = 4;
					while(true){
						if(image.width / divider > 256 or image.height / divider > 128){
							divider += 1;
							std::cout << "TOO BIG OF AN IMAGE!" << std::endl;
							
						}
						else{
							break;
						}
					}
					std::cout << "RESIZING TO" << (int)(image.height / divider) << " x " << (int)(image.width / divider) << std::endl;
					//std::cout << "resize" << std::endl;
					//SleepInMs(200);
					int resizeW = (int)(image.width / divider);
					int resizeH = (int)(image.height / divider);
					ImageResize(&image, resizeW, resizeH);
					//std::cout << "T " << image.width << " " << image.height << std::endl;

					ImageColorTint(&image, Color{30,30,30,255});
					ImageBlurGaussian(&image, 0.3f);

					backgroundTextures.data[gameFile.events[j].filename] = LoadTextureFromImage(&image);
					UnloadImage(&image); 
					
					backgroundTextures.pos[gameFile.events[j].filename] = {gameFile.events[j].xOffset, gameFile.events[j].yOffset};
					if(backgroundTextures.data[gameFile.events[j].filename].width != 0){
						backgroundTextures.loaded[gameFile.events[j].filename].value = true;
						std::cout << "Loaded: Background with filename: " << gameFile.events[j].filename << std::endl;
						SetTextureFilter(&backgroundTextures.data[gameFile.events[j].filename], TEXTURE_FILTER_BILINEAR) ;
					}
					
					//std::cout << "*. no backgroundss *.\n";
				}
			}
		}
	}


	

	//AAAAAAAAAAA

    Global.GameTextures = 0;
}

void GameManager::unloadGameTextures(){
	std::cout << "Unloading Textures" << std::endl;
    Global.GameTextures = 2;
    UnloadTexture(&hitCircleOverlay);
    UnloadTexture(&hitCircle);
    UnloadTexture(&sliderscorepoint);
    UnloadTexture(&approachCircle);
    UnloadTexture(&hit300);
    UnloadTexture(&hit100);
    UnloadTexture(&hit50);
    UnloadTexture(&hit0);
    UnloadTexture(&sliderb);
    UnloadTexture(&sliderin);
	UnloadTexture(&sliderblank);
    UnloadTexture(&sliderout);
    UnloadTexture(&selectCircle);
    UnloadTexture(&reverseArrow);
    UnloadTexture(&spinnerBottom);
    UnloadTexture(&spinnerTop);
    UnloadTexture(&spinnerCircle);
    UnloadTexture(&spinnerApproachCircle);
    UnloadTexture(&spinnerMetre);
	UnloadTexture(&followPoint);

	UnloadRenderTexture(&sliderInnerBall);
	UnloadRenderTexture(&sliderOuterBall);

    for(int i = 0; i < 10; i++){
        UnloadTexture(&numbers[i]);
    }
    std::string key;
    for(std::map<std::string, Texture2D>::iterator it = backgroundTextures.data.begin(); it != backgroundTextures.data.end(); ++it){
        key = it->first;
        std::cout << "Removed: " << it->first << "\n";
        UnloadTexture(&backgroundTextures.data[key]);
    }

    


	Global.GameTextures = 10;
	std::cout << "Unloading SUTUFF" << std::endl;
	backgroundTextures.data.clear();
    backgroundTextures.pos.clear();
    backgroundTextures.loaded.clear();
	//Global.mutex2.lock();
	Node * deadHitObjectNode = deadObjectsLinkedList.getHead();
	Node * deadHitObjectNodeNext;
	HitObject* deadHitObject;
	while(true){
		if(deadHitObjectNode == NULL){
			break;
		}
		deadHitObject = (HitObject*)deadHitObjectNode->object;
		deadHitObjectNodeNext = deadHitObjectNode->next;

		if(deadHitObject->data.type == 2){
			if(Slider* tempslider = dynamic_cast<Slider*>(deadHitObject)){
				tempslider->readyToDelete = true;
				std::cout << "Unloading SLIDER" << std::endl;
			}
		}
		
		deadHitObjectNode = deadHitObjectNodeNext;
	}

	Node * HitObjectNode = objectsLinkedList.getHead();
	Node * HitObjectNodeNext;
	HitObject* hitObject;
	while(true){
		if(HitObjectNode == NULL){
			break;
		}
		hitObject = (HitObject*)HitObjectNode->object;
		HitObjectNodeNext = HitObjectNode->next;

		if(hitObject->data.type == 2){
			if(Slider* tempslider = dynamic_cast<Slider*>(hitObject)){
				tempslider->readyToDelete = true;
				std::cout << "Unloading SLIDER" << std::endl;
			}
		}
		
		HitObjectNode = HitObjectNodeNext;
	}

	//Global.mutex.unlock();
    //LightLock_Unlock(&Global.lightlock);
	unloadSliderTextures();
	//Global.mutex.lock();
    //LightLock_Lock(&Global.lightlock);
	//Global.mutex2.unlock();
	Global.GameTextures = 15;
}

int orientation2(Vector2 &p1, Vector2 &p2, Vector2 &p3){
    int val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
    return (val > 0)? false: true;
}

float interpolate2(float *p, float *time, float t) {
    float L01 = p[0] * (time[1] - t) / (time[1] - time[0]) + p[1] * (t - time[0]) / (time[1] - time[0]);
    float L12 = p[1] * (time[2] - t) / (time[2] - time[1]) + p[2] * (t - time[1]) / (time[2] - time[1]);
    float L23 = p[2] * (time[3] - t) / (time[3] - time[2]) + p[3] * (t - time[2]) / (time[3] - time[2]);
    float L012 = L01 * (time[2] - t) / (time[2] - time[0]) + L12 * (t - time[0]) / (time[2] - time[0]);
    float L123 = L12 * (time[3] - t) / (time[3] - time[1]) + L23 * (t - time[1]) / (time[3] - time[1]);
    float C12 = L012 * (time[2] - t) / (time[2] - time[1]) + L123 * (t - time[1]) / (time[2] - time[1]);
    return C12;
}   

std::vector<Vector2> interpolate2(std::vector<Vector2> &points, int index, int pointsPerSegment) {
    std::vector<Vector2> result;
    float x[4];
    float y[4];
    float time[4];
    for (int i = 0; i < 4; i++) {
        x[i] = points[index + i].x;
        y[i] = points[index + i].y;
        time[i] = i;
    }
    float tstart = 1;
    float tend = 2;
    float total = 0;
    for (int i = 1; i < 4; i++) {
        float dx = x[i] - x[i - 1];
        float dy = y[i] - y[i - 1];
        total += std::pow(dx * dx + dy * dy, .25);
        time[i] = total;
    }
    tstart = time[1];
    tend = time[2];
    int segments = pointsPerSegment - 1;
    result.push_back(points[index + 1]);
    for (int i = 1; i < segments; i++) {
        float xi = interpolate2(x, time, tstart + (i * (tend - tstart)) / segments);
        float yi = interpolate2(y, time, tstart + (i * (tend - tstart)) / segments);
        result.push_back(Vector2{xi, yi});
    }
    result.push_back(points[index + 2]);
    return result;
}

std::vector<Vector2> interpolate2(std::vector<Vector2> &coordinates, float length){
    std::vector<Vector2> vertices;
    std::vector<int> pointsPerSegment;
    for (size_t i = 0; i < coordinates.size(); i++){
        vertices.push_back(coordinates[i]);
        if(i > 0)
            pointsPerSegment.push_back(distance(vertices[i], vertices[i-1]));
    }
    float lengthAll = 0;
    for(size_t i = 0; i < pointsPerSegment.size(); i++) 
        lengthAll += pointsPerSegment[i];
    for(size_t i = 0; i < pointsPerSegment.size(); i++)
        pointsPerSegment[i] *= length/lengthAll;
    float dx = vertices[1].x - vertices[0].x;
    float dy = vertices[1].y - vertices[0].y;
    float x1 = vertices[0].x - dx;
    float y1 = vertices[0].y - dy;
    Vector2 start = {x1, y1};
    int n = vertices.size() - 1;
    dx = vertices[n].x - vertices[n-1].x;
    dy = vertices[n].y - vertices[n-1].y;
    float xn = vertices[n].x + dx;
    float yn = vertices[n].y + dy;
    Vector2 end = {xn, yn};
    vertices.insert(vertices.begin(), start);
    vertices.push_back(end);
    std::vector<Vector2> result;
    for (size_t i = 0; i < vertices.size() - 3; i++) {
        std::vector<Vector2> points = interpolate2(vertices, i, pointsPerSegment[i]);
        for(size_t i = (result.size() > 0) ? 1 : 0; i < points.size(); i++)
            result.push_back(points[i]);
    }
    return result;
}

//std::vector<int> GameManager::sliderPreInit(HitObjectData data){
int * GameManager::sliderPreInit(HitObjectData data){
    bool durationNull = false;
    double templength = data.length;
    if(data.length < 1){
        data.length = 1;
        durationNull = true;
    }

	std::vector<Vector2> edgePoints;
	//std::vector<Vector2> renderPoints;
	Vector2* renderPoints;
	int renderPointsSize = 0;
    edgePoints.push_back(Vector2{(float)data.x, (float)data.y});

    float resolution = data.length;
    float currentResolution = 0;
    float lengthScale, totalLength = 0;

	Vector2 extraPosition;

    for(size_t i = 0; i < data.curvePoints.size(); i++)
        edgePoints.push_back(Vector2{(float)data.curvePoints[i].first, (float)data.curvePoints[i].second});
	
    if(edgePoints.size() == 1){
		renderPoints = (Vector2*)malloc(sizeof(Vector2) * data.length);
		renderPointsSize = data.length;
        for(int k = 0; k < data.length; k++){
            renderPoints[k] = (edgePoints[0]);
        }
    }
    else{
        if(data.curveType == 'L'){
            extraPosition = data.extraPos;
            edgePoints[edgePoints.size()-1] = extraPosition;
            data.totalLength-=data.lengths[data.lengths.size()-1];
            data.lengths[data.lengths.size()-1] = std::sqrt(std::pow(std::abs(edgePoints[edgePoints.size()-2].x - edgePoints[edgePoints.size()-1].x),2)+std::pow(std::abs(edgePoints[edgePoints.size()-2].y - edgePoints[edgePoints.size()-1].y),2));
            data.totalLength+=data.lengths[data.lengths.size()-1];
            lengthScale = data.totalLength/data.length;

			renderPoints = (Vector2*)malloc(sizeof(Vector2) * (data.length));
			renderPointsSize = data.length;
			int ending = 0;
            for(size_t i = 0; i < edgePoints.size()-1; i++){
                for(float j = 0; j < data.lengths[i] && j < renderPointsSize; j += lengthScale){
                    renderPoints[ending] = (Vector2{edgePoints[i].x + (edgePoints[i+1].x - edgePoints[i].x)*j/data.lengths[i], edgePoints[i].y + (edgePoints[i+1].y - edgePoints[i].y)*j/data.lengths[i]});
					ending++;
					if(ending >= renderPointsSize)
						break;
				}
				if(ending >= renderPointsSize)
					break;
			}
			while(ending < renderPointsSize){
				renderPoints[ending] = renderPoints[ending - 1];
				ending++;
			}
            
        }
        else if(data.curveType == 'B'){
			Vector2 edges[edgePoints.size()];
			for(size_t i = 0; i < edgePoints.size(); i++)
				edges[i] = edgePoints[i];
			std::vector<Vector2> tempEdges;
			std::vector<Vector2> tempRender;
			std::vector<float> curveLengths;
			double totalCalculatedLength = 0;
			tempEdges.clear();
			tempRender.clear();
			int curveIndex = 0;
			double currentMax = 0;
			std::vector<Vector2> samples;
			std::vector<int> indices;
			std::vector<float> lengths;
			bool first = true;
			double tempResolution;
			for(size_t i = 0; i < edgePoints.size(); i++){
				tempEdges.push_back(edgePoints[i]);
				if(i == edgePoints.size()-1 || (edgePoints[i].x == edgePoints[i+1].x && edgePoints[i].y == edgePoints[i+1].y)){
					tempResolution = data.lengths[curveIndex]; //clip(data.lengths[curveIndex], 0, 20000);
					//std::cout << "tempResolution: " << tempResolution << std::endl;
					tempResolution = std::min(data.lengths[curveIndex], 400.0f);
					//tempResolution = std::max(tempResolution, 1);
					if(tempResolution > 0 and tempEdges.size() > 1){
						if(first){
							samples.push_back(get2BezierPoint(tempEdges, tempEdges.size(), 0));
							lengths.push_back(0);
						}
						int lastk = 0;
						int k = 1;
						if(!first)
							k = 0;
						for(; k < tempResolution; k++){
							samples.push_back(get2BezierPoint(tempEdges, tempEdges.size(), ((double)k)/tempResolution));
							lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);
							lastk = k;
						}

						samples.push_back(get2BezierPoint(tempEdges, tempEdges.size(), 1));
						lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);
						if(first)
							first = false;
					}
					curveIndex++;
					tempEdges.clear();
				}
			}

			totalCalculatedLength = lengths[lengths.size() - 1];
			tempResolution = data.length;
			if(totalCalculatedLength < data.length){
				float angle = atan2(samples[samples.size()-1].y - samples[samples.size()-2].y, samples[samples.size()-1].x - samples[samples.size()-2].x) * 180 / 3.14159265;
				float hipotenus = data.length - totalCalculatedLength;
				float xdiff = hipotenus * cos(-angle * 3.14159265 / 180.0f);
				float ydiff = sqrt(std::abs(hipotenus*hipotenus-xdiff*xdiff));
				int ything = 1;
				if(angle < 0.0f){
					ything = -1;
				}
				else if(angle == 0.0f){
					ything = 0;
				}

				Vector2 extraPosition = {samples[samples.size()-1].x + xdiff, samples[samples.size()-1].y - ydiff * (float)ything};
				samples.push_back(extraPosition);
				lengths.push_back(distance(samples[samples.size() - 1], samples[samples.size() - 2]) + lengths[lengths.size() - 1]);
			}
			renderPoints = (Vector2*)malloc(sizeof(Vector2) * (data.length));
			renderPointsSize = data.length;
			int ending = 0;
			int SampleIndex = 1;
			for(int index = 0; index <= data.length; index++){
				while(index > lengths[SampleIndex]){
					if(SampleIndex == lengths.size() - 1)  
						break;
					else{
						SampleIndex++;
					}
				}
				double lerpPos = (index - lengths[SampleIndex - 1]) / (lengths[SampleIndex] - lengths[SampleIndex - 1]);
				renderPoints[ending] = (lerp(samples[SampleIndex], samples[SampleIndex - 1], lerpPos));
				ending++;
				if(ending >= renderPointsSize){
					break;
				}
			}
        }
        else if(data.curveType == 'P'){
            std::pair<Vector2, float> circleData = get2PerfectCircle(edgePoints[0], edgePoints[1], edgePoints[2]);
            float inf = std::numeric_limits<float>::infinity();
            if(circleData.first.x == -inf or circleData.first.x == inf or circleData.first.y == -inf or circleData.first.y == inf){
                extraPosition = data.extraPos;
                edgePoints[edgePoints.size()-1] = extraPosition;
                data.totalLength-=data.lengths[data.lengths.size()-1];
                data.lengths[data.lengths.size()-1] = std::sqrt(std::pow(std::abs(edgePoints[edgePoints.size()-2].x - edgePoints[edgePoints.size()-1].x),2)+std::pow(std::abs(edgePoints[edgePoints.size()-2].y - edgePoints[edgePoints.size()-1].y),2));
                data.totalLength+=data.lengths[data.lengths.size()-1];

                lengthScale = data.totalLength/data.length;
				renderPoints = (Vector2*)malloc(sizeof(Vector2) * (data.length));
				renderPointsSize = data.length;
				int ending = 0;
				for(size_t i = 0; i < edgePoints.size()-1; i++){
					for(float j = 0; j < data.lengths[i] && j < renderPointsSize; j += lengthScale){
						renderPoints[ending] = (Vector2{edgePoints[i].x + (edgePoints[i+1].x - edgePoints[i].x)*j/data.lengths[i], edgePoints[i].y + (edgePoints[i+1].y - edgePoints[i].y)*j/data.lengths[i]});
						ending++;
						if(ending >= renderPointsSize)
							break;
					}
					if(ending >= renderPointsSize)
						break;
				}
				while(ending < renderPointsSize){
					renderPoints[ending] = (edgePoints[edgePoints.size()-1]);
					ending++;
				}
            }
            else{
                Vector2 center = circleData.first;
                int radius = circleData.second;
                float degree1 = atan2(edgePoints[0].y - center.y , edgePoints[0].x - center.x) * RAD2DEG;
                float degree2 = atan2(edgePoints[1].y - center.y , edgePoints[1].x - center.x) * RAD2DEG;
                float degree3 = atan2(edgePoints[2].y - center.y , edgePoints[2].x - center.x) * RAD2DEG;
                degree1 = degree1 < 0 ? degree1 + 360 : degree1;
                degree2 = degree2 < 0 ? degree2 + 360 : degree2;
                degree3 = degree3 < 0 ? degree3 + 360 : degree3;
                bool clockwise = !orientation2(edgePoints[0], edgePoints[1], edgePoints[2]);
                float angle = (((data.length * 360) / radius ) / 3.14159265 ) / 2;
				lengthScale = data.totalLength/data.length;

				renderPoints = (Vector2*)malloc(sizeof(Vector2) * (data.length));
				renderPointsSize = data.length;
				int ending = 0;
                if(clockwise){
                    degree1 = degree1 < degree3 ? degree1 + 360 : degree1;
                    degree2 = degree2 < degree3 ? degree2 + 360 : degree2;
                    for(float i = degree1; i > degree1 - angle; i-=angle/data.length){
                        if(ending >= data.length){
                            break;
                        }
                        Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius, center.y + sin(i / RAD2DEG) * radius};
                        renderPoints[ending] = (tempPoint);
                        ending++;
                    }
                }
                else{
                    degree2 = degree2 < degree1 ? degree2 + 360 : degree2;
                    degree3 = degree3 < degree1 ? degree3 + 360 : degree3;
                    for(float i = degree1; i < degree1 + angle; i+=angle/data.length){
                        if(ending >= data.length){
                            break;
                        }
                        Vector2 tempPoint = Vector2{center.x + cos(i / RAD2DEG) * radius, center.y + sin(i / RAD2DEG) * radius};
                        renderPoints[ending] = (tempPoint);
                        ending++;
                    }
                    //std::reverse(renderPoints.begin(), renderPoints.end());
                }
				while(ending < renderPointsSize){
					renderPoints[ending] = renderPoints[ending - 1];
					ending++;
				}
                //std::cout << "Pdata: " << data.length << " size: " << renderPoints.size() << std::endl;
                
            }
            
        }
        else if(data.curveType == 'C'){
			std::vector<Vector2> amogus = interpolate2(edgePoints, data.length);
            renderPoints = (Vector2*)malloc(sizeof(Vector2) * (data.length));
			renderPointsSize = data.length;
			int ending = 0;
			for(int i = 0; i < renderPointsSize && i < amogus.size(); i++){
				renderPoints[i] = amogus[i];
				ending++;
			}
			while(ending < renderPointsSize){
				renderPoints[ending] = renderPoints[ending - 1];
				ending++;
			}
			amogus.clear();
        }
        else{
            std::__throw_invalid_argument("Invalid Slider type!");
        }

    }
    for(size_t i = 0; i < renderPointsSize; i++){
        if(renderPoints[i].x < Global.sliderMinimumX){
            renderPoints[i].x = Global.sliderMinimumX;
        }
        if(renderPoints[i].y < Global.sliderMinimumY){
            renderPoints[i].y = Global.sliderMinimumY;
        }
        if(renderPoints[i].x > Global.sliderMaximumX){
            renderPoints[i].x = Global.sliderMaximumX;
        }
        if(renderPoints[i].y > Global.sliderMaximumY){
            renderPoints[i].y = Global.sliderMaximumY;
        }
    }

    if(data.slides % 2 == 0 || renderPointsSize < 1){
        data.ex = data.x;
        data.ey = data.y;
    }
    else{
        data.ex = renderPoints[renderPointsSize - 1].x;
        data.ey = renderPoints[renderPointsSize - 1].y;
    }

	float tempTimeLength = (data.length/100) * (data.timing.beatLength) / (sliderSpeed * data.timing.sliderSpeedOverride) * data.slides;
	float endTime = data.time + tempTimeLength - (36 - (18 * (tempTimeLength <= 72.0f)));
	if(durationNull){
		endTime = data.time;
	}

	//std::vector<int> out;
	int *out = (int*)malloc(sizeof(int) * 3);
	edgePoints.clear();
	//renderPoints.clear();
	free(renderPoints);


	//out.push_back(data.ex);
	//out.push_back(data.ey);
	//out.push_back(endTime);
	out[0] = data.ex;
	out[1] = data.ey;
	out[2] = endTime;
    return out;
}




void GameManager::loadGameSounds(){
	long long int loadedBytes = 0;
	SoundFilesAll.data.clear();
	SoundFilesAll.loaded.clear();

	std::string last = Global.Path;
	std::string dontTouch = gameFile.configGeneral["AudioFilename"];
	Global.Path = GamePathWithSlash;
	std::cout << GamePathWithSlash << std::endl;
	std::vector<std::string> ComboBreak = ls(".wav");
	std::cout << ComboBreak.size() << std::endl;
	std::cout << "first comcobreak ls done, press select to continue" << std::endl;
	if(Global.settings.useDefaultSounds) ComboBreak.clear();
	for(int i = 0; i < ComboBreak.size(); i++){
		//std::cout << "loaded " << loadedBytes << " bytes" << std::endl;
		//std::cout << ComboBreak[i] << std::endl;
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		SleepInUs(1*200*1000);
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from game" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind(dontTouch, 0) != 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			if(name.rfind("drum", 0) == 0 or name.rfind("soft", 0) == 0 or name.rfind("normal", 0) == 0){
				if(name[name.size() - 1] == 'l' or name[name.size() - 1] == 'e' or name[name.size() - 1] == 'h' or name[name.size() - 1] == 'p'){
					name += '1';
				}
			}
			SoundFilesAll.data[name] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
			SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
			if(SoundFilesAll.loaded[name].value){
				std::cout << "loaded " << name << " from game" << std::endl;
			}
		}
		
	}
	
	//std::cout << "Done with all the waw stuff, press select to continue" << std::endl;
	ComboBreak = ls(".ogg");
	if(Global.settings.useDefaultSounds) ComboBreak.clear();
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from game" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind(dontTouch, 0) != 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			if(name.rfind("drum", 0) == 0 or name.rfind("soft", 0) == 0 or name.rfind("normal", 0) == 0){
				if(name[name.size() - 1] == 'l' or name[name.size() - 1] == 'e' or name[name.size() - 1] == 'h' or name[name.size() - 1] == 'p'){
					name += '1';
				}
			}
			SoundFilesAll.data[name] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
			SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
			if(SoundFilesAll.loaded[name].value){
				std::cout << "loaded " << name << " from game" << std::endl;
			}
		}
	}
	
	//std::cout << "Done with all the ogg stuff, press select to continue" << std::endl;
	ComboBreak = ls(".mp3");
	if(Global.settings.useDefaultSounds) ComboBreak.clear();
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from game" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind(dontTouch, 0) != 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			if(name.rfind("drum", 0) == 0 or name.rfind("soft", 0) == 0 or name.rfind("normal", 0) == 0){
				if(name[name.size() - 1] == 'l' or name[name.size() - 1] == 'e' or name[name.size() - 1] == 'h' or name[name.size() - 1] == 'p'){
					name += '1';
				}
			}
			SoundFilesAll.data[name] = LoadSound((GamePathWithSlash + ComboBreak[i]).c_str());
			SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
			if(SoundFilesAll.loaded[name].value){
				std::cout << "loaded " << name << " from game" << std::endl;
			}
		}
	}

	//std::cout << "Done with all the mp3 stuff, press select to continue" << std::endl;
	Global.Path = Global.GamePath + "/resources/skin/";
	ComboBreak = ls(".wav");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GamePath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GamePath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::filesystem::path p{Global.Path + ComboBreak[i]};
					loadedBytes += std::filesystem::file_size(p);
					std::cout << "loaded " << name << " from skin" << std::endl;
				}
			}
		}
	}

	//std::cout << "Done with all the skinwaw stuff, press select to continue" << std::endl;
	ComboBreak = ls(".ogg");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GamePath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GamePath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "loaded " << name << " from skin" << std::endl;
				}
			}
		}
	}

	//std::cout << "Done with all the skinogg stuff, press select to continue" << std::endl;

	ComboBreak = ls(".mp3");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GamePath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GamePath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "loaded " << name << " from skin" << std::endl;
				}
			}
		}
	}

	//std::cout << "Done with all the skinmp3 stuff, press select to continue" << std::endl;


	Global.Path = Global.GamePath + "/resources/default_skin/";
	ComboBreak = ls(".wav");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GamePath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GamePath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "loaded " << name << " from default skin" << std::endl;
				}
			}
		}
	}
	//std::cout << "Done with all the dskinwaw stuff, press select to continue" << std::endl;
	ComboBreak = ls(".ogg");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GamePath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GamePath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "loaded " << name << " from default skin" << std::endl;
				}
			}
		}
	}
	//std::cout << "Done with all the dskinogg stuff, press select to continue" << std::endl;
	ComboBreak = ls(".mp3");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GamePath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GamePath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "loaded " << name << " from default skin" << std::endl;
				}
			}
		}
	}




	//gameFile.configGeneral["AudioFilename"]


	/*SetSoundVolume(&SoundFilesAll.data["combobreak"], 1.0f);
	PlaySound(&SoundFilesAll.data["combobreak"]);*/
	std::cout << "loaded " << loadedBytes / 1024 << "KB of sound data" << std::endl;
	Global.Path = last;
	
}
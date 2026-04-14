#include "gamemanager.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <hitobject.hpp>
#include "globals.hpp"
#include "raylib.h"
#include "utils.hpp"
#include "fs.hpp"
#include <sys/time.h>
#include "followpoint.hpp"
#include "time_util.hpp"
#include "state.hpp"
#include "platformspesifics.hpp"

#include "linkedListImpl.hpp"

// For some reason the clamp function didnt work so here is a manual one
float GameManager::clip(float value, float min, float max){
  	return std::min(std::max(value,min), max);
}


GameManager* GameManager::inst_ = NULL;

// If we don't current have an instance, create one, if we do just give it back!
GameManager* GameManager::getInstance() {
   	if (inst_ == NULL)
   		inst_ = new GameManager();
   	return(inst_);
}

// Initialize on creation
GameManager::GameManager(){
	init();
}

// Initilize the game manager?
void GameManager::init(){

}

// Osu! Game loop
void GameManager::update(){
	// For now the left key on the keyboard plays the map automatically
	// this is pretty useful when you need to quickly test the timings
	// since the bot is always pressing in the correct time window
	if(IsKeyDown(Global.AUTO_KEY)){
		Global.useAuto = true;
	}
	else{
		Global.useAuto = false;
	}

	
	// Current implementation of the backgrounds
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
			if(lastTimingLoc < 0) 
				lastTimingLoc = 0;
		}
		else
			break;
	}
	if(timingSettingsForHitObject.size() == 0){
		if(gameFile.timingPoints.size() == 0){
			std::cout << "\e[1;38;5;52m[ERR] \e[38;5;52m" << "missing timingpoints? cant do much. crash imminent." << std::endl;
		}
		else{
			tempTiming.renderTicks = gameFile.timingPoints[lastTimingLoc].renderTicks;
			tempTiming.sliderSpeedOverride = 1;
			tempTiming.time = gameFile.timingPoints[lastTimingLoc].time;
			double tempBeatLength;
			tempBeatLength = gameFile.timingPoints[lastTimingLoc].beatLength;
			if(tempBeatLength >= 0){
				tempTiming.beatLength = tempBeatLength;
				verytempbeat = tempBeatLength;
				tempTiming.sliderSpeedOverride = 1;
			}
			if(tempBeatLength < 0){
				tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
				tempTiming.beatLength = verytempbeat;
			}
			tempTiming.meter = gameFile.timingPoints[lastTimingLoc].meter;
			tempTiming.sampleSet = gameFile.timingPoints[lastTimingLoc].sampleSet;
			tempTiming.sampleIndex = gameFile.timingPoints[lastTimingLoc].sampleIndex;
			tempTiming.volume = gameFile.timingPoints[lastTimingLoc].volume;
			tempTiming.uninherited = gameFile.timingPoints[lastTimingLoc].uninherited;
			tempTiming.effects = gameFile.timingPoints[lastTimingLoc].effects;
			timingSettingsForHitObject.push_back(tempTiming);
		}
	}
	

	//spawn the hitobjects when their time comes
	// o şejkilde lişğaksda başka ne uzun biliyor musun bence benim akıllığım terinde
	// -ömer 2022
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
			else{
				hitObject->data.colour.push_back(255);
				hitObject->data.colour.push_back(255);
				hitObject->data.colour.push_back(255);
			}
			hitObject->data.comboNumber = combo;
			combo++;

			int index = 0;
			for(int j = 0; j < (int)timingSettingsForHitObject.size(); j++){
				if(timingSettingsForHitObject[j].time > gameFile.hitObjects[i].time)
					break;
				index = j;
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
				//hitObject->data.time + (hitObject->data.length/100) * (hitObject->data.timing.beatLength) / (sliderSpeed * hitObject->data.timing.sliderSpeedOverride) * hitObject->data.slides;
			}
			gameFile.hitObjects.pop_back();
			spawnedHitObjects++;
			for(int j = 0; j < index - 1; j++){
				timingSettingsForHitObject.pop_front();
				//std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236mPopped front of timingSettings! Size: " << timingSettingsForHitObject.size() << std::endl;
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
	while(hitObjectNode != NULL){
		hitObject = (HitObject*)hitObjectNode->object;
		hitObjectNodeNext = hitObjectNode->next;
		processed++;

		if(IsKeyPressed(Global.AUTO_KEY)){
			Global.AutoMouseStartTime = currentTime*1000.0f;
			Global.AutoMousePositionStart = {320, 240};
		}

		if(hitObjectNode->prev == NULL){
			hitObject->data.touch = true;
			Global.AutoMousePosition = lerp(Global.AutoMousePositionStart, {static_cast<float>(hitObject->data.x), static_cast<float>(hitObject->data.y)}, clip((currentTime*1000.0f-Global.AutoMouseStartTime) / (hitObject->data.time-Global.AutoMouseStartTime), 0, 1));
		}

		if (stop && hitObjectNode->prev == NULL && (Global.Key1P or Global.Key2P)){
			if (hitObject->data.type != 2){
				if (CheckCollisionPointCircle(Global.MousePosition,Vector2{static_cast<float>(hitObject->data.x),(float)hitObject->data.y}, circlesize/2.0f)){
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
					for(int soundIndex = 0; soundIndex < (int)sounds.size(); soundIndex+=2){
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
					if(CheckCollisionPointCircle(Global.MousePosition,Vector2{static_cast<float>(hitObject->data.x),(float)hitObject->data.y}, circlesize/2.0f) && currentTime*1000.0f < tempslider->data.time + gameFile.p50Final){
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

						Global.AutoMousePositionStart = {static_cast<float>(hitObject->data.x), static_cast<float>(hitObject->data.y)};
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

	Node * deadHitObjectNode = deadObjectsLinkedList.getHead();
	Node * deadHitObjectNodeNext;
	HitObject* deadHitObject;
	while(deadHitObjectNode != NULL){
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
	if(Global.GameTextures != TEXTUREOPS_LOADED){
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

	//std::cout << score << " " << difficultyMultiplier << std::endl;
	int change = (int)std::max(std::max(0.f, (float)Global.FrameTime), std::max(0.f, (float)((float)(score - animatedScore) / 30) * (float)Global.FrameTime));
	int change2 = (int)std::max(std::max(0.f, 0.0f), std::max(0.f, (float)((float)(score - animatedScore) / 30) * (float)Global.FrameTime));
	animatedScore += change;
	if(animatedScore > score){
		animatedScore = score;
		change2 = 0;
	}
	change2 = clip(change2, 0.01, 25);
	float scale = clip((float)sqrt((float)change2), 0.0f, 5.0f) / 100;
	DrawCNumbersRight(animatedScore, 640 - 15 + (ScaleCordX(0) / Scale(1)), (15 + scale * (numbers[0].height / 4.0)) - (ScaleCordY(0) / Scale(1)), 0.5f + scale, Fade(WHITE, 0.8-scale));

	DrawCNumbersLeft(clickCombo, 15 - ((GetScreenWidth() - Scale(640))) / Scale(2), 460 + ((GetScreenHeight() - Scale(480))) / Scale(2), 0.6f, WHITE);

	if(spawnedHitObjects == 0 && gameFile.hitObjects[gameFile.hitObjects.size() - 1].time > 6000 + currentTime*1000.0f){
		////Global.mutex.lock();
		DrawTextEx(&Global.DefaultFont, TextFormat("TO SKIP PRESS ENTER or A\n(Keep in mind that this can affect the offset\nbecause of how the sounds system currently works)"), {static_cast<float>((int)Scale(5)), static_cast<float>(GetScreenHeight() - (int)Scale(80))}, Scale(20.05), Scale(2), WHITE);
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
	if(Global.CurrentState->initializationStage == STATE_UNLOAD || Global.CurrentState->initializationStage == STATE_FORCED_EXIT){
		maxCombo = std::max(maxCombo, clickCombo);
		StopMusicStream(&backgroundMusic);
		TimerLast = (double)GetMusicTimeLength(&backgroundMusic) * 1000.0;
		TimeLast = getTimer();
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "waiting for 0.25 secs for the song to end\n";
		while((getTimer() - TimeLast) < 250.0){
			if(Global.CurrentState->initializationStage == STATE_FORCED_EXIT)
				break;
			currentTime = (double)(TimerLast + (getTimer() - TimeLast)) / 1000.0;
			GameManager::update();
			SleepInMs(5);
		}
		//std::cout << "waiting done\n";
		/*MutexUnlock(ACCESSING_OBJECTS);
		MutexLock(SWITCHING_STATE);
		MutexLock(ACCESSING_OBJECTS);
		MutexUnlock(SWITCHING_STATE);
		MutexLock(SWITCHING_STATE);*/
		Global.CurrentState->initializationStage = STATE_UNLOAD;
		//MutexUnlock(ACCESSING_OBJECTS);
		MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "trying to lock render block for game unload" << std::endl;
		MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);

		//MutexLock(ACCESSING_OBJECTS);
		
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "trying to lock switch block for game unload" << std::endl;
		MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
		
		Global.CurrentState->unload();
		//MutexUnlock(ACCESSING_OBJECTS);
		

		Global.CurrentState.reset(new ResultsMenu());
		Global.CurrentState->init();
		MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
		MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
		MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);

		return;
	}

	if(Global.startTime < 0){
		if(Global.FrameTime < 50.0f)
			Global.startTime += Global.FrameTime;
		Time = Global.startTime;
		//std::cout << Time << std::endl;
	}
	if(Global.startTime >= 0 and startMusic){
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Starting Music Playback" << std::endl;
		PlayMusicStream(&backgroundMusic);
		//Global.volume = 1.0f;
		//std::cout << Global.volume << std::endl;
    	SetMusicVolume(&backgroundMusic, Global.volume); //Global.volume
		//SeekMusicStream(&backgroundMusic, 0.0f);
		UpdateMusicStream(&backgroundMusic);
		initTimer();
		//std::cout << "started music"  << std::endl;
		//std::cout << "first update" << std::endl;
		//std::cout << sizeof(HitObjectData) << std::endl;
		//std::cout << sizeof(HitObjectData) * gameFile.hitObjects.size() << std::endl;
		//std::cout << "sizes given in bytes\n";
		Global.CurrentInterpolatedTime = 0;
		Global.LastOsuTime = 0;
		TimeLast = ms;
		startMusic = false;
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;51m" << "Extra Judgement Time in ms " << Global.extraJudgementTime << std::endl;

		Global.LastFrameTime = getTimer();
	}
	if(Global.startTime >= 0){
		if(Global.volumeChanged){
			SetMusicVolume(&backgroundMusic, Global.volume);
			Global.volumeChanged = false;
		}
		UpdateMusicStream(&backgroundMusic);
		if(spawnedHitObjects == 0 && gameFile.hitObjects[gameFile.hitObjects.size() - 1].time > 6000 + currentTime*1000.0f){
			//DrawTextEx(Global.DefaultFont, TextFormat("TO SKIP PRESS \"S\"\n(Keep in mind that this can affect the offset\nbecause of how the raylib sounds system works)"), {ScaleCordX(5), ScaleCordY(420)}, Scale(15), Scale(1), WHITE);
			if(IsKeyPressed(Global.SKIP_KEY)){
				SeekMusicStream(&backgroundMusic, std::max(0.0f, (gameFile.hitObjects[gameFile.hitObjects.size() - 1].time - 3000.0f) / 1000.0f));
			}
		}
		//if(GetMusicTimeLength(&backgroundMusic) - GetMusicTimePlayed(&backgroundMusic) < 0.1f)
		//	stop = true;
		if(_music_check_if_ended(&backgroundMusic)){
			maxCombo = std::max(maxCombo, clickCombo);
			StopMusicStream(&backgroundMusic);
			TimerLast = (double)GetMusicTimeLength(&backgroundMusic) * 1000.0;
			TimeLast = getTimer();
			std::cout << "\e[1;38;5;236m[INFO] \e[38;5;40m" << "waiting for 1.5 secs for the beatmap to end\n";
			while((getTimer() - TimeLast) < 1500.0){
				currentTime = (double)(TimerLast + (getTimer() - TimeLast)) / 1000.0;
				GameManager::update();
				SleepInMs(5);
			}
			//std::cout << "waiting done\n";
			/*MutexUnlock(ACCESSING_OBJECTS);
			MutexLock(SWITCHING_STATE);
			MutexLock(ACCESSING_OBJECTS);
			MutexUnlock(SWITCHING_STATE);
			MutexLock(SWITCHING_STATE);*/
			Global.CurrentState->initializationStage = STATE_UNLOAD;
            //MutexUnlock(ACCESSING_OBJECTS);
			MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
			std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "trying to lock render block for game unload" << std::endl;
			MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);

            //MutexLock(ACCESSING_OBJECTS);
            
			std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "trying to lock switch block for game unload" << std::endl;
			MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
			
			Global.CurrentState->unload();
			//MutexUnlock(ACCESSING_OBJECTS);
			

            Global.CurrentState.reset(new ResultsMenu());
            Global.CurrentState->init();
			MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
			MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
			MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);

			return;
		}
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
			Global.TimeStepSize = ms - TimeLast;
		}
	}
	else{
		TimeLast = ms;
	}

	double LastInterpolatedTime = Global.currentOsuTime;

	bool IsInterpolating = true;

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
			currentTime += (Global.offsetTime / 1000.0f);
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
	Global.Path = Global.GameBinaryPath + "/resources/default_skin/";
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (std::string_view first, std::string_view second){
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

void GameManager::loadGameSkin(std::string filename){
	temprenderSpinnerCircle = false;
	temprenderSpinnerMetre = false;
	temprenderSpinnerBack = false;

	std::vector<std::string> files;
	files.clear();
	Global.Path = Global.GameBinaryPath + "/resources/skin/";
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (std::string_view first, std::string_view second){
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

void GameManager::loadBeatmapSkin(std::string filename){
	std::vector<std::string> files;
	files.clear();
	Global.Path = lastPath + '/';
	
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (std::string_view first, std::string_view second){
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


	//clear linked lists, WARNING, WILL MEMORY LEAK!
	deadObjectsLinkedList.init();
	objectsLinkedList.init();

	//misc variables
	spawnedHitObjects = 0;
	Parser parser = Parser();
	Global.loadingState = LOADINGSTATE_PARSING_LINES;
	gameFile.configGeneral["SampleSet"] = "Normal";
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Parsing game!" << std::endl;
	gameFile = parser.parse(filename);
    std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Found " << gameFile.hitObjects.size() << " HitObjects and " << gameFile.timingPoints.size() << " Timing Points!" << std::endl;
	Global.loadingState = LOADINGSTATE_PRECALC_HITOBJECT;
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

	BeatmapFolderPathWithSlash = temporaryPath + '/';

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
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;20m" << "Loading sliders..." << std::endl;
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
				gameFile.hitObjects[i].curveType = 'L';
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
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;32m" << amountOfSliders << " Sliders precalculated in " << getTimer() - start << "ms" << std::endl;
	
	

	
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


	Global.loadingState = LOADINGSTATE_LOADING_SOUNDS;

	//reset the score and the combo
	score = 0;
	animatedScore = 0;
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
	

	if (gameFile.configDifficulty.find("SliderMultiplier") != gameFile.configDifficulty.end())
		sliderSpeed = std::stof(gameFile.configDifficulty["SliderMultiplier"]);

	
	



	
	//Global.GameTextures = TEXTUREOPS_UNLOADING_IN_PROGRESS;
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
    double preCalcLength = 0;
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
        else{
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
	int followComboColors = 0;
	float followPointFadeTime = gameFile.preempt - gameFile.fade_in;
	followLines.clear();
	
	for(int i = 1; i < gameFile.hitObjects.size(); i++){
		//float templength = (data.length/100) * (data.timing.beatLength) / (gm->sliderSpeed * data.timing.sliderSpeedOverride) * data.slides; //slider length
		if(gameFile.hitObjects[i - 1].startingACombo){
			followComboColors++;
			if(gameFile.comboColours.size())
				followComboColors = (followComboColors + gameFile.hitObjects[i - 1].skipComboColours) % gameFile.comboColours.size();
		}
		if(gameFile.hitObjects[i].startingACombo == false and gameFile.hitObjects[i - 1].type != 3){
			FollowPoint tempPoint;
			tempPoint.endTime = gameFile.hitObjects[i].time;
			tempPoint.endTime2 = gameFile.hitObjects[i].time + followPointFadeTime;
			tempPoint.endX = gameFile.hitObjects[i].x;
			tempPoint.endY = gameFile.hitObjects[i].y;
			
			
			if(gameFile.comboColours.size())
				tempPoint.color = Color{gameFile.comboColours[followComboColors][0], gameFile.comboColours[followComboColors][1], gameFile.comboColours[followComboColors][2], 255};
			else{
				tempPoint.color = Color{255,200,255,255};
			}
			
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
			tempPoint.angle = atan2(tempPoint.endY - tempPoint.startY, tempPoint.endX - tempPoint.startX);
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

	Global.loadingState = LOADINGSTATE_LISTING_HITSOUNDS;
	
	loadGameSounds();
	



	Global.loadingState = LOADINGSTATE_LOADING_BACKGROUND_MUSIC;
	Global.Path.pop_back();
	backgroundMusic = LoadMusicStream((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str());


	difficultyMultiplier = (((hpdrainrate + std::stof(gameFile.configDifficulty["CircleSize"]) + overalldifficulty) + clip((float)gameFile.hitObjects.size() / GetMusicTimeLength(&backgroundMusic) * 8.f, 0.f, 16.f)) / 38.f * 5.f);
	
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "hp cs od ??: " <<  hpdrainrate << " " << std::stof(gameFile.configDifficulty["CircleSize"]) << " " << overalldifficulty << " " << (float)gameFile.hitObjects.size() / GetMusicTimeLength(&backgroundMusic) * 8.f << std::endl;



#ifdef THREEDS_BUILD
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free Vram: " << _os_get_free_vram() / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_ALL: " << _os_get_free_ram(MEMREGION_ALL) / 1024 << "/" << _os_get_size_ram(MEMREGION_ALL) / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_APP: " << _os_get_free_ram(MEMREGION_APPLICATION) / 1024 << "/" << _os_get_size_ram(MEMREGION_APPLICATION) / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_SYS: " << _os_get_free_ram(MEMREGION_SYSTEM) / 1024 << "/" << _os_get_size_ram(MEMREGION_SYSTEM) / 1024 << "KB" << std::endl;
	std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_BSE: " << _os_get_free_ram(MEMREGION_BASE) / 1024<< "/" << _os_get_size_ram(MEMREGION_BASE) / 1024 << "KB" << std::endl;
    std::cout << "\e[1;36m[3DS] \033[38;5;110m" << "Free M_LIN: " << _os_get_free_linear_ram() / 1024 << "/" << Global.linearSpaceFree / 1024 << "KB" << std::endl;
#endif



	//SleepInMs(5000);

	

	Global.loadingState = LOADINGSTATE_LOADING_TEXTURES;

	Global.Path = lastPath;
	Global.GameTextures = TEXTUREOPS_START_LOADING;
	
	startMusic = true;
	stop = false;
}

void GameManager::unloadGame(){
	//std::cout << "UnloadingGame" << std::endl;
	currentComboIndex = 0;
	
	SleepInMs(20);

	for(auto& pair : SoundFilesAll.data) {
    	UnloadSound(&pair.second);
  	}
	/*for(int i = objects.size()-1; i >= 0; i--){
		destroyHitObject(i);
	}*/
	
	UnloadMusicStream(&backgroundMusic);
	Global.numberLines = -1;
    Global.parsedLines = -1;
	//Global.mutex.unlock();
	//LightLock_Unlock(&Global.lightlock);
	Global.GameTextures = TEXTUREOPS_START_UNLOADING;
	MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
	MutexUnlock(SWITCHING_STATE, UPDATETHREAD_ID);
	MutexUnlock(RENDER_BLOCK, UPDATETHREAD_ID);
	
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "unlocking the hold access lock\n";
	//MutexUnlock(SWITCHING_STATE);
	while(true){
		SleepInMs(500);
		
		std::cout << "\e[1;38;5;236m[INFO] \e[38;5;32m" << "waiting for textures to unload\n";
		if(Global.GameTextures == TEXTUREOPS_UNLOADED)
			break;
		
	}
	//MutexLock(SWITCHING_STATE);
	//MutexLock(RENDER_BLOCK);

	

	MutexLock(RENDER_BLOCK, UPDATETHREAD_ID);
	MutexLock(SWITCHING_STATE, UPDATETHREAD_ID);
	MutexLock(ACCESSING_OBJECTS, UPDATETHREAD_ID);


	
	//Global.mutex.lock();
	//LightLock_Lock(&Global.lightlock);


	for(int i = gameFile.hitObjects.size() - 1; i >= 0; i--){
		gameFile.hitObjects[i].curvePoints.clear();
		gameFile.hitObjects[i].edgeSounds.clear();
		gameFile.hitObjects[i].edgeSets.clear();
		gameFile.hitObjects[i].colour.clear();
		gameFile.hitObjects[i].lengths.clear();
		gameFile.hitObjects[i].filename = "";

		gameFile.hitObjects[i].curvePoints.shrink_to_fit();
		gameFile.hitObjects[i].edgeSounds.shrink_to_fit();
		gameFile.hitObjects[i].edgeSets.shrink_to_fit();
		gameFile.hitObjects[i].colour.shrink_to_fit();
		gameFile.hitObjects[i].lengths.shrink_to_fit();
	}

	for(int i = gameFile.followPoints.size() - 1; i >= 0; i--){
		gameFile.followPoints[i].points.clear();

		gameFile.followPoints[i].points.shrink_to_fit();
	}

	gameFile.hitObjects.clear();
	gameFile.timingPoints.clear();
	gameFile.followPoints.clear();
	gameFile.events.clear();
	
	gameFile.hitObjects.shrink_to_fit();
	gameFile.timingPoints.shrink_to_fit();
	gameFile.followPoints.shrink_to_fit();
	gameFile.events.shrink_to_fit();


	followLines.clear();
	followLines.shrink_to_fit();

	
	//Global.mutex2.lock();
	while(true){
		if(objectsLinkedList.getHead() == NULL)
			break;
		((HitObject*)objectsLinkedList.getHead()->object)->deinit();
		delete (HitObject*)objectsLinkedList.getHead()->object;
		objectsLinkedList.deleteHead();
	}
	while(true){
		if(deadObjectsLinkedList.getHead() == NULL)
			break;
		((HitObject*)deadObjectsLinkedList.getHead()->object)->deinit();
		delete (HitObject*)deadObjectsLinkedList.getHead()->object;
		deadObjectsLinkedList.deleteHead();
	}
	MutexUnlock(ACCESSING_OBJECTS, UPDATETHREAD_ID);
	//MutexUnlock(RENDER_BLOCK);
	//Global.mutex2.unlock();
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;40m" << "Game fully (?) unloaded\n";
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
	delete (HitObject*)node->object;
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
	
	Image tempImage = LoadImage((Global.GameBinaryPath + "/resources/sliderin.png").c_str());
	ImageColorReplace(&tempImage, {255,255,255,159}, {255,255,255,0});
	sliderin = LoadTextureFromImage(&tempImage);
	UnloadImage(&tempImage);
	Image tempImage2 = GenImageGradientRadial(sliderin.width, sliderin.height, 0.1, {255,0,0,255}, {0,0,0,0});
	sliderblank = LoadTexture((Global.GameBinaryPath + "/resources/SliderBlank.png").c_str());
	//sliderout = LoadTexture((Global.GameBinaryPath + "/resources/SliderBlank.png").c_str());
	sliderout = LoadTextureFromImage(&tempImage2);
	UnloadImage(&tempImage2);
    //sliderout = LoadTexture((Global.GameBinaryPath + "/resources/sliderout.png").c_str());

	_gpu_start_drawing(Global.window);



	sliderInnerBall = LoadRenderTexture(64, 64);
	sliderOuterBall = LoadRenderTexture(64, 64);	
	
	BeginTextureMode(&sliderOuterBall);
    ClearBackground({0,0,0,0});
	EndBlendMode();
	//rlEnableDepthTest2();
	//rlDisableDepthTest();
	//rlEnableDepthTest();
	rlSetBlendFactorsSeparate(RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_SRC_ALPHA, RL_ONE_MINUS_SRC_ALPHA, RL_MAX, RL_MAX);
	DrawCircleWithDepth((Vector2){32, 32}, 30.5, 64, 0.5f, {255, 255, 255, 255});
	DrawCircleWithDepth((Vector2){32, 32}, 31.0, 64, 0.4f, {255, 255, 255, 200});
	DrawCircleWithDepth((Vector2){32, 32}, 31.2, 64, 0.3f, {255, 255, 255, 150});
	DrawCircleWithDepth((Vector2){32, 32}, 31.5, 64, 0.2f, {255, 255, 255, 50});
	//rlDisableDepthTest();
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

	followPoint = LoadTexture((Global.GameBinaryPath + "/resources/followpoint.png").c_str());
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
			std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Time: " << gameFile.events[i].startTime << "ms - Filename: " << gameFile.events[i].filename << '.' << std::endl;
			if(gameFile.events[i].startTime < 10000){
				gameFile.events[i].startTime -= 87000;
				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Time changed to: " << gameFile.events[i].startTime << std::endl;
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
				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "attempting to load a background\n";
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
					
					int divider = OSUS_DEFAULT_DIVIDER;
					while(true){
						if(image.width / divider > OSUS_MAX_TEXTURE_WIDTH or image.height / divider > OSUS_MAX_TEXTURE_HEIGHT){
							divider += 1;
							std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "TOO BIG OF AN IMAGE!" << std::endl;
							
						}
						else{
							break;
						}
					}
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "RESIZING TO" << (int)(image.height / divider) << " x " << (int)(image.width / divider) << std::endl;
					//std::cout << "resize" << std::endl;
					//SleepInMs(200);
					int resizeW = (int)(image.width / divider);
					int resizeH = (int)(image.height / divider);
					ImageResize(&image, resizeW, resizeH);
					//std::cout << "T " << image.width << " " << image.height << std::endl;

					ImageBlurGaussian(&image, 2.0f / divider);
					ImageColorTint(&image, Color{30,30,30,255});
					
					#ifdef THREEDS_BUILD
						ImageDither(&image, 5, 6, 5, 0);
					#endif
					backgroundTextures.data[gameFile.events[j].filename] = LoadTextureFromImage(&image);
					UnloadImage(&image); 
					
					backgroundTextures.pos[gameFile.events[j].filename] = {static_cast<float>(gameFile.events[j].xOffset), static_cast<float>(gameFile.events[j].yOffset)};
					if(backgroundTextures.data[gameFile.events[j].filename].width != 0){
						backgroundTextures.loaded[gameFile.events[j].filename].value = true;
						std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Loaded background: " << get_filename(gameFile.events[j].filename) << std::endl;
						SetTextureFilter(&backgroundTextures.data[gameFile.events[j].filename], TEXTURE_FILTER_BILINEAR) ;
					}
					
					//std::cout << "*. no backgroundss *.\n";
				}
			}
		}
	}


	

	//AAAAAAAAAAA

    Global.GameTextures = TEXTUREOPS_LOADED;
}

void GameManager::unloadGameTextures(){

	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Unloading Textures" << std::endl;
	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Started marking sliders for unloading" << std::endl;
	Node * HitObjectNode = objectsLinkedList.getHead();
	Node * HitObjectNodeNext;
	HitObject* hitObject;
	while(true){
		if(HitObjectNode == NULL or objectsLinkedList.getSize() == 0){
			break;
		}
		hitObject = (HitObject*)HitObjectNode->object;
		HitObjectNodeNext = HitObjectNode->next;

		if(hitObject->data.type == 2){
			if(Slider* tempslider = dynamic_cast<Slider*>(hitObject)){
				tempslider->readyToDelete = true;
				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Marking slider at time " << tempslider->data.time << std::endl;
			}
		}
		
		HitObjectNode = HitObjectNodeNext;
	}

	Node * deadHitObjectNode = deadObjectsLinkedList.getHead();
	Node * deadHitObjectNodeNext;
	HitObject* deadHitObject;
	while(true){
		if(deadHitObjectNode == NULL or deadObjectsLinkedList.getSize() == 0){
			break;
		}
		deadHitObject = (HitObject*)deadHitObjectNode->object;
		deadHitObjectNodeNext = deadHitObjectNode->next;

		if(deadHitObject->data.type == 2){
			if(Slider* tempslider = dynamic_cast<Slider*>(deadHitObject)){
				tempslider->readyToDelete = true;

				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Marking dead slider at time " << tempslider->data.time << " linked list size: " << deadObjectsLinkedList.getSize() << std::endl;
			}
		}
		
		deadHitObjectNode = deadHitObjectNodeNext;
	}

    //Global.GameTextures = TEXTUREOPS_UNLOADING_IN_PROGRESS;
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
        std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Unloaded background: " << it->first << "\n";
        UnloadTexture(&backgroundTextures.data[key]);
    }

    


	//Global.GameTextures = TEXTUREOPS_UNLOADING_DONE_BASIC;
	
	backgroundTextures.data.clear();
    backgroundTextures.pos.clear();
    backgroundTextures.loaded.clear();
	//Global.mutex2.lock();
	

	

	//Global.mutex.unlock();
    //LightLock_Unlock(&Global.lightlock);
	unloadSliderTextures();
	//Global.mutex.lock();
    //LightLock_Lock(&Global.lightlock);
	//Global.mutex2.unlock();
	Global.GameTextures = TEXTUREOPS_UNLOADED;
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
			std::vector<Vector2> temporaryPoints = interpolate2(edgePoints, data.length);
            renderPoints = (Vector2*)malloc(sizeof(Vector2) * (data.length));
			renderPointsSize = data.length;
			int ending = 0;
			for(int i = 0; i < renderPointsSize && i < temporaryPoints.size(); i++){
				renderPoints[i] = temporaryPoints[i];
				ending++;
			}
			while(ending < renderPointsSize){
				renderPoints[ending] = renderPoints[ending - 1];
				ending++;
			}
			temporaryPoints.clear();
        }
        else{
			renderPoints = (Vector2*)malloc(sizeof(Vector2) * (1));
			renderPointsSize = 0;
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

// Load all the sounds the game actually needs. Call this after actually parsing the game
void GameManager::loadGameSounds(){
	long long int loadedBytes = 0;
	SoundFilesAll.data.clear();
	SoundFilesAll.loaded.clear();
	//std::string last = Global.Path;
	std::string dontTouch = gameFile.configGeneral["AudioFilename"];
	//Global.Path = BeatmapFolderPathWithSlash;
	//std::cout << BeatmapFolderPathWithSlash << std::endl;

	//std::vector<std::string> ComboBreak = ls(".wav");
	std::vector<std::string> ComboBreak = ls(".wav", BeatmapFolderPathWithSlash);
	//std::cout << ComboBreak.size() << std::endl;
	if(Global.settings.useDefaultSounds) ComboBreak.clear();
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		SleepInUs(1*200*1000);
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((BeatmapFolderPathWithSlash + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Loaded " << ComboBreak[i] << " from beatmap files" << std::endl;
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
			SoundFilesAll.data[name] = LoadSound((BeatmapFolderPathWithSlash + ComboBreak[i]).c_str());
			SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
			if(SoundFilesAll.loaded[name].value){
				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "Loaded " << name << " from beatmap files" << std::endl;
			}
		}
		
	}
	
	//std::cout << "Done with all the waw stuff, press select to continue" << std::endl;
	ComboBreak = ls(".ogg", BeatmapFolderPathWithSlash);
	if(Global.settings.useDefaultSounds) ComboBreak.clear();
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((BeatmapFolderPathWithSlash + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from game" << std::endl;
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
			SoundFilesAll.data[name] = LoadSound((BeatmapFolderPathWithSlash + ComboBreak[i]).c_str());
			SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
			if(SoundFilesAll.loaded[name].value){
				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from game" << std::endl;
			}
		}
	}
	
	//std::cout << "Done with all the ogg stuff, press select to continue" << std::endl;
	ComboBreak = ls(".mp3", BeatmapFolderPathWithSlash);
	if(Global.settings.useDefaultSounds) ComboBreak.clear();
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((BeatmapFolderPathWithSlash + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from game" << std::endl;
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
			SoundFilesAll.data[name] = LoadSound((BeatmapFolderPathWithSlash + ComboBreak[i]).c_str());
			SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
			if(SoundFilesAll.loaded[name].value){
				std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from game" << std::endl;
			}
		}
	}

	//std::cout << "Done with all the mp3 stuff, press select to continue" << std::endl;
	//Global.Path = Global.GameBinaryPath + "/resources/skin/";
	ComboBreak = ls(".wav", Global.GameBinaryPath + "/resources/skin/");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GameBinaryPath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GameBinaryPath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					//std::filesystem::path p{Global.Path + ComboBreak[i]};
					//loadedBytes += std::filesystem::file_size(p);
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from skin" << std::endl;
				}
			}
		}
	}

	//std::cout << "Done with all the skinwaw stuff, press select to continue" << std::endl;
	ComboBreak = ls(".ogg", Global.GameBinaryPath + "/resources/skin/");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GameBinaryPath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GameBinaryPath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from skin" << std::endl;
				}
			}
		}
	}

	//std::cout << "Done with all the skinogg stuff, press select to continue" << std::endl;

	ComboBreak = ls(".mp3", Global.GameBinaryPath + "/resources/skin/");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GameBinaryPath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GameBinaryPath + "/resources/skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from skin" << std::endl;
				}
			}
		}
	}

	//std::cout << "Done with all the skinmp3 stuff, press select to continue" << std::endl;


	//Global.Path = Global.GameBinaryPath + "/resources/default_skin/";
	ComboBreak = ls(".wav", Global.GameBinaryPath + "/resources/default_skin/");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GameBinaryPath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GameBinaryPath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from default skin" << std::endl;
				}
			}
		}
	}
	//std::cout << "Done with all the dskinwaw stuff, press select to continue" << std::endl;
	ComboBreak = ls(".ogg", Global.GameBinaryPath + "/resources/default_skin/");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GameBinaryPath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GameBinaryPath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from default skin" << std::endl;
				}
			}
		}
	}
	//std::cout << "Done with all the dskinogg stuff, press select to continue" << std::endl;
	ComboBreak = ls(".mp3", Global.GameBinaryPath + "/resources/default_skin/");
	for(int i = 0; i < ComboBreak.size(); i++){
		if(ComboBreak[i][ComboBreak[i].size() - 1] == '/') continue;
		if(ComboBreak[i].rfind("combobreak", 0) == 0){
			if(SoundFilesAll.loaded.count("combobreak") == 0 or SoundFilesAll.loaded["combobreak"].value == false){
				SoundFilesAll.data["combobreak"] = LoadSound((Global.GameBinaryPath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded["combobreak"].value = IsSoundReady(&SoundFilesAll.data["combobreak"]);
				if(SoundFilesAll.loaded["combobreak"].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << ComboBreak[i] << " from default skin" << std::endl;
				}
			}
		}
		else if(ComboBreak[i].rfind("drum", 0) == 0 or ComboBreak[i].rfind("soft", 0) == 0 or ComboBreak[i].rfind("normal", 0) == 0){
			std::string name = ComboBreak[i].substr(0, ComboBreak[i].length() - 4);
			while(std::isdigit(name[name.size() - 1])) name.pop_back();
			if(SoundFilesAll.loaded.count(name) == 0 or SoundFilesAll.loaded[name].value == false){
				SoundFilesAll.data[name] = LoadSound((Global.GameBinaryPath + "/resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFilesAll.loaded[name].value = IsSoundReady(&SoundFilesAll.data[name]);
				if(SoundFilesAll.loaded[name].value){
					std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << name << " from default skin" << std::endl;
				}
			}
		}
	}

	std::cout << "\e[1;38;5;236m[INFO] \e[38;5;236m" << "loaded " << loadedBytes / 1024 << "KB of sound data" << std::endl;
	//Global.Path = last;
	
}
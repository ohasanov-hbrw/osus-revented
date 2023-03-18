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
	if(IsKeyDown(SDL_SCANCODE_LEFT)){
		Global.useAuto = true;
	}
	else{
		Global.useAuto = false;
	}
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

	timingSettings tempTiming;
	int timingSize = gameFile.timingPoints.size(); 
	for(int i = timingSize-1; i >= 0; i--){
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
			gameFile.timingPoints.pop_back();
		}
		else
			break;
	}

	if(timingSettingsForHitObject.size() == 0){
		int i = gameFile.timingPoints.size() - 1;
		if(gameFile.timingPoints.size() == 0){
			std::cout << "what the fuck" << std::endl;
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
			gameFile.timingPoints.pop_back();
		}
	}
	
	//spawn the hitobjects when their time comes
	int size = gameFile.hitObjects.size();	
	for(int i = size-1; i >= 0; i--){
		if(gameFile.hitObjects[i].time - gameFile.preempt <= currentTime*1000.0f){
			/*if(gameFile.hitObjects[i].type == 2 and gameFile.hitObjects[i].totalLength > Global.maxSliderSize){
				std::cout << "well fuck this long slider i guess. \n";
				gameFile.hitObjects.pop_back();
			}*/
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
				/*std::cout << "Time:" << timingSettingsForHitObject[index].time << " Beat:" << objects[objects.size()-1]->data.timing.beatLength <<
				" Meter:" << objects[objects.size()-1]->data.timing.meter << " SV:" << objects[objects.size()-1]->data.timing.sliderSpeedOverride <<
				" SS:" << sliderSpeed << " RT:" << objects[objects.size()-1]->data.timing.renderTicks;*/
				objects[objects.size()-1]->init();

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
	//update and check collision for every hit circle
	int newSize = objects.size();
	int oldSize = objects.size();
	int susSize = objects.size();
	bool stop = true;
	if(susSize == 0){
		Global.AutoMouseStartTime = currentTime*1000.0f;
	}
	for(int i = 0; i < objects.size(); i++){
		//if((std::abs(currentTime*1000 - objects[i]->data.time) <= gameFile.p50Final)){
		
		if(IsKeyPressed(SDL_SCANCODE_LEFT)){
			Global.AutoMouseStartTime = currentTime*1000.0f;
			Global.AutoMousePositionStart = {320, 240};
		}

		if(i == 0){
			objects[i]->data.touch = true;
			Global.AutoMousePosition = lerp(Global.AutoMousePositionStart, {objects[i]->data.x, objects[i]->data.y}, clip((currentTime*1000.0f-Global.AutoMouseStartTime) / (objects[i]->data.time-Global.AutoMouseStartTime), 0, 1));
		}
		if (stop && i == 0 && (Global.Key1P or Global.Key2P)){
			if (objects[i]->data.type != 2){
				Global.amogus2 = 12.0f;
				if (CheckCollisionPointCircle(Global.MousePosition,Vector2{objects[i]->data.x,(float)objects[i]->data.y}, circlesize/2.0f)){
					if(std::abs(currentTime*1000.0f - objects[i]->data.time) > gameFile.p50Final + Global.amogus2/2.0f){
						objects[i]->data.point = 0;
						if(clickCombo > 30){
							SetSoundVolume(SoundFiles.data["combobreak"], 50/100.0f);
							PlaySound(SoundFiles.data["combobreak"]);
						}
						clickCombo = 0;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					else if(std::abs(currentTime*1000.0f - objects[i]->data.time) > gameFile.p100Final + Global.amogus2/2.0f){
						objects[i]->data.point = 1;
						score+= 50 + (50 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
						Global.errorDiv++;
						Global.errorLast = (long long)((currentTime*1000.0f - objects[i]->data.time) * 1000.0f);
						Global.errorSum += Global.errorLast;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					else if(std::abs(currentTime*1000.0f - objects[i]->data.time) > gameFile.p300Final + Global.amogus2/2.0f){
						objects[i]->data.point = 2;
						score+= 100 + (100 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
						Global.errorDiv++;
						Global.errorLast = (long long)((currentTime*1000.0f - objects[i]->data.time) * 1000.0f);
						Global.errorSum += Global.errorLast;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					else{
						objects[i]->data.point = 3;
						score+= 300 + (300 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
						Global.errorDiv++;
						Global.errorLast = (long long)((currentTime*1000.0f - objects[i]->data.time) * 1000.0f);
						Global.errorSum += Global.errorLast;
						Global.Key1P = false;
						Global.Key2P = false;
					}
					int volume = objects[i]->data.volume;
					if(volume == 0){
						objects[i]->data.volume = objects[i]->data.timing.volume;
						volume = objects[i]->data.volume;
					}
					SetSoundPan(SoundFiles.data[objects[i]->data.NormalSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
					SetSoundVolume(SoundFiles.data[objects[i]->data.NormalSound], (float)volume/100.0f);
					PlaySound(SoundFiles.data[objects[i]->data.NormalSound]);
					if(objects[i]->data.PlayAddition){
						SetSoundPan(SoundFiles.data[objects[i]->data.AdditionSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
						SetSoundVolume(SoundFiles.data[objects[i]->data.AdditionSound], (float)volume/100.0f);
						PlaySound(SoundFiles.data[objects[i]->data.AdditionSound]);
					}
					if(objects[i]->data.PlayCustom){
						SetSoundPan(SoundFiles.data[objects[i]->data.CustomSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
						SetSoundVolume(SoundFiles.data[objects[i]->data.CustomSound], (float)volume/100.0f);
						PlaySound(SoundFiles.data[objects[i]->data.CustomSound]);
					}
					objects[i]->data.time = currentTime*1000.0f;
					destroyHitObject(i);
					newSize = objects.size();
					stop = false;
					if(newSize != oldSize){
						i--;
						oldSize = newSize;
					}
				}
				else{
					objects[i]->data.touch = true;
					objects[i]->update();
					newSize = objects.size();
					if(newSize != oldSize){
						i--;
						oldSize = newSize;
					}
				}
			}
			else if (objects[i]->data.type == 2){
				if(Slider* tempslider = dynamic_cast<Slider*>(objects[i])){
					if(CheckCollisionPointCircle(Global.MousePosition,Vector2{objects[i]->data.x,(float)objects[i]->data.y}, circlesize/2.0f) && currentTime*1000.0f < tempslider->data.time + gameFile.p50Final){
						if(std::abs(currentTime*1000.0f - tempslider->data.time) > gameFile.p50Final + Global.amogus2/2.0f){
							tempslider->is_hit_at_first = true;
							stop = false;
							tempslider->earlyhit = true;
							if(clickCombo > 30){
								SetSoundVolume(SoundFiles.data["combobreak"], 50/100.0f);
								PlaySound(SoundFiles.data["combobreak"]);
							}
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
						SetSoundPan(SoundFiles.data[objects[i]->data.EdgeNormalSound[0]], 1-clip(objects[i]->data.x / 640.0, 0, 1));
						SetSoundPan(SoundFiles.data[objects[i]->data.EdgeAdditionSound[0]], 1-clip(objects[i]->data.x / 640.0, 0, 1));
						SetSoundVolume(SoundFiles.data[objects[i]->data.EdgeNormalSound[0]], (float)volume/100.0f);
						SetSoundVolume(SoundFiles.data[objects[i]->data.EdgeAdditionSound[0]], (float)volume/100.0f);
						PlaySound(SoundFiles.data[objects[i]->data.EdgeNormalSound[0]]);
						PlaySound(SoundFiles.data[objects[i]->data.EdgeAdditionSound[0]]);
						if(tempslider->data.PlayCustom){
							SetSoundPan(SoundFiles.data[tempslider->data.CustomSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
							SetSoundVolume(SoundFiles.data[tempslider->data.CustomSound], (float)volume/100.0f);
							PlaySound(SoundFiles.data[tempslider->data.CustomSound]);
						}
					}
				}
				//this cursed else train is nothing to worry about...
				objects[i]->data.index = i;
				objects[i]->update();
				newSize = objects.size();
				if(newSize != oldSize){
					i--;
					oldSize = newSize;
				}
			}
			else{
				objects[i]->data.index = i;
				objects[i]->update();
				newSize = objects.size();
				if(newSize != oldSize){
					i--;
					oldSize = newSize;
				}
			}
			Global.Key1P = false;
			Global.Key2P = false;
		}
		else{
			bool debugf = IsKeyDown(SDL_SCANCODE_LEFT);
			if(debugf){
				//std::cout << "updating object on time: " << objects[i]->data.time << std::endl;
				
				newSize = objects.size();
				if(newSize != oldSize){
					i--;
					oldSize = newSize;
				}
				else{
					if(objects[i]->data.point != 3 && currentTime*1000.0f > objects[i]->data.time){
						if (objects[i]->data.type != 2){
							//Global.MousePosition = {objects[i]->data.x, objects[i]->data.y};
							objects[i]->data.point = 3;
							score+= 300 + (300 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
							clickCombo++;
							int volume = objects[i]->data.volume;
							if(volume == 0){
								objects[i]->data.volume = objects[i]->data.timing.volume;
								volume = objects[i]->data.volume;
							}
							SetSoundPan(SoundFiles.data[objects[i]->data.NormalSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
							SetSoundVolume(SoundFiles.data[objects[i]->data.NormalSound], (float)volume/100.0f);
							PlaySound(SoundFiles.data[objects[i]->data.NormalSound]);
							if(objects[i]->data.PlayAddition){
								SetSoundPan(SoundFiles.data[objects[i]->data.AdditionSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
								SetSoundVolume(SoundFiles.data[objects[i]->data.AdditionSound], (float)volume/100.0f);
								PlaySound(SoundFiles.data[objects[i]->data.AdditionSound]);
							}
							if(objects[i]->data.PlayCustom){
								SetSoundPan(SoundFiles.data[objects[i]->data.CustomSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
								SetSoundVolume(SoundFiles.data[objects[i]->data.CustomSound], (float)volume/100.0f);
								PlaySound(SoundFiles.data[objects[i]->data.CustomSound]);
							}
							objects[i]->data.time = currentTime*1000.0f;

							Global.AutoMousePositionStart = {objects[i]->data.x, objects[i]->data.y};
							Global.AutoMouseStartTime = currentTime*1000.0f;

							destroyHitObject(i);
							newSize = objects.size();
							stop = false;
							if(newSize != oldSize){
								i--;
								oldSize = newSize;
							}
						}
						else if (objects[i]->data.type == 2){
							Slider* tempslider = dynamic_cast<Slider*>(objects[i]);
							tempslider->is_hit_at_first = true;
							objects[i]->data.point = 3;
							stop = false;
							clickCombo++;
							int volume = tempslider->data.volume;
							if(volume == 0){
								tempslider->data.volume = tempslider->data.timing.volume;
								volume = tempslider->data.volume;
							}
							SetSoundPan(SoundFiles.data[objects[i]->data.EdgeNormalSound[0]], 1-clip(objects[i]->data.x / 640.0, 0, 1));
							SetSoundPan(SoundFiles.data[objects[i]->data.EdgeAdditionSound[0]], 1-clip(objects[i]->data.x / 640.0, 0, 1));
							SetSoundVolume(SoundFiles.data[objects[i]->data.EdgeNormalSound[0]], (float)volume/100.0f);
							SetSoundVolume(SoundFiles.data[objects[i]->data.EdgeAdditionSound[0]], (float)volume/100.0f);
							PlaySound(SoundFiles.data[objects[i]->data.EdgeNormalSound[0]]);
							PlaySound(SoundFiles.data[objects[i]->data.EdgeAdditionSound[0]]);
							if(tempslider->data.PlayCustom){
								SetSoundPan(SoundFiles.data[tempslider->data.CustomSound], 1-clip(objects[i]->data.x / 640.0, 0, 1));
								SetSoundVolume(SoundFiles.data[tempslider->data.CustomSound], (float)volume/100.0f);
								PlaySound(SoundFiles.data[tempslider->data.CustomSound]);
							}
							objects[i]->data.index = i;
							objects[i]->update();
							newSize = objects.size();
							if(newSize != oldSize){
								i--;
								oldSize = newSize;
							}
						}
					}
					else{
						if (objects[i]->data.type == 2){
							objects[i]->data.index = i;
							objects[i]->update();
							newSize = objects.size();
							if(newSize != oldSize){
								i--;
								oldSize = newSize;
							}
						}
					}
				}

				newSize = objects.size();
				if(newSize != oldSize){
					i--;
					oldSize = newSize;
				}
			}
			else{
				objects[i]->data.index = i;
				objects[i]->update();
				newSize = objects.size();
				if(newSize != oldSize){
					i--;
					oldSize = newSize;
				}
			}
		}
	}


	int deadoldsize = dead_objects.size();
	int deadnewsize = dead_objects.size();

	for(int i = 0; i < dead_objects.size(); i++){
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
	}
}

//main rendering loop
void GameManager::render(){

	if(currentBackgroundTexture.length() > 0 && backgroundTextures.loaded[currentBackgroundTexture].value){
		//std::cout << currentBackgroundTexture << std::endl;
		DrawTextureCenter(backgroundTextures.data[currentBackgroundTexture], 320, 240, (double)std::max((double)GetScreenWidth()/(double)backgroundTextures.data[currentBackgroundTexture].width, (double)GetScreenHeight()/(double)backgroundTextures.data[currentBackgroundTexture].height) / (double)Global.Scale , WHITE);
		DrawRectangle(-5, -5, GetScreenWidth() + 10, GetScreenHeight() + 10, Fade(BLACK, 0.8f));
	}
	
	for(int i = objects.size() - 1; i >= 0; i--){
		////Global.mutex.lock();
		objects[i]->render();
		////Global.mutex.unlock();
	}
	for(int i = dead_objects.size() - 1; i >= 0; i--){
		////Global.mutex.lock();
		dead_objects[i]->dead_render();
		////Global.mutex.unlock();
	}
	DrawCNumbersCenter(score, 320, 10, 0.4f, WHITE);
	DrawCNumbersLeft(clickCombo, 15, 460, 0.6f, WHITE);

	
	if(spawnedHitObjects == 0 && gameFile.hitObjects[gameFile.hitObjects.size() - 1].time > 6000 + currentTime*1000.0f){
		////Global.mutex.lock();
		DrawTextEx(Global.DefaultFont, TextFormat("TO SKIP PRESS \"S\"\n(Keep in mind that this can affect the offset\nbecause of how the raylib sounds system works)"), {ScaleCordX(5), ScaleCordY(420)}, Scale(15), Scale(1), WHITE);
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
		Global.amogus2 = 16;
		Global.amogus3 = 0.0f;
		Global.startTime += Global.FrameTime;
		Time = Global.startTime;
		//std::cout << Time << std::endl;
	}
	if(Global.startTime >= 0 and startMusic){
		std::cout << "trying to start music" << std::endl;
		PlayMusicStream(backgroundMusic);
    	SetMusicVolume(backgroundMusic, Global.volume);
		SeekMusicStream(backgroundMusic, 0.0f);
		UpdateMusicStream(backgroundMusic);
		initTimer();
		std::cout << Global.amogus << std::endl;
		std::cout << "started music" << std::endl;
		std::cout << "first update" << std::endl;
		Global.CurrentInterpolatedTime = 0;
		Global.LastOsuTime = 0;
		TimeLast = ms;
		startMusic = false;
		Global.startTime2 = ms;
		double Time = (double)GetMusicTimePlayed(backgroundMusic) * 1000.0;
		double amog = getTimer();
		Global.amogus4 = getTimer();
		Global.amogus2 = 12;
		Global.amogus3 = 0.0f;
		std::cout << "time delay??? " << Global.amogus2 << std::endl;
		std::cout << "Time:" << Time << std::endl;
		std::cout << "amog?:" << amog << std::endl;
		Global.avgSum = 0;
    	Global.avgNum = 0;
    	Global.avgTime = 0;

		Global.LastFrameTime = getTimer();
	}
	if(true){
		UpdateMusicStream(backgroundMusic);
		if(spawnedHitObjects == 0 && gameFile.hitObjects[gameFile.hitObjects.size() - 1].time > 6000 + currentTime*1000.0f){
			//DrawTextEx(Global.DefaultFont, TextFormat("TO SKIP PRESS \"S\"\n(Keep in mind that this can affect the offset\nbecause of how the raylib sounds system works)"), {ScaleCordX(5), ScaleCordY(420)}, Scale(15), Scale(1), WHITE);
			if(IsKeyPressed(SDL_SCANCODE_S )){
				SeekMusicStream(backgroundMusic, (gameFile.hitObjects[gameFile.hitObjects.size() - 1].time - 3000.0f) / 1000.0f);
			}
		}
		if(GetMusicTimeLength(backgroundMusic) - GetMusicTimePlayed(backgroundMusic) < 0.05f)
			stop = true;
		if(stop && Global.curTime2 < 1.0f){
			StopMusicStream(backgroundMusic);
		}
		
		
		if (IsMusicStreamPlaying(backgroundMusic)){
			Time = (double)GetMusicTimePlayed(backgroundMusic) * 1000.0;
			if(!AreSame(TimerLast, Time)){
				//Global.amogus2 = std::abs((Time - TimerLast) / 1.5f);
				Global.amogus3 = Time - TimerLast;

				TimerLast = (double)GetMusicTimePlayed(backgroundMusic) * 1000.0;
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

		if (IsMusicStreamPlaying(backgroundMusic)){
			if (GetMusicTimePlayed(backgroundMusic) * 1000.0 != 0)
				IsInterpolating = true;
			double ElapsedTime = getTimer() - Global.LastOsuTime;
			Global.LastOsuTime = getTimer();
			Global.CurrentInterpolatedTime += ElapsedTime;
			if (!IsInterpolating || std::abs(GetMusicTimePlayed(backgroundMusic) * 1000.0 - Global.CurrentInterpolatedTime) > 8){
				Global.CurrentInterpolatedTime = ElapsedTime < 0 ? GetMusicTimePlayed(backgroundMusic) * 1000.0: std::max(LastInterpolatedTime, GetMusicTimePlayed(backgroundMusic) * 1000.0);
				IsInterpolating = false;
				std::cout << "failed interpolation at time " << Global.CurrentInterpolatedTime << "\n";
			}
			else{
				Global.CurrentInterpolatedTime += (GetMusicTimePlayed(backgroundMusic) * 1000.0 - Global.CurrentInterpolatedTime) / 5;
				Global.CurrentInterpolatedTime = std::max(LastInterpolatedTime, Global.CurrentInterpolatedTime);
			}
		}

		Global.currentOsuTime = IsMusicStreamPlaying(backgroundMusic) ? Global.CurrentInterpolatedTime : GetMusicTimePlayed(backgroundMusic);

		currentTime = (double)Time / 1000.0;
		if(IsMusicStreamPlaying(backgroundMusic)){
			currentTime = Global.currentOsuTime / 1000.0;
		}

		currentTime -= 8/1000.0f;
		GameManager::update();
		//std::cout << "called update at time " << Global.currentOsuTime << "\n";
		currentTime += 8/1000.0f;
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
	std::vector<std::string> files;
	files.clear();
	Global.Path = "resources/default_skin/";
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0)
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircleselect.png", 0) == 0)
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircle", 0) == 0)
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("approachcircle", 0) == 0)
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit300k", 0) == 0)
				;
			else if(files[i].rfind("hit300", 0) == 0)
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit100k", 0) == 0)
				;
			else if(files[i].rfind("hit100", 0) == 0)
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit50k", 0) == 0)
				;
			else if(files[i].rfind("hit50", 0) == 0)
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit0", 0) == 0)
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderscorepoint", 0) == 0)
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderfollowcircle", 0) == 0)
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderb0", 0) == 0)
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("reversearrow", 0) == 0)
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-circle", 0) == 0){
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre", 0) == 0){
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-bottom", 0) == 0)
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-top", 0) == 0)
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-approachcircle", 0) == 0)
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0){
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
	Global.Path = "resources/skin/";
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0)
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircleselect.png", 0) == 0)
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircle", 0) == 0)
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("approachcircle", 0) == 0)
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit300k", 0) == 0)
				;
			else if(files[i].rfind("hit300", 0) == 0)
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit100k", 0) == 0)
				;
			else if(files[i].rfind("hit100", 0) == 0)
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit50k", 0) == 0)
				;
			else if(files[i].rfind("hit50", 0) == 0)
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit0", 0) == 0)
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderscorepoint", 0) == 0)
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderfollowcircle", 0) == 0)
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderb0", 0) == 0)
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("reversearrow", 0) == 0)
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-circle", 0) == 0){
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				temprenderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre", 0) == 0){
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				temprenderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-background", 0) == 0){
				spinnerBack = LoadTexture((Global.Path + files[i]).c_str());
				temprenderSpinnerBack = true;
			}
			else if(files[i].rfind("spinner-bottom", 0) == 0)
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-top", 0) == 0)
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-approachcircle", 0) == 0)
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0){
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
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0)
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircleselect.png", 0) == 0)
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircle", 0) == 0)
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("approachcircle", 0) == 0)
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit300k", 0) == 0)
				;
			else if(files[i].rfind("hit300", 0) == 0)
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit100k", 0) == 0)
				;
			else if(files[i].rfind("hit100", 0) == 0)
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit50k", 0) == 0)
				;
			else if(files[i].rfind("hit50", 0) == 0)
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit0", 0) == 0)
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderscorepoint", 0) == 0)
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderfollowcircle", 0) == 0)
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderb0", 0) == 0)
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("reversearrow", 0) == 0)
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-circle", 0) == 0){
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre", 0) == 0){
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-background", 0) == 0){
				spinnerBack = LoadTexture((Global.Path + files[i]).c_str());
				temprenderSpinnerBack = true;
			}
			else if(files[i].rfind("spinner-bottom", 0) == 0)
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-top", 0) == 0)
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-approachcircle", 0) == 0)
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0){
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
	Global.amogus = 0;
	Global.amogus2 = 0;
	Global.amogus3 = 0;
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
	std::reverse(gameFile.hitObjects.begin(),gameFile.hitObjects.end());
	std::reverse(gameFile.timingPoints.begin(),gameFile.timingPoints.end());

	lastPath = Global.Path;

	//---------------------------------------------------------------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------|LOADING BACKGROUNDS|------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------


	backgroundTextures.data.clear();
	backgroundTextures.pos.clear();
	backgroundTextures.loaded.clear();

	std::vector<std::string> files;
	files.clear();
	Global.Path = lastPath + '/';
	files = ls(".png");
	/*std::vector<std::string> files2 = ls(".jpg");
	std::vector<std::string> files3 = ls(".jpeg");
	files.insert(files.end(), files2.begin(), files2.end());
	files.insert(files.end(), files3.begin(), files3.end());

	for(int i = 0; i < (int)gameFile.events.size(); i++){
		if(gameFile.events[i].eventType == 0){
			std::cout << "Time: " << gameFile.events[i].startTime << "ms - Filename: " << gameFile.events[i].filename << '.' << std::endl;
			if(gameFile.events[i].startTime == 0){
				gameFile.events[i].startTime -= 7000;
				std::cout << "Time changed to: " << gameFile.events[i].startTime << std::endl;
			}
		}
	}
	

	std::cout << "Found this many files: " << files.size() << std::endl;
	for(int i = 0; i < files.size(); i++){
		std::cout << files[i] << std::endl;
	}

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
				std::cout << "finding function returned: " << files[i].rfind(gameFile.events[j].filename, 0) << " for: " << gameFile.events[j].filename << " and " << files[i] << std::endl;
				if(files[i].rfind(gameFile.events[j].filename, 0) == 0){
					std::cout << "WHAT DA HEEEEEEEEEELLLLLLLLLLLLL" << std::endl;
					Image image = LoadImage((Global.Path + files[i]).c_str());
					backgroundTextures.data[gameFile.events[j].filename] = LoadTextureFromImage(image);
					UnloadImage(image); 
					backgroundTextures.pos[gameFile.events[j].filename] = {gameFile.events[j].xOffset, gameFile.events[j].yOffset};
					if(backgroundTextures.data[gameFile.events[j].filename].width != 0){
						backgroundTextures.loaded[gameFile.events[j].filename].value = true;
						std::cout << "Loaded: Background with filename: " << gameFile.events[j].filename << std::endl;
						GenTextureMipmaps(&backgroundTextures.data[gameFile.events[j].filename]);
						SetTextureFilter(backgroundTextures.data[gameFile.events[j].filename], TEXTURE_FILTER_TRILINEAR );
					}
				}
			}
		}
	}*/


	std::reverse(gameFile.events.begin(),gameFile.events.end());
	double start = getTimer();
	for(int i = 0; i < gameFile.hitObjects.size(); i++){
		if(gameFile.hitObjects[i].type == 2){
			std::vector<Vector2> edgePoints; 
			edgePoints.push_back(Vector2{(float)gameFile.hitObjects[i].x, (float)gameFile.hitObjects[i].y});
			float resolution = gameFile.hitObjects[i].length;
    		float currentResolution = 0;
    		float lengthScale, totalLength = 0;
			
			for(size_t j = 0; j < gameFile.hitObjects[i].curvePoints.size(); j++)
        		edgePoints.push_back(Vector2{(float)gameFile.hitObjects[i].curvePoints[j].first, (float)gameFile.hitObjects[i].curvePoints[j].second});
			
			if(edgePoints.size() == 2 and gameFile.hitObjects[i].curveType == 'B'){
				gameFile.hitObjects[i].curveType == 'L';
			}


			redoCalc:

			if(gameFile.hitObjects[i].curveType == 'L'){
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
				
				gameFile.hitObjects[i].totalLength = totalLength;
				gameFile.hitObjects[i].lengths = lineLengths;
				gameFile.hitObjects[i].extraPos = extraPosition;
			}
			if(gameFile.hitObjects[i].curveType == 'P'){
				if((edgePoints[0].x == edgePoints[2].x and edgePoints[0].y == edgePoints[2].y) or (edgePoints[1].x == edgePoints[2].x and edgePoints[1].y == edgePoints[2].y) or (edgePoints[0].x == edgePoints[1].x and edgePoints[0].y == edgePoints[1].y)){
					gameFile.hitObjects[i].curveType = 'L';
					goto redoCalc;
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
					Vector2 edges[edgePoints.size()];
					for(size_t j = 0; j < edgePoints.size(); j++)
						edges[j] = edgePoints[j];
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
					for(size_t k = 0; k < edgePoints.size(); k++){
						tempEdges.push_back(edgePoints[k]);
						if(k == edgePoints.size()-1 || (edgePoints[k].x == edgePoints[k+1].x && edgePoints[k].y == edgePoints[k+1].y)){
							currentResolution = 0;
							int num = tempEdges.size();
							num = std::max((int)(gameFile.hitObjects[i].length/curves), 50);
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
							// sometimes +1 is better?????
							totalCalculatedLength += tempLength;
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

		}
	}
	std::cout << getTimer() - start << "ms -> The time it took for you to finish :)" << std::endl;
	Global.loadingState = 2;
	
	//calculate all the variables for the game (these may be a bit wrong but they feel right)
	if(std::stof(gameFile.configDifficulty["ApproachRate"]) < 5.0f){
		gameFile.preempt = 1200.0f + 600.0f * (5.0f - std::stof(gameFile.configDifficulty["ApproachRate"])) / 5.0f;
		gameFile.fade_in = 800.0f + 400.0f * (5.0f - std::stof(gameFile.configDifficulty["ApproachRate"])) / 5.0f;
	}
	else if(std::stof(gameFile.configDifficulty["ApproachRate"]) > 5.0f){
		gameFile.preempt = 1200.0f - 750.0f * (std::stof(gameFile.configDifficulty["ApproachRate"]) - 5.0f) / 5;
		gameFile.fade_in = 800.0f - 500.0f * (std::stof(gameFile.configDifficulty["ApproachRate"]) - 5.0f) / 5.0f;
	}
	else{
		gameFile.preempt = 1200.0f;
		gameFile.fade_in = 800.0f;
	}
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
	gameFile.p300Final = gameFile.p300 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p300Change;
	gameFile.p100Final = gameFile.p100 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p100Change;
	gameFile.p50Final = gameFile.p50 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p50Change;
	//debug, just say what the name of the music file is and load it
	//std::cout << (Global.Path + '/' + gameFile.configGeneral["AudioFilename"]) << std::endl;




	//backgroundMusic = LoadMusicStream((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str());



	
	FILE *music = fopen((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str(), "rb");
	fseek(music, 0, SEEK_END);
	musicSize = ftell(music);
	fseek(music, 0, SEEK_SET);  /* same as rewind(f); */

	musicData = (char *)malloc(musicSize + 1);
	fread(musicData, musicSize, 1, music);
	fclose(music);

	musicData[musicSize] = 0;

	backgroundMusic = LoadMusicStreamFromMemory(GetFileExtension((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str()), (const unsigned char *)musicData, musicSize);
	Global.loadingState = 6;
	score = 0;
	clickCombo = 0;
    //TODO: these are not used right now, USE THEM
	float hpdrainrate = std::stof(gameFile.configDifficulty["HPDrainRate"]);
	circlesize = 54.4f - (4.48f * std::stof(gameFile.configDifficulty["CircleSize"]));
	slidertickrate = std::stof(gameFile.configDifficulty["SliderTickRate"]);
	circlesize *= 2.0f;
	//std::cout << circlesize << std::endl;
	float overalldifficulty = std::stof(gameFile.configDifficulty["OverallDifficulty"]);
	//more difficulty stuff, may also be wrong
	difficultyMultiplier = ((hpdrainrate + circlesize + overalldifficulty + clip((float)gameFile.hitObjects.size() / GetMusicTimeLength(backgroundMusic) * 8.f, 0.f, 16.f)) / 38.f * 5.f);
	if (gameFile.configDifficulty.find("SliderMultiplier") != gameFile.configDifficulty.end())
		sliderSpeed = std::stof(gameFile.configDifficulty["SliderMultiplier"]);

	
	

	
	Global.GameTextures = 2;
	/*GameManager::loadDefaultSkin(filename); // LOADING THE DEFAULT SKIN USING A SEPERATE FUNCTION
	GameManager::loadGameSkin(filename); // LOADING THE GAME SKIN USING A SEPERATE FUNCTION
	if(!IsKeyDown(KEY_S)){
		GameManager::loadBeatmapSkin(filename); // LOADING THE BEATMAP SKIN USING A SEPERATE FUNCTION
	}*/




	if(temprenderSpinnerCircle == true){
		renderSpinnerCircle = true;
		std::cout << "================================== RENDERING THE SPINNER CIRCLE ==================================\n";
	}
	else{
		renderSpinnerCircle = false;
	}
	if(temprenderSpinnerMetre == true){
		renderSpinnerMetre = true;
		std::cout << "=================================== RENDERING THE SPINNER METRE ==================================\n";
	}
	else{
		renderSpinnerMetre = false;
	}
	if(temprenderSpinnerBack == true){
		renderSpinnerBack = true;
		std::cout << "================================ RENDERING THE SPINNER BACKGROUND ================================\n";
	}
	else{
		renderSpinnerBack = false;
	}




	




	

	files.clear();
	Global.Path = "resources/skin/";
	files = ls(".wav");

    timingSettings tempTiming;
    std::vector<timingSettings> times;
    int amogus;
    for(int i =  gameFile.timingPoints.size()-1; i >= 0; i--){
        tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
        tempTiming.sliderSpeedOverride = 1;
        tempTiming.time = gameFile.timingPoints[i].time;
        double tempBeatLength;
        tempBeatLength = gameFile.timingPoints[i].beatLength;
        if(tempBeatLength >= 0){
            tempTiming.beatLength = tempBeatLength;
            amogus = tempBeatLength;
            tempTiming.sliderSpeedOverride = 1;
        }
        if(tempBeatLength < 0){
            tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
            tempTiming.beatLength = amogus;
        }
        tempTiming.meter = gameFile.timingPoints[i].meter;
        tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
        tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
        tempTiming.volume = gameFile.timingPoints[i].volume;
        tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
        tempTiming.effects = gameFile.timingPoints[i].effects;
        times.push_back(tempTiming);
    }
	reverse(times.begin(), times.end());

	int defaultSampleSet = 0;
	if(gameFile.configGeneral["SampleSet"] == "Soft"){
		defaultSampleSet = 1;
	}
	else if(gameFile.configGeneral["SampleSet"] == "Drum"){
		defaultSampleSet = 2;
	}

	SoundFiles.data.clear();
	SoundFiles.loaded.clear();
	
	Global.loadingState = 3;

	if(SoundFiles.loaded["combobreak"].value == false){
		Global.Path = "resources/skin/";
		std::vector<std::string> ComboBreak = ls(".mp3");
		for(int i = 0; i < ComboBreak.size(); i++){
			if(ComboBreak[i] == "combobreak.mp3"){
				SoundFiles.data["combobreak"] = LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
				SoundFiles.loaded["combobreak"].value = true;
				std::cout << "loaded combobreak.mp3 from skin\n";
			}
		}
		ComboBreak = ls(".wav");
		for(int i = 0; i < ComboBreak.size(); i++){
			if(ComboBreak[i] == "combobreak.wav"){
				SoundFiles.data["combobreak"] = LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
				SoundFiles.loaded["combobreak"].value = true;
				std::cout << "loaded combobreak.wav from skin\n";
			}
		}
		ComboBreak = ls(".ogg");
		for(int i = 0; i < ComboBreak.size(); i++){
			if(ComboBreak[i] == "combobreak.ogg"){
				SoundFiles.data["combobreak"] = LoadSound(("resources/skin/" + ComboBreak[i]).c_str());
				SoundFiles.loaded["combobreak"].value = true;
				std::cout << "loaded combobreak.ogg from skin\n";
			}
		}
	}

	
	if(SoundFiles.loaded["combobreak"].value == false){
		Global.Path = "resources/default_skin/";
		std::vector<std::string> ComboBreak = ls(".mp3");
		for(int i = 0; i < ComboBreak.size(); i++){
			if(ComboBreak[i] == "combobreak.mp3"){
				SoundFiles.data["combobreak"] = LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFiles.loaded["combobreak"].value = true;
				std::cout << "loaded combobreak.mp3 from default skin\n";
			}
		}
		ComboBreak = ls(".wav");
		for(int i = 0; i < ComboBreak.size(); i++){
			if(ComboBreak[i] == "combobreak.wav"){
				SoundFiles.data["combobreak"] = LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFiles.loaded["combobreak"].value = true;
				std::cout << "loaded combobreak.wav from default skin\n";
			}
		}
		ComboBreak = ls(".ogg");
		for(int i = 0; i < ComboBreak.size(); i++){
			if(ComboBreak[i] == "combobreak.ogg"){
				SoundFiles.data["combobreak"] = LoadSound(("resources/default_skin/" + ComboBreak[i]).c_str());
				SoundFiles.loaded["combobreak"].value = true;
				std::cout << "loaded combobreak.ogg from default skin\n";
			}
		}
	}


	Global.loadingState = 4;

	std::cout << filename << std::endl;
	Global.Path = "resources/default_skin/";
	std::vector<std::string> defaultSounds = ls(".wav");
	Global.Path = "resources/skin/";
	std::vector<std::string> skinSounds = ls(".wav");
	Global.Path = lastPath + '/';
	std::vector<std::string> beatmapSounds = ls(".wav");
	std::vector<std::string> beatmapSoundsOgg = ls(".ogg");
	std::cout << beatmapSoundsOgg.size() << std::endl;

	int TimingPointIndex = gameFile.timingPoints.size() - 1;
	int HitObjectIndex = gameFile.hitObjects.size() - 1;
    Global.parsedLines = 0;
	for(; HitObjectIndex >= 0; HitObjectIndex--){
		Global.parsedLines++;
		for(int amog = TimingPointIndex; amog >= 0; amog--){
			if(times[amog].time > gameFile.hitObjects[HitObjectIndex].time)
				break;
			TimingPointIndex = amog;
			
		}

		int defaultSampleSetForObject = 0;
		int defaultSampleIndexForObject = times[TimingPointIndex].sampleIndex;
		int defaultVolumeForObject = times[TimingPointIndex].volume;

		if(times[TimingPointIndex].sampleSet == 1)
			defaultSampleSetForObject = 0;
		else if(times[TimingPointIndex].sampleSet == 2)
			defaultSampleSetForObject = 1;
		else if(times[TimingPointIndex].sampleSet == 3)
			defaultSampleSetForObject = 2;
		else
			defaultSampleSetForObject = defaultSampleSet;
		
		int NormalSetForObject = 0;
		int AdditionSetForObject = 0;
		int HitSoundForObject = 0;
		int SampleIndexForObject = 0;

		if(gameFile.hitObjects[HitObjectIndex].normalSet == 1)
			NormalSetForObject = 0;
		else if(gameFile.hitObjects[HitObjectIndex].normalSet == 2)
			NormalSetForObject = 1;
		else if(gameFile.hitObjects[HitObjectIndex].normalSet == 3)
			NormalSetForObject = 2;
		else
			NormalSetForObject = defaultSampleSetForObject;
		
		if(gameFile.hitObjects[HitObjectIndex].additionSet == 1)
			AdditionSetForObject = 0;
		else if(gameFile.hitObjects[HitObjectIndex].additionSet == 2)
			AdditionSetForObject = 1;
		else if(gameFile.hitObjects[HitObjectIndex].additionSet == 3)
			AdditionSetForObject = 2;
		else
			AdditionSetForObject = NormalSetForObject;
		
		if(gameFile.hitObjects[HitObjectIndex].hitSound == 2)
			HitSoundForObject = 1;
		else if(gameFile.hitObjects[HitObjectIndex].hitSound == 4)
			HitSoundForObject = 2;
		else if(gameFile.hitObjects[HitObjectIndex].hitSound == 8)
			HitSoundForObject = 3;
		else
			HitSoundForObject = 0;
		
		if(gameFile.hitObjects[HitObjectIndex].index != 0)
			SampleIndexForObject = gameFile.hitObjects[HitObjectIndex].index;
		else
			SampleIndexForObject = defaultSampleIndexForObject;

		if(gameFile.hitObjects[HitObjectIndex].filename.size() > 1){
			gameFile.hitObjects[HitObjectIndex].PlayAddition = false;
			gameFile.hitObjects[HitObjectIndex].PlayCustom = true;
			gameFile.hitObjects[HitObjectIndex].CustomSound = gameFile.hitObjects[HitObjectIndex].filename;
		}
		else{
			gameFile.hitObjects[HitObjectIndex].PlayAddition = true;
			gameFile.hitObjects[HitObjectIndex].PlayCustom = false;
		}
		
		std::string HitSoundIndex = "";

		if(!(SampleIndexForObject == 0 or SampleIndexForObject == 1)){
			HitSoundIndex = std::to_string(SampleIndexForObject);
		}

		std::string NormalFileName;

		if(NormalSetForObject == 0)
			NormalFileName = "normal-hitnormal";
		else if(NormalSetForObject == 1)
			NormalFileName = "soft-hitnormal";
		else
			NormalFileName = "drum-hitnormal";

		std::string tempNoIndex = NormalFileName + ".wav";
		std::string temp = NormalFileName + HitSoundIndex + ".wav";
		std::string tempogg = NormalFileName + HitSoundIndex + ".ogg";
		if(SoundFiles.loaded[temp].value == false){
			for(int i = 0; i < defaultSounds.size(); i++){
				if(defaultSounds[i] == tempNoIndex){
					SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + defaultSounds[i]).c_str());
					SoundFiles.loaded[temp].value = true;
				}
			}
			
			for(int i = 0; i < skinSounds.size(); i++){
				if(skinSounds[i] == tempNoIndex){
					SoundFiles.data[temp] = LoadSound(("resources/skin/" + skinSounds[i]).c_str());
					SoundFiles.loaded[temp].value = true;
				}
			}
			if(!IsKeyDown(SDL_SCANCODE_D )){
				for(int i = 0; i < beatmapSounds.size(); i++){
					if(beatmapSounds[i] == temp){
						SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSounds[i]).c_str());
						SoundFiles.loaded[temp].value = true;
					}
				}
				for(int i = 0; i < beatmapSoundsOgg.size(); i++){
					if(beatmapSoundsOgg[i] == tempogg){
						SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSoundsOgg[i]).c_str());
						SoundFiles.loaded[temp].value = true;
					}
				}
			}
			if(SoundFiles.loaded[temp].value == true){
				std::cout << "loaded "  << temp << std::endl;
				if(SoundFiles.data[temp].frameCount == 0){
					SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + tempNoIndex).c_str());
				}
				std::cout << SoundFiles.data[temp].frameCount << std::endl;
			}
		}
		gameFile.hitObjects[HitObjectIndex].NormalSound = temp;

		std::string AdditionFilename;

		if(AdditionSetForObject == 0)
			AdditionFilename = "normal-hit";
		else if(AdditionSetForObject == 1)
			AdditionFilename = "soft-hit";
		else
			AdditionFilename = "drum-hit";

		if(HitSoundForObject == 0)
			AdditionFilename += "normal";
		else if(HitSoundForObject == 1)
			AdditionFilename += "whistle";
		else if(HitSoundForObject == 2)
			AdditionFilename += "finish";
		else
			AdditionFilename += "clap";

		tempNoIndex = AdditionFilename + ".wav";
		temp = AdditionFilename + HitSoundIndex + ".wav";
		tempogg = AdditionFilename + HitSoundIndex + ".ogg";
		if(SoundFiles.loaded[temp].value == false){
			for(int i = 0; i < defaultSounds.size(); i++){
				if(defaultSounds[i] == tempNoIndex){
					SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + defaultSounds[i]).c_str());
					SoundFiles.loaded[temp].value = true;
				}
			}
			for(int i = 0; i < skinSounds.size(); i++){
				if(skinSounds[i] == tempNoIndex){
					SoundFiles.data[temp] = LoadSound(("resources/skin/" + skinSounds[i]).c_str());
					SoundFiles.loaded[temp].value = true;
				}
			}
			if(!IsKeyDown(SDL_SCANCODE_D )){
				for(int i = 0; i < beatmapSounds.size(); i++){
					if(beatmapSounds[i] == temp){
						SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSounds[i]).c_str());
						SoundFiles.loaded[temp].value = true;
					}
				}
				for(int i = 0; i < beatmapSoundsOgg.size(); i++){
					if(beatmapSoundsOgg[i] == tempogg){
						SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSoundsOgg[i]).c_str());
						SoundFiles.loaded[temp].value = true;
					}
				}
			}
			if(SoundFiles.loaded[temp].value == true){
				std::cout << "loaded "  << temp << std::endl;
				if(SoundFiles.data[temp].frameCount == 0){
					SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + tempNoIndex).c_str());
				}
				std::cout << SoundFiles.data[temp].frameCount << std::endl;
			}
		}

		gameFile.hitObjects[HitObjectIndex].AdditionSound = temp;
		if(gameFile.hitObjects[HitObjectIndex].PlayCustom == true){
			temp = gameFile.hitObjects[HitObjectIndex].CustomSound;
			if(SoundFiles.loaded[temp].value == false){
				if(!IsKeyDown(SDL_SCANCODE_F )){
					for(int i = 0; i < beatmapSounds.size(); i++){
						if(beatmapSounds[i] == temp){
							SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSounds[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
					for(int i = 0; i < beatmapSoundsOgg.size(); i++){
						if(beatmapSoundsOgg[i] == temp){
							SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSoundsOgg[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
				}
				if(SoundFiles.loaded[temp].value == true){
					std::cout << "loaded "  << temp << std::endl;
					if(SoundFiles.data[temp].frameCount == 0){
						SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + tempNoIndex).c_str());
					}
					std::cout << SoundFiles.data[temp].frameCount << std::endl;
				}
			}
		}


		//SLIDERS ---------------------------------------------------------------------------------------------------------------------------
		if(gameFile.hitObjects[HitObjectIndex].type == 2){
			if(NormalSetForObject == 0)
				NormalFileName = "normal-slidertick";
			else if(NormalSetForObject == 1)
				NormalFileName = "soft-slidertick";
			else
				NormalFileName = "drum-slidertick";

			tempNoIndex = NormalFileName + ".wav";
			temp = NormalFileName + HitSoundIndex + ".wav";
			tempogg = NormalFileName + HitSoundIndex + ".ogg";
			if(SoundFiles.loaded[temp].value == false){
				for(int i = 0; i < defaultSounds.size(); i++){
					if(defaultSounds[i] == tempNoIndex){
						SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + defaultSounds[i]).c_str());
						SoundFiles.loaded[temp].value = true;
					}
				}
				for(int i = 0; i < skinSounds.size(); i++){
					if(skinSounds[i] == tempNoIndex){
						SoundFiles.data[temp] = LoadSound(("resources/skin/" + skinSounds[i]).c_str());
						SoundFiles.loaded[temp].value = true;
					}
				}
				if(!IsKeyDown(SDL_SCANCODE_F )){
					for(int i = 0; i < beatmapSounds.size(); i++){
						if(beatmapSounds[i] == temp){
							SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSounds[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
					for(int i = 0; i < beatmapSoundsOgg.size(); i++){
						if(beatmapSoundsOgg[i] == tempogg){
							SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSoundsOgg[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
				}
				if(SoundFiles.loaded[temp].value == true){
					std::cout << "loaded "  << temp << std::endl;
					if(SoundFiles.data[temp].frameCount == 0){
						SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + tempNoIndex).c_str());
					}
					std::cout << SoundFiles.data[temp].frameCount << std::endl;
				}
			}
			gameFile.hitObjects[HitObjectIndex].NormalSound = temp;
			for(int i = 0; i < gameFile.hitObjects[HitObjectIndex].slides + 1; i++){
				NormalSetForObject = 0;
				AdditionSetForObject = 0;
				HitSoundForObject = 0;
				//SampleIndexForObject = 0;
				/*if(i > gameFile.hitObjects[HitObjectIndex].edgeSets.size() - 1 && i > 0){
					gameFile.hitObjects[HitObjectIndex].edgeSets.push_back(gameFile.hitObjects[HitObjectIndex].edgeSets[i - 1]);
				}*/
				if(i < gameFile.hitObjects[HitObjectIndex].edgeSets.size()){
					if(gameFile.hitObjects[HitObjectIndex].edgeSets[i].first == 1)
						NormalSetForObject = 0;
					else if(gameFile.hitObjects[HitObjectIndex].edgeSets[i].first == 2)
						NormalSetForObject = 1;
					else if(gameFile.hitObjects[HitObjectIndex].edgeSets[i].first == 3)
						NormalSetForObject = 2;
					else
						NormalSetForObject = defaultSampleSetForObject;
					
					if(gameFile.hitObjects[HitObjectIndex].edgeSets[i].second == 1)
						AdditionSetForObject = 0;
					else if(gameFile.hitObjects[HitObjectIndex].edgeSets[i].second == 2)
						AdditionSetForObject = 1;
					else if(gameFile.hitObjects[HitObjectIndex].edgeSets[i].second == 3)
						AdditionSetForObject = 2;
					else
						AdditionSetForObject = NormalSetForObject;
				}
				else{
					NormalSetForObject = defaultSampleSetForObject;
					AdditionSetForObject = NormalSetForObject;
				}

				if(i < gameFile.hitObjects[HitObjectIndex].edgeSounds.size()){
				if(gameFile.hitObjects[HitObjectIndex].edgeSounds[i] == 2)
					HitSoundForObject = 1;
				else if(gameFile.hitObjects[HitObjectIndex].edgeSounds[i] == 4)
					HitSoundForObject = 2;
				else if(gameFile.hitObjects[HitObjectIndex].edgeSounds[i] == 8)
					HitSoundForObject = 3;
				else
					HitSoundForObject = 0;
				}
				else{
					HitSoundForObject = 0;
				}
				if(NormalSetForObject == 0)
					NormalFileName = "normal-hitnormal";
				else if(NormalSetForObject == 1)
					NormalFileName = "soft-hitnormal";
				else
					NormalFileName = "drum-hitnormal";

				tempNoIndex = NormalFileName + ".wav";
				temp = NormalFileName + HitSoundIndex + ".wav";
				tempogg = NormalFileName + HitSoundIndex + ".ogg";
				if(SoundFiles.loaded[temp].value == false){
					for(int i = 0; i < defaultSounds.size(); i++){
						if(defaultSounds[i] == tempNoIndex){
							SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + defaultSounds[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
					for(int i = 0; i < skinSounds.size(); i++){
						if(skinSounds[i] == tempNoIndex){
							SoundFiles.data[temp] = LoadSound(("resources/skin/" + skinSounds[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
					if(!IsKeyDown(SDL_SCANCODE_D )){
						for(int i = 0; i < beatmapSounds.size(); i++){
							if(beatmapSounds[i] == temp){
								SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSounds[i]).c_str());
								SoundFiles.loaded[temp].value = true;
							}
						}
						for(int i = 0; i < beatmapSoundsOgg.size(); i++){
							if(beatmapSoundsOgg[i] == tempogg){
								SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSoundsOgg[i]).c_str());
								SoundFiles.loaded[temp].value = true;
							}
						}
					}
					if(SoundFiles.loaded[temp].value == true){
						std::cout << "loaded "  << temp << std::endl;
						if(SoundFiles.data[temp].frameCount == 0){
							SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + tempNoIndex).c_str());
						}
						std::cout << SoundFiles.data[temp].frameCount << std::endl;
					}
				}
				gameFile.hitObjects[HitObjectIndex].EdgeNormalSound.push_back(temp);

				std::string AdditionFilename;

				if(AdditionSetForObject == 0)
					AdditionFilename = "normal-hit";
				else if(AdditionSetForObject == 1)
					AdditionFilename = "soft-hit";
				else
					AdditionFilename = "drum-hit";

				if(HitSoundForObject == 0)
					AdditionFilename += "normal";
				else if(HitSoundForObject == 1)
					AdditionFilename += "whistle";
				else if(HitSoundForObject == 2)
					AdditionFilename += "finish";
				else
					AdditionFilename += "clap";

				tempNoIndex = AdditionFilename + ".wav";
				temp = AdditionFilename + HitSoundIndex + ".wav";
				tempogg = AdditionFilename + HitSoundIndex + ".ogg";
				if(SoundFiles.loaded[temp].value == false){
					for(int i = 0; i < defaultSounds.size(); i++){
						if(defaultSounds[i] == tempNoIndex){
							SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + defaultSounds[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
					for(int i = 0; i < skinSounds.size(); i++){
						if(skinSounds[i] == tempNoIndex){
							SoundFiles.data[temp] = LoadSound(("resources/skin/" + skinSounds[i]).c_str());
							SoundFiles.loaded[temp].value = true;
						}
					}
					if(!IsKeyDown(SDL_SCANCODE_D )){
						for(int i = 0; i < beatmapSounds.size(); i++){
							if(beatmapSounds[i] == temp){
								SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSounds[i]).c_str());
								SoundFiles.loaded[temp].value = true;
							}
						}
						for(int i = 0; i < beatmapSoundsOgg.size(); i++){
							if(beatmapSoundsOgg[i] == tempogg){
								SoundFiles.data[temp] = LoadSound((lastPath + '/' + beatmapSoundsOgg[i]).c_str());
								SoundFiles.loaded[temp].value = true;
							}
						}
					}
					if(SoundFiles.loaded[temp].value == true){
						std::cout << "loaded "  << temp << std::endl;
						if(SoundFiles.data[temp].frameCount == 0){
							SoundFiles.data[temp] = LoadSound(("resources/default_skin/" + tempNoIndex).c_str());
						}
						std::cout << SoundFiles.data[temp].frameCount << std::endl;
					}
				}
				gameFile.hitObjects[HitObjectIndex].EdgeAdditionSound.push_back(temp);
			}
		}
		/*std::cout << gameFile.hitObjects[HitObjectIndex].slides << " - " << gameFile.hitObjects[HitObjectIndex].edgeSounds.size() << " - " << gameFile.hitObjects[HitObjectIndex].edgeSets.size() << std::endl;
		for(int i = 0; i < gameFile.hitObjects[HitObjectIndex].edgeSounds.size(); i++){
			std::cout << gameFile.hitObjects[HitObjectIndex].edgeSounds[i] << " ";
		}
		std::cout << "EdgeSounds\n";
		for(int i = 0; i < gameFile.hitObjects[HitObjectIndex].edgeSets.size(); i++){
			std::cout << gameFile.hitObjects[HitObjectIndex].edgeSets[i].first << ":" << gameFile.hitObjects[HitObjectIndex].edgeSets[i].second << " ";
		}
		std::cout << "EdgeSounds\n";*/

	}
	

	Global.loadingState = 7;

	Global.Path = lastPath;
	Global.GameTextures = 1;
	std::cout << "mark text for loading" << std::endl;

	/*GenTextureMipmaps(&hit0);
	SetTextureFilter(hit0, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hit50);
	SetTextureFilter(hit50, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hit100);
	SetTextureFilter(hit100, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hit300);
	SetTextureFilter(hit300, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&approachCircle);
	SetTextureFilter(approachCircle, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&selectCircle);
	SetTextureFilter(selectCircle, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hitCircleOverlay);
	SetTextureFilter(hitCircleOverlay, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hitCircle);
	SetTextureFilter(hitCircle, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&sliderb);
	SetTextureFilter(sliderb, TEXTURE_FILTER_TRILINEAR );


	SetTextureFilter(sliderin, TEXTURE_FILTER_POINT);
	SetTextureFilter(sliderout, TEXTURE_FILTER_POINT);

	GenTextureMipmaps(&sliderscorepoint);
	SetTextureFilter(sliderscorepoint, TEXTURE_FILTER_TRILINEAR );
	
	//GenTextureMipmaps(&sliderout);
	//SetTextureFilter(sliderout, TEXTURE_FILTER_TRILINEAR );
	
	

	GenTextureMipmaps(&reverseArrow);
	SetTextureFilter(reverseArrow, TEXTURE_FILTER_TRILINEAR );
	for(int i = 0; i < 10; i++){
		GenTextureMipmaps(&numbers[i]);
		SetTextureFilter(numbers[i], TEXTURE_FILTER_TRILINEAR );  //OPENGL1.1 DOESNT SUPPORT THIS
	}*/
	startMusic = true;
	stop = false;
}

void GameManager::unloadGame(){
	std::cout << "GameManager::unloadGame()" << std::endl;
	currentComboIndex = 0;
	Global.GameTextures = -1;
	
	for(auto& pair : SoundFiles.data) {
    	UnloadSound(pair.second);
  	}

	UnloadMusicStream(backgroundMusic);
	free(musicData);

	Global.numberLines = -1;
    Global.parsedLines = -1;

	SoundFiles.data.clear();
	SoundFiles.loaded.clear();
	gameFile.hitObjects.clear();
	objects.clear();
	dead_objects.clear();
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

    objects.push_back(temp);
}

void GameManager::destroyHitObject(int index){
	//declare a hitobject dead
	dead_objects.push_back(objects[index]);
	objects.erase(objects.begin()+index);
}

void GameManager::destroyDeadHitObject(int index){
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
		delete dead_objects[index];
		dead_objects.erase(dead_objects.begin()+index);
	//}
}

void GameManager::render_points(){
	//garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
    
}

void GameManager::render_combo(){
	//garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
    
}

void GameManager::loadGameTextures(){
	sliderin = LoadTexture("resources/sliderin.png");
    sliderout = LoadTexture("resources/sliderout.png");
    loadDefaultSkin(Global.selectedPath); // LOADING THE DEFAULT SKIN USING A SEPERATE FUNCTION
    loadGameSkin(Global.selectedPath); // LOADING THE GAME SKIN USING A SEPERATE FUNCTION
    if(!IsKeyDown(SDL_SCANCODE_S )){
        loadBeatmapSkin(Global.selectedPath); // LOADING THE BEATMAP SKIN USING A SEPERATE FUNCTION
    }
    GenTextureMipmaps(&hit0);
    SetTextureFilter(hit0, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&hit50);
    SetTextureFilter(hit50, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&hit100);
    SetTextureFilter(hit100, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&hit300);
    SetTextureFilter(hit300, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&approachCircle);
    SetTextureFilter(approachCircle, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&selectCircle);
    SetTextureFilter(selectCircle, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&hitCircleOverlay);
    SetTextureFilter(hitCircleOverlay, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&hitCircle);
    SetTextureFilter(hitCircle, TEXTURE_FILTER_TRILINEAR );
    GenTextureMipmaps(&sliderb);
    SetTextureFilter(sliderb, TEXTURE_FILTER_TRILINEAR );


    SetTextureFilter(sliderin, TEXTURE_FILTER_POINT);
    SetTextureFilter(sliderout, TEXTURE_FILTER_POINT);

    GenTextureMipmaps(&sliderscorepoint);
    SetTextureFilter(sliderscorepoint, TEXTURE_FILTER_TRILINEAR );

    GenTextureMipmaps(&reverseArrow);
    SetTextureFilter(reverseArrow, TEXTURE_FILTER_TRILINEAR );
    for(int i = 0; i < 10; i++){
        GenTextureMipmaps(&numbers[i]);
        SetTextureFilter(numbers[i], TEXTURE_FILTER_TRILINEAR );  //OPENGL1.1 DOESNT SUPPORT THIS
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
			if(gameFile.events[i].startTime == 0){
				gameFile.events[i].startTime -= 7000;
				std::cout << "Time changed to: " << gameFile.events[i].startTime << std::endl;
			}
		}
	}
	

	std::cout << "Found this many files: " << files.size() << std::endl;
	for(int i = 0; i < files.size(); i++){
		std::cout << files[i] << std::endl;
	}

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
				std::cout << "finding function returned: " << files[i].rfind(gameFile.events[j].filename, 0) << " for: " << gameFile.events[j].filename << " and " << files[i] << std::endl;
				if(files[i].rfind(gameFile.events[j].filename, 0) == 0){
					std::cout << "WHAT DA HEEEEEEEEEELLLLLLLLLLLLL" << std::endl;
					Image image = LoadImage((Global.Path + files[i]).c_str());
					backgroundTextures.data[gameFile.events[j].filename] = LoadTextureFromImage(image);
					UnloadImage(image); 
					backgroundTextures.pos[gameFile.events[j].filename] = {gameFile.events[j].xOffset, gameFile.events[j].yOffset};
					if(backgroundTextures.data[gameFile.events[j].filename].width != 0){
						backgroundTextures.loaded[gameFile.events[j].filename].value = true;
						std::cout << "Loaded: Background with filename: " << gameFile.events[j].filename << std::endl;
						GenTextureMipmaps(&backgroundTextures.data[gameFile.events[j].filename]);
						SetTextureFilter(backgroundTextures.data[gameFile.events[j].filename], TEXTURE_FILTER_TRILINEAR );
					}
				}
			}
		}
	}

    Global.GameTextures = 0;
}
void GameManager::unloadGameTextures(){
	std::cout << "UnloadingTextures" << std::endl;
    Global.GameTextures = 0;
    UnloadTexture(hitCircleOverlay);
    UnloadTexture(hitCircle);
    UnloadTexture(sliderscorepoint);
    UnloadTexture(approachCircle);
    UnloadTexture(hit300);
    UnloadTexture(hit100);
    UnloadTexture(hit50);
    UnloadTexture(hit0);
    UnloadTexture(sliderb);
    UnloadTexture(sliderin);
    UnloadTexture(sliderout);
    UnloadTexture(selectCircle);
    UnloadTexture(reverseArrow);
    UnloadTexture(spinnerBottom);
    UnloadTexture(spinnerTop);
    UnloadTexture(spinnerCircle);
    UnloadTexture(spinnerApproachCircle);
    UnloadTexture(spinnerMetre);
    for(int i = 0; i < 10; i++){
        UnloadTexture(numbers[i]);
    }
    std::string key;
    for(std::map<std::string, Texture2D>::iterator it = backgroundTextures.data.begin(); it != backgroundTextures.data.end(); ++it){
        key = it->first;
        std::cout << "Removed: " << it->first << "\n";
        UnloadTexture(backgroundTextures.data[key]);
    }

    backgroundTextures.data.clear();
    backgroundTextures.pos.clear();
    backgroundTextures.loaded.clear();
}
